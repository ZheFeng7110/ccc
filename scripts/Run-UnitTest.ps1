#!/usr/bin/env pwsh

<#
.SYNOPSIS
    CI unit test runner for the ccc C++ library.

.DESCRIPTION
    Builds and runs unit tests across all major C++ standards (11-23).
    For C++20 and above, tests both header-only and C++20 module configurations.

    Toolchain matrix:
        Windows  : MSVC (Visual Studio 2026) + Ninja
        macOS    : GCC (libstdc++) + Ninja, Clang (libc++) + Ninja
        Linux    : GCC (libstdc++) + Ninja, Clang (libc++) + Ninja

    CMake variables used:
        CCC_BUILD_TESTS=ON
        CCC_TEST_CPP_STANDARD=<11|14|17|20|23>
        CCC_USE_CPP_MODULES=<ON|OFF>

.PARAMETER BuildRoot
    Root directory for CI build artifacts (default: build_ci).

.PARAMETER Standards
    C++ standards to test. Defaults to all: 11, 14, 17, 20, 23.

.PARAMETER Clean
    Remove existing build directories before starting.

.PARAMETER BuildType
    CMake build type (default: Release).

.PARAMETER Toolchains
    Specific toolchains to test. If empty, auto-detects based on platform.
    Valid values: msvc, gcc, clang.

.PARAMETER CcOverride
    Override C compiler path (sets CC environment variable).

.PARAMETER CxxOverride
    Override C++ compiler path (sets CXX environment variable).
#>

[CmdletBinding()]
param(
    [string]$BuildRoot = "build_ci",
    [ValidateSet("11", "14", "17", "20", "23")]
    [string[]]$Standards = @("11", "14", "17", "20", "23"),
    [switch]$Clean,
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Release",
    [ValidateSet("msvc", "gcc", "clang")]
    [string[]]$Toolchains = @(),
    [string]$CcOverride = "",
    [string]$CxxOverride = ""
)

$ErrorActionPreference = "Stop"
# ----------------------------------------------------------------------
# Paths
# ----------------------------------------------------------------------
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/.."
$BuildRootFull = Join-Path $ProjectRoot $BuildRoot

# ----------------------------------------------------------------------
# Platform detection
# ----------------------------------------------------------------------
function Get-Platform
{
    if ($IsWindows) { return "windows" }
    if ($IsMacOS) { return "macos" }
    if ($IsLinux) { return "linux" }
    throw "Unsupported platform: unable to determine OS."
}

$Platform = Get-Platform

# ----------------------------------------------------------------------
# Results tracking
# ----------------------------------------------------------------------
$Script:AllResults = [System.Collections.Generic.List[PSCustomObject]]::new()

function Add-Result
{
    param([string]$Config, [bool]$Passed, [string]$Message)
    $Script:AllResults.Add([PSCustomObject]@{ Config = $Config; Passed = $Passed; Message = $Message })
}

# ----------------------------------------------------------------------
# Output helpers
# ----------------------------------------------------------------------
function Write-Heading
{
    param([string]$Text, [string]$Color = "Cyan")
    Write-Host ""
    Write-Host ("=" * 72) -ForegroundColor $Color
    Write-Host "  $Text" -ForegroundColor $Color
    Write-Host ("=" * 72) -ForegroundColor $Color
}

function Write-Subheading
{
    param([string]$Text)
    Write-Host ""
    Write-Host ("-" * 48)
    Write-Host "  $Text"
    Write-Host ("-" * 48)
}
# ----------------------------------------------------------------------
# Environment setup helpers
# ----------------------------------------------------------------------

