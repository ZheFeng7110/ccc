#!/usr/bin/env python3
"""Build each isolated tassert scenario and check real subprocess results."""
import argparse
import fnmatch
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent


def execute(command, *, cwd, log, timeout=180):
    result = subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, timeout=timeout, errors="replace")
    with log.open("a", encoding="utf-8") as stream:
        stream.write(f"$ {command!r}\n{result.stdout}{result.stderr}\nexit: {result.returncode}\n")
    return result


def check(result, case, failure_code):
    expected = failure_code if case["exit"] == "failure" else case["exit"]
    if result.returncode != expected:
        raise RuntimeError(f"exit {result.returncode}, expected {expected}")
    for name in ("stdout", "stderr"):
        for pattern in case.get(name, []):
            if not re.search(pattern, getattr(result, name)):
                raise RuntimeError(f"{name} missing {pattern!r}")
    for pattern in case.get("absent", []):
        if re.search(pattern, result.stdout + result.stderr):
            raise RuntimeError(f"unexpected output {pattern!r}")


def compiler_diagnostics(result):
    return '\n'.join(re.findall(r'(?:fatal )?error(?: [A-Z]+[0-9]+)?:\s*([^\n]+)',
                                result.stdout + result.stderr, re.I))


def checked(command, *, cwd, log):
    result = execute(command, cwd=cwd, log=log)
    if result.returncode:
        raise RuntimeError(f"command failed ({result.returncode}); see {log}")
    return result


def cmake_options(entry, mode):
    return [f'-DTEST_STANDARD={entry["standard"]}', f'-DTEST_BACKEND={entry["backend"]}',
            f'-DTEST_CONFIG={entry["config"]}', f'-DTEST_IMPORT={mode}',
            '-DTEST_DEFINES=' + ';'.join(entry["defines"])]


def cmake_build(entry, mode, build, args, log):
    source = HERE / entry["path"]
    options = cmake_options(entry, mode)
    common = ['-G', args.generator, '-DCMAKE_BUILD_TYPE=Release'] + args.cmake_arg
    dependency_prefixes = []
    if entry["path"] == "install_consumer" and entry["backend"] == 2:
        dependencies = build / 'dependencies'
        dependency_prefix = build / 'dependency-prefix'
        checked(['cmake', '-S', str(ROOT / 'test'), '-B', str(dependencies)] + common + [
            '-DCCC_TEST_PREPARE_FMT=ON', '-DCCC_TEST_DEPENDENCIES_ONLY=ON',
            f'-DCMAKE_INSTALL_PREFIX={dependency_prefix}'], cwd=ROOT, log=log)
        checked(['cmake', '--build', str(dependencies), '--config', 'Release', '-j', '2'], cwd=ROOT, log=log)
        checked(['cmake', '--install', str(dependencies), '--config', 'Release'], cwd=ROOT, log=log)
        dependency_prefixes.append(str(dependency_prefix))
    if entry["path"] == "install_consumer":
        provider = build / "provider"
        prefix = build / "prefix"
        command = ['cmake', '-S', str(ROOT), '-B', str(provider)] + common + [
            '-DCCC_INSTALL=ON', '-DCCC_BUILD_TESTS=OFF',
            f'-DCMAKE_CXX_STANDARD={entry["standard"]}',
            f'-DCCC_TASSERT_FORMAT_BACKEND={entry["backend"]}',
            '-DCCC_USE_CPP_MODULES=' + ('OFF' if mode == 'header' else 'ON'),
            f'-DCMAKE_INSTALL_PREFIX={prefix}',
            '-DCCC_TASSERT_CONFIG_HEADER=' + (str(HERE / entry["config"]) if entry["config"] else '')]
        if dependency_prefixes:
            command += ['-DCMAKE_PREFIX_PATH=' + ';'.join(dependency_prefixes)]
        checked(command, cwd=ROOT, log=log)
        checked(['cmake', '--build', str(provider), '--config', 'Release', '-j', '2'], cwd=ROOT, log=log)
        checked(['cmake', '--install', str(provider), '--config', 'Release'], cwd=ROOT, log=log)
        options += ['-DTEST_INSTALLED=ON', '-DCMAKE_PREFIX_PATH=' + ';'.join([str(prefix)] + dependency_prefixes)]
    result = execute(['cmake', '-S', str(source), '-B', str(build)] + common + options,
                     cwd=ROOT, log=log)
    if result.returncode:
        raise RuntimeError(f"configuration failed; see {log}")
    result = execute(['cmake', '--build', str(build), '--config', 'Release', '-j', '2'], cwd=ROOT, log=log)
    if entry.get('compile_error'):
        if result.returncode == 0 or not re.search(entry['compile_error'], compiler_diagnostics(result), re.I):
            raise RuntimeError(f"expected compile error {entry['compile_error']!r}; see {log}")
        return None, None
    if result.returncode:
        raise RuntimeError(f"build failed; see {log}")
    extension = '.exe' if os.name == 'nt' else ''
    probe = checked([str(build / 'bin' / ('exit_failure' + extension))], cwd=build, log=log)
    return [str(build / 'bin' / ('test' + extension))], int(probe.stdout.strip())