function Import-VsEnvironment
{
    <#
    .SYNOPSIS
        Locate Visual Studio 2026 and import its environment into the current
        PowerShell session so that cl.exe, Ninja, and CMake all work together.
        Returns $true on success.
    #>
    # Quick check: is MSVC already configured?
    if (Get-Command cl.exe -ErrorAction SilentlyContinue)
    {
        Write-Host "    MSVC environment already active (cl.exe in PATH)."
        return $true
    }

    # Try vswhere first
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vsPath = $null
    if (Test-Path $vswhere)
    {
        $vsPath = & $vswhere -latest -version "[18.0,19.0)" -property installationPath 2> $null
    }

    # Fallback: common paths
    if (-not $vsPath)
    {
        $candidates = @(
            "${env:ProgramFiles}\Microsoft Visual Studio\2026\Enterprise",
            "${env:ProgramFiles}\Microsoft Visual Studio\2026\Professional",
            "${env:ProgramFiles}\Microsoft Visual Studio\2026\Community"
        )
        foreach ($c in $candidates)
        {
            if (Test-Path "$c\VC\Auxiliary\Build\vcvarsall.bat")
            {
                $vsPath = $c
                break
            }
        }
    }

    if (-not $vsPath)
    {
        Write-Host "    Visual Studio 2026 not found." -ForegroundColor Yellow
        Write-Host "    Searched via vswhere and common install paths." -ForegroundColor Yellow
        return $false
    }

    Write-Host "    Found Visual Studio 2026 at: $vsPath"
    $vcvarsBat = "$vsPath\VC\Auxiliary\Build\vcvarsall.bat"

    # Capture the full environment after running vcvarsall x64
    Write-Host "    Importing VS environment (vcvarsall x64)..."
    $captured = cmd /c "`"$vcvarsBat`" x64 > nul 2>&1 && set" 2> $null

    foreach ($line in $captured)
    {
        if ($line -match '^([^=]+)=(.*)$')
        {
            $name = $Matches[1]
            $value = $Matches[2]
            # Skip internal variables that are noisy or harmful
            if ($name -in @('_', 'ERRORLEVEL', 'PROMPT', 'PWD', 'OLDPWD'))
            {
                continue
            }
            [Environment]::SetEnvironmentVariable($name, $value)
        }
    }

    Write-Host "    VS environment imported."
    return $true
}

function Find-Compiler
{
    param(
        [string[]]$CcNames,
        [string[]]$CxxNames
    )
    for ($i = 0; $i -lt [Math]::Min($CcNames.Count, $CxxNames.Count); $i++) {
        $cc = Get-Command $CcNames[$i]  -ErrorAction SilentlyContinue
        $cxx = Get-Command $CxxNames[$i] -ErrorAction SilentlyContinue
        if ($cc -and $cxx)
        {
            return @{ CC = $cc.Source; CXX = $cxx.Source }
        }
    }
    return $null
}
# ----------------------------------------------------------------------
# Core build-and-test function
# ----------------------------------------------------------------------

function Invoke-BuildAndTest
{
    param(
        [string]$Generator,
        [string]$Toolchain,
        [string]$CppStandard,
        [bool]$UseModules,
        [hashtable]$CompilerEnv,
        [string[]]$ExtraCmakeArgs
    )

    $label = "$Toolchain / C++$CppStandard"
    if ($UseModules)
    {
        $label += " (modules)"
    }

    $dirname = "$Toolchain-cpp$CppStandard"
    if ($UseModules)
    {
        $dirname += "-modules"
    }
    $buildDir = Join-Path $BuildRootFull $dirname

    Write-Subheading $label
    Write-Host "    Build dir : $buildDir"

    # Clean on request
    if ($Clean -and (Test-Path $buildDir))
    {
        Write-Host "    Cleaning..."
        Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue
    }

    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

    # Build cmake configure arguments
    $cmakeConfigArgs = @(
        "-G", $Generator,
        "-S", $ProjectRoot,
        "-B", $buildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCCC_BUILD_TESTS=ON",
        "-DCCC_TEST_CPP_STANDARD=$CppStandard"
    )
    if ($Generator -like "Visual Studio*")
    {
        $cmakeConfigArgs += "-A", "x64"
    }
    if ($UseModules)
    {
        $cmakeConfigArgs += "-DCCC_USE_CPP_MODULES=ON"
    }
    if ($ExtraCmakeArgs)
    {
        $cmakeConfigArgs += $ExtraCmakeArgs
    }

    # Set CC / CXX env vars for this invocation (only when non-empty)
    $prevCC = $env:CC
    $prevCXX = $env:CXX
    try
    {
        if ($CompilerEnv)
        {
            if ($CompilerEnv.ContainsKey('CC') -and $CompilerEnv['CC'])
            {
                $env:CC = $CompilerEnv['CC']
                Write-Host "    CC         = $( $CompilerEnv['CC'] )"
            }
            if ($CompilerEnv.ContainsKey('CXX') -and $CompilerEnv['CXX'])
            {
                $env:CXX = $CompilerEnv['CXX']
                Write-Host "    CXX        = $( $CompilerEnv['CXX'] )"
            }
        }

        # --- Configure ---
        Write-Host "    Configuring..."
        $cfgOutput = & cmake @cmakeConfigArgs 2>&1
        if ($LASTEXITCODE -ne 0)
        {
            $msg = ($cfgOutput | Out-String).Trim()
            $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
            Write-Host "    CONFIGURE FAILED" -ForegroundColor Red
            Write-Host $msg -ForegroundColor Red
            Add-Result -Config $label -Passed $false -Message "configure: $short"
            return
        }

        # --- Build ---
        Write-Host "    Building..."
        $bldOutput = & cmake --build $buildDir --config $BuildType 2>&1
        if ($LASTEXITCODE -ne 0)
        {
            $msg = ($bldOutput | Out-String).Trim()
            $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
            Write-Host "    BUILD FAILED" -ForegroundColor Red
            Write-Host $msg -ForegroundColor Red
            Add-Result -Config $label -Passed $false -Message "build: $short"
            return
        }

        # --- Test ---
        Write-Host "    Testing..."
        $tstOutput = & ctest --test-dir $buildDir -C $BuildType --output-on-failure 2>&1
        if ($LASTEXITCODE -ne 0)
        {
            $msg = ($tstOutput | Out-String).Trim()
            $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
            Write-Host "    TESTS FAILED" -ForegroundColor Red
            Write-Host $msg -ForegroundColor Red
            Add-Result -Config $label -Passed $false -Message "tests: $short"
            return
        }

        Write-Host "    PASSED" -ForegroundColor Green
        Add-Result -Config $label -Passed $true -Message ""
    }
    finally
    {
        $env:CC = $prevCC
        $env:CXX = $prevCXX
    }
}
# ----------------------------------------------------------------------
# Per-toolchain runner
# ----------------------------------------------------------------------

function Invoke-ToolchainSuite
{
    param(
        [string]$Generator,
        [string]$Name,
        [scriptblock]$EnvSetupScript,
        [string[]]$ExtraCmakeArgs
    )

    Write-Heading "Toolchain: $Name"

    $compilerEnv = & $EnvSetupScript
    if ($null -eq $compilerEnv)
    {
        Write-Host "  SKIPPED -- toolchain not available on this system." -ForegroundColor Yellow
        return
    }

    foreach ($std in $Standards)
    {
        # All standards: header-only mode
        Invoke-BuildAndTest -Generator $Generator -Toolchain $Name -CppStandard $std -UseModules $false `
            -CompilerEnv $compilerEnv -ExtraCmakeArgs $ExtraCmakeArgs

        # C++20+: also test module mode
        if ([int]$std -ge 20)
        {
            Invoke-BuildAndTest -Generator $Generator -Toolchain $Name -CppStandard $std -UseModules $true `
                -CompilerEnv $compilerEnv -ExtraCmakeArgs $ExtraCmakeArgs
        }
    }
}

# ----------------------------------------------------------------------
# Toolchain definitions
# ----------------------------------------------------------------------

$SupportedToolchains = @()

# --- MSVC (Windows only) ---
if ($Platform -eq "windows")
{
    $SupportedToolchains += "msvc"
}

# --- GCC and Clang (macOS / Linux) ---
if ($Platform -in @("macos", "linux"))
{
    if ($Platform -eq "macos")
    {
        # On macOS, GNU GCC is often installed via Homebrew as gcc-14 / g++-14
        $gccCandidates = @("gcc-14", "gcc-13", "gcc-12", "gcc-11", "gcc")
        $gxxCandidates = @("g++-14", "g++-13", "g++-12", "g++-11", "g++")
    }
    else
    {
        # Linux: prefer versioned names
        $gccCandidates = @("gcc-14", "gcc-13", "gcc-12", "gcc-11", "gcc")
        $gxxCandidates = @("g++-14", "g++-13", "g++-12", "g++-11", "g++")
    }

    $clangCandidates = @("clang-18", "clang-17", "clang-16", "clang-15", "clang")
    $clangxxCandidates = @("clang++-18", "clang++-17", "clang++-16", "clang++-15", "clang++")

    $SupportedToolchains += "gcc"
    $SupportedToolchains += "clang"
}
# Apply toolchain filter if specified
if ($Toolchains.Count -gt 0)
{
    $activeToolchains = $Toolchains | Where-Object { $_ -in $SupportedToolchains }
    $skipped = $Toolchains | Where-Object { $_ -notin $SupportedToolchains }
    if ($skipped)
    {
        Write-Host "WARNING: Requested toolchains not available on $Platform : $( $skipped -join ', ' )" -ForegroundColor Yellow
    }
}
else
{
    $activeToolchains = $SupportedToolchains
}

# Apply overrides
$userCC = if ($CcOverride) { $CcOverride } else { "" }
$userCXX = if ($CxxOverride) { $CxxOverride } else { "" }

# ----------------------------------------------------------------------
# Entry point
# ----------------------------------------------------------------------

Write-Host ""
Write-Host "  ccc Unit Test Runner (CI)" -ForegroundColor Cyan
Write-Host "  Project   : $ProjectRoot"
Write-Host "  Platform  : $Platform"
Write-Host "  Standards : $( $Standards -join ', ' )"
Write-Host "  BuildType : $BuildType"
Write-Host "  Toolchains: $( $activeToolchains -join ', ' )"
Write-Host ""

# Check prerequisites
if (-not (Get-Command cmake -ErrorAction SilentlyContinue))
{
    Write-Host "ERROR: cmake not found in PATH." -ForegroundColor Red
    exit 1
}
if ($Platform -ne "windows")
{
    if (-not (Get-Command ninja -ErrorAction SilentlyContinue))
    {
        Write-Host "ERROR: ninja not found in PATH." -ForegroundColor Red
        exit 1
    }
    Write-Host "  ninja : $( ninja --version 2>&1 | Select-Object -First 1 )"
}

Write-Host "  cmake : $( cmake --version | Select-Object -First 1 )"
Write-Host ""
# ----------------------------------------------------------------------
# Run all toolchain suites
# ----------------------------------------------------------------------

foreach ($tc in $activeToolchains)
{
    $setup = $null
    $extra = @()
    switch ($tc)
    {
        "msvc" {
            $setup = {
                if (-not (Import-VsEnvironment))
                {
                    if ($userCC -and $userCXX)
                    {
                        Write-Host "    Using user-specified compiler: $userCXX"
                        return @{ CC = $userCC; CXX = $userCXX }
                    }
                    Write-Host "    MSVC toolchain not available." -ForegroundColor Yellow
                    return $null
                }
                if ($userCC -and $userCXX)
                {
                    return @{ CC = $userCC; CXX = $userCXX }
                }
                return @{ }
            }
        }
        "gcc" {
            $setup = {
                if ($userCC -and $userCXX)
                {
                    Write-Host "    Using user-specified compiler: $userCXX"
                    return @{ CC = $userCC; CXX = $userCXX }
                }
                $info = Find-Compiler -CcNames $gccCandidates -CxxNames $gxxCandidates
                if (-not $info)
                {
                    Write-Host "    GCC not found." -ForegroundColor Yellow
                    return $null
                }
                Write-Host "    Found GCC: $( $info.CXX )"
                return $info
            }
        }
        "clang" {
            $setup = {
                if ($userCC -and $userCXX)
                {
                    Write-Host "    Using user-specified compiler: $userCXX"
                    return @{ CC = $userCC; CXX = $userCXX }
                }
                $info = Find-Compiler -CcNames $clangCandidates -CxxNames $clangxxCandidates
                if (-not $info)
                {
                    Write-Host "    Clang not found." -ForegroundColor Yellow
                    return $null
                }
                Write-Host "    Found Clang: $( $info.CXX )"
                return $info
            }
            $extra = @("-DCMAKE_CXX_FLAGS=-stdlib=libc++")
        }
    }
    if ($setup)
    {
        $gen = if ($tc -eq "msvc") { "Visual Studio 18 2026" } else { "Ninja" }
        Invoke-ToolchainSuite -Generator $gen -Name $tc -EnvSetupScript $setup -ExtraCmakeArgs $extra
    }
}
# ----------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------

Write-Host ""
Write-Host ("=" * 72)
Write-Host "  TEST SUMMARY"
Write-Host ("=" * 72)

$passed = ($Script:AllResults | Where-Object { $_.Passed }).Count
$failed = ($Script:AllResults | Where-Object { -not $_.Passed }).Count
$total = $Script:AllResults.Count

foreach ($r in $Script:AllResults)
{
    $tag = if ($r.Passed) { "PASS" } else { "FAIL" }
    $col = if ($r.Passed) { "Green" } else { "Red" }
    Write-Host "  [$tag] $( $r.Config )" -ForegroundColor $col
    if ($r.Message)
    {
        Write-Host "          $( $r.Message )" -ForegroundColor $col
    }
}

Write-Host ""
Write-Host "  Total: $total  |  Passed: $passed  |  Failed: $failed" -ForegroundColor `
    $( if ($failed -gt 0) { "Red" } else { "Green" } )

if ($failed -gt 0)
{
    Write-Host ""
    Write-Host "  SOME TESTS FAILED." -ForegroundColor Red
    exit 1
}

Write-Host "  All tests passed." -ForegroundColor Green
exit 0