def mcpp_build(entry, mode, build, args, log):
    # Each variant gets its own workspace, path dependency and module cache.
    repo = build / 'ccc'
    repo.mkdir(exist_ok=True)
    for directory in ('include', 'modules'):
        shutil.copytree(ROOT / directory, repo / directory, dirs_exist_ok=True)
    shutil.copytree(HERE, repo / 'test' / 'tassert', dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns('target', '.mcpp', '__pycache__', 'build*', '*.log'))
    library = (ROOT / 'mcpp.toml').read_text(encoding='utf-8')
    library = library.replace('members = ["test"]', 'members = []')
    library = library.replace('[package]', f'[package]\nstandard = "c++{entry["standard"]}"')
    (repo / 'mcpp.toml').write_text(library, encoding='utf-8')
    project = repo / 'test' / 'tassert' / entry['path']
    manifest = (project / 'mcpp.toml').read_text(encoding='utf-8')
    # Rewrite variant settings through TOML values, keeping the declared dependencies.
    import tomllib
    data = tomllib.loads(manifest)
    flags = [] if entry["backend"] == "auto" else [f'-DCCC_TASSERT_FORMAT_BACKEND={entry["backend"]}']
    if entry['config']:
        flags += ['-DCCC_TASSERT_CONFIG_HEADER="' + (repo / 'test' / 'tassert' / entry['config']).as_posix() + '"']
    if entry['backend'] == 2:
        flags += ['-DFMT_ATTACH_TO_GLOBAL_MODULE=1']
    defines = entry['defines'][:]
    if entry['backend'] in (1, 2):
        defines += ['TEST_FORMAT=1', f'TEST_FORMAT_BACKEND={entry["backend"]}']
    if mode != 'header':
        defines += ['TEST_IMPORT_ALL=1' if mode == 'aggregate' else 'TEST_IMPORT_TASSERT=1']
    if mode == 'mixed':
        defines += ['TEST_MIXED=1']
    manifest = '[workspace]\nmembers = []\n\n[package]\nname = "test"\nversion = "0.0.0"\n'
    manifest += f'standard = "c++{entry["standard"]}"\n\n[build]\n'
    manifest += 'sources = ' + json.dumps(data['build']['sources']) + '\n'
    manifest += 'dialect_cxxflags = ' + json.dumps([flag.replace(chr(34), chr(92) + chr(34)) for flag in flags]) + '\n'
    manifest += 'defines = ' + json.dumps(defines) + '\n'
    manifest += '\n[targets.test]\nkind = "bin"\nmain = "main.cc"\n'
    manifest += '\n[targets.exit_failure]\nkind = "bin"\nmain = "' + '../' * len(Path(entry['path']).parts) + 'exit_failure.cc"\n'
    manifest += '\n[dependencies]\nccc = { path = "' + '../' * (len(Path(entry['path']).parts) + 2) + '"'
    if entry['backend'] == 2:
        manifest += ', features = ["tassert-fmt"]'
    manifest += ' }\n'
    if entry['backend'] == 2:
        manifest += 'fmtlib.fmt = ">=11.2.0"\n'
    if mode != 'header':
        for source in data['build']['sources']:
            if mode == 'mixed' and source == 'helper.cc':
                continue
            manifest += '\n[scan_overrides.' + json.dumps(source) + ']\nimports = ["'
            manifest += ('ccc' if mode == 'aggregate' else 'ccc.tassert') + '"]\n'
    (project / 'mcpp.toml').write_text(manifest, encoding='utf-8')
    # Share downloaded dependency sources; each scenario compiles its own objects and BMIs.
    payload_relative = Path('.mcpp/.xlings/data/xpkgs/fmtlib-x-fmt')
    payload_cache = args.build_root / 'mcpp-payloads' / 'fmtlib.fmt'
    if entry['backend'] == 2:
        existing_payload = HERE / 'format_fmt' / payload_relative
        if not payload_cache.exists() and existing_payload.exists():
            shutil.copytree(existing_payload, payload_cache)
        if payload_cache.exists():
            shutil.copytree(payload_cache, project / payload_relative, dirs_exist_ok=True)
    options = ['--cache', 'local'] + args.mcpp_arg
    result = execute(['mcpp', 'build', '-p', 'test'] + options, cwd=project, log=log)
    if entry['backend'] == 2 and (project / payload_relative).exists():
        shutil.copytree(project / payload_relative, payload_cache, dirs_exist_ok=True)
    if entry.get('compile_error'):
        if result.returncode == 0 or not re.search(entry['compile_error'], compiler_diagnostics(result), re.I):
            raise RuntimeError(f"expected compile error {entry['compile_error']!r}; see {log}")
        return None, None
    if result.returncode:
        raise RuntimeError(f"build failed ({result.returncode}); see {log}")
    probe = checked(['mcpp', 'run', '-p', 'test', 'exit_failure'] + options, cwd=project, log=log)
    numbers = re.findall(r'^([0-9]+)\s*$', probe.stdout, re.M)
    if not numbers:
        raise RuntimeError(f"cannot read EXIT_FAILURE probe; see {log}")
    return ['mcpp', 'run', '-p', 'test', 'test'] + options, int(numbers[-1])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool', choices=['cmake', 'mcpp'], default='cmake')
    parser.add_argument('--project', action='append', default=[], help='Path glob; may be repeated')
    parser.add_argument('--build-root', type=Path, default=ROOT / 'build-tassert')
    parser.add_argument('--generator', default='Ninja')
    parser.add_argument('--cmake-arg', action='append', default=[])
    parser.add_argument('--mcpp-arg', action='append', default=[])
    parser.add_argument('--fail-fast', action='store_true')
    parser.add_argument('--mode', action='append', choices=['header', 'module', 'aggregate', 'mixed'])
    args = parser.parse_args()
    args.build_root = args.build_root.resolve()
    failures = []
    count = 0
    for project in json.loads((HERE / 'projects.json').read_text(encoding='utf-8')):
        if args.tool not in project['tools']:
            continue
        if args.project and not any(fnmatch.fnmatch(project['path'], p) for p in args.project):
            continue
        for variant in project.get('variants', [{}]):
            entry = dict(project, **variant)
            for mode in entry['imports']:
                entry = dict(project, **variant)
                if args.mode and mode not in args.mode:
                    continue
                # The Visual Studio generator cannot consume an installed target
                # whose file set contains BMI-only C++ modules. The provider and
                # all in-tree module scenarios remain covered on this platform;
                # leave this generator limitation out of the installed-consumer
                # count rather than reporting a library failure.
                if (args.tool == 'cmake' and entry['path'] == 'install_consumer' and
                        mode != 'header' and 'Visual Studio' in args.generator):
                    print(f'SKIP {entry["path"]}/{variant.get("name", "default")}/{mode} '
                          '(Visual Studio installed-module generator limitation)', flush=True)
                    continue
                label = f'{entry["path"]}/{variant.get("name", "default")}/{mode}'
                build = args.build_root / args.tool / label
                build.mkdir(parents=True, exist_ok=True)
                log = build / 'scenario.log'
                log.write_text('', encoding='utf-8')
                count += 1
                try:
                    if entry.get('rebuild_config'):
                        mutable_config = build / 'config.hh'
                        mutable_config.write_text((HERE / entry['config']).read_text(encoding='utf-8'),
                                                  encoding='utf-8')
                        entry = dict(entry, config=str(mutable_config))
                    builder = cmake_build if args.tool == 'cmake' else mcpp_build
                    command, failure_code = builder(entry, mode, build, args, log)
                    if command:
                        cwd = build if args.tool == 'cmake' else build / 'ccc' / 'test' / 'tassert' / entry['path']
                        for case in entry['runs']:
                            run_command = command + (['--'] if args.tool == 'mcpp' and case['args'] else []) + case['args']
                            result = execute(run_command, cwd=cwd, log=log, timeout=30)
                            # mcpp may decorate output; verify the actual program directly too.
                            if args.tool == 'mcpp':
                                binaries = list((cwd / 'target').rglob('bin/test' + ('.exe' if os.name == 'nt' else '')))
                                if len(binaries) != 1:
                                    raise RuntimeError(f"expected one mcpp binary, found {len(binaries)}")
                                direct = execute([str(binaries[0])] + case['args'], cwd=cwd, log=log, timeout=30)
                                check(direct, case, failure_code)
                                if result.returncode != direct.returncode:
                                    raise RuntimeError('mcpp run did not propagate the program exit status')
                            else:
                                check(result, case, failure_code)
                    if entry.get('rebuild_config'):
                        mutable_config.write_text((HERE / 'ignore_tags' / 'empty.hh').read_text(encoding='utf-8'),
                                                  encoding='utf-8')
                        rebuilt = dict(entry, defines=entry['defines'] + ['TEST_EMPTY_MODULE_CONFIG=1'])
                        command, failure_code = builder(rebuilt, mode, build, args, log)
                        # The same build directory and configuration filename now describe enabled tags.
                        # A stale module interface fails the consumer's static_assert.
                        case = entry['runs'][0]
                        result = execute(command + case['args'], cwd=cwd, log=log, timeout=30)
                        if args.tool == 'mcpp':
                            # The run command must preserve the custom handler's exit code.
                            check(result, dict(case, stdout=[], stderr=[]), failure_code)
                        else:
                            check(result, case, failure_code)
                    print(f'PASS {label}', flush=True)
                except (RuntimeError, OSError, subprocess.TimeoutExpired) as error:
                    failures.append(label)
                    print(f'FAIL {label}: {error} (log: {log})', flush=True)
                    if args.fail_fast:
                        return 1
    print(f'{count - len(failures)}/{count} projects passed', flush=True)
    return 1 if failures or not count else 0


if __name__ == '__main__':
    sys.exit(main())
