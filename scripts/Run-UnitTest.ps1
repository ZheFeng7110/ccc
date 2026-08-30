#!/usr/bin/env pwsh

<#
.SYNOPSIS
    CI unit test runner for the ccc C++ library.

.DESCRIPTION
    Builds and runs unit tests across all major C++ standards (11-23).
    For C++20 and above, tests both header-only and C++20 module configurations.

    CMake automatically detects the toolchain. Use CcOverride and CxxOverride
    to override the C and C++ compilers respectively.

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

.PARAMETER CcOverride
    Override C compiler path (sets -DCMAKE_C_COMPILER).

.PARAMETER CxxOverride
    Override C++ compiler path (sets -DCMAKE_CXX_COMPILER).

.PARAMETER UseLibCXX
    Add -stdlib=libc++ to the C++ compiler flags.
#>

[CmdletBinding()]
param(
    [string]$BuildRoot = "build_ci",
    [ValidateSet("11", "14", "17", "20", "23")]
    [string[]]$Standards = @("11", "14", "17", "20", "23"),
    [switch]$Clean,
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Release",
    [string]$CcOverride = "",
    [string]$CxxOverride = "",
    [switch]$UseLibCXX
)

$ErrorActionPreference = "Stop"

New-Variable -Name paramVerboseOn `
    -Value ($PSBoundParameters.ContainsKey('Verbose') -and ($PSBoundParameters['Verbose'] -eq $true)) `
    -Option ReadOnly

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
# macOS SDK / libc++ helpers (needed by C++ module dependency scanning)
# ----------------------------------------------------------------------
function Get-MacOsSdkPath
{
    if ($Platform -ne "macos") { return "" }
    if (-not (Get-Command xcrun -ErrorAction SilentlyContinue)) { return "" }
    $sdkPath = & xcrun --show-sdk-path 2>$null
    if ($LASTEXITCODE -eq 0 -and $sdkPath -and (Test-Path $sdkPath)) { return $sdkPath }
    return ""
}

function Get-LibCxxIncludeDir
{
    param([string]$Compiler)
    if ($Platform -ne "macos") { return "" }
    $resolved = if ($Compiler) { $Compiler } else { "clang++" }
    $cmd = Get-Command $resolved -ErrorAction SilentlyContinue
    if (-not $cmd) { return "" }
    $llvmPrefix = Split-Path -Parent (Split-Path -Parent $cmd.Source)
    $includeDir = Join-Path $llvmPrefix "include/c++/v1"
    if (Test-Path $includeDir) { return $includeDir }
    return ""
}

function Get-LibCxxCxxFlags
{
    # clang-scan-deps (invoked by CMake to scan C++20+ sources) does not
    # reproduce the clang driver's own macOS SDK and libc++ include path
    # inference, failing with "'cstddef' file not found". CMake forwards
    # CMAKE_CXX_FLAGS to the scanner, so pass both paths explicitly.
    param([string]$Compiler)
    $flags = "-stdlib=libc++"
    $sdkPath = Get-MacOsSdkPath
    if ($sdkPath) { $flags = "$flags -isysroot $sdkPath" }
    $libcxxInclude = Get-LibCxxIncludeDir -Compiler $Compiler
    if ($libcxxInclude) { $flags = "$flags -isystem $libcxxInclude" }
    return $flags
}

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
# Core build-and-test function
# ----------------------------------------------------------------------
function Invoke-BuildAndTest
{
    param(
        [string]$Generator,
        [string]$CppStandard,
        [bool]$UseModules,
        [string]$CcOverride,
        [string]$CxxOverride,
        [switch]$UseLibCXX
    )

    $label = "C++$CppStandard"
    if ($UseModules)
    {
        $label += " (modules)"
    }

    $dirname = "cpp$CppStandard"
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
    if ($CcOverride -and $CxxOverride)
    {
        $cmakeConfigArgs += "-DCMAKE_C_COMPILER=$CcOverride"
        $cmakeConfigArgs += "-DCMAKE_CXX_COMPILER=$CxxOverride"
        Write-Host "    CC         = $CcOverride"
        Write-Host "    CXX        = $CxxOverride"
    }
    if ($UseLibCXX)
    {
        $cxxFlags = Get-LibCxxCxxFlags -Compiler $CxxOverride
        $cmakeConfigArgs += "-DCMAKE_CXX_FLAGS=$cxxFlags"
        Write-Host "    CXXFLAGS   = $cxxFlags"
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
    if ($paramVerboseOn) { Write-Host ($cfgOutput | Out-String).TrimEnd() }

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
    if ($paramVerboseOn) { Write-Host ($bldOutput | Out-String).TrimEnd() }

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
    if ($paramVerboseOn) { Write-Host ($tstOutput | Out-String).TrimEnd() }

    Write-Host "    PASSED" -ForegroundColor Green
    Add-Result -Config $label -Passed $true -Message ""
}

# ----------------------------------------------------------------------
# Entry point
# ----------------------------------------------------------------------

Write-Host ""
Write-Host "  ccc Unit Test Runner (CI)" -ForegroundColor Cyan
Write-Host "  Project   : $ProjectRoot"
Write-Host "  Platform  : $Platform"
Write-Host "  Standards : $( $Standards -join ', ' )"
Write-Host "  BuildType : $BuildType"
if ($CcOverride -or $CxxOverride)
{
    Write-Host "  CC override  : $CcOverride"
    Write-Host "  CXX override : $CxxOverride"
    if (-not ($CcOverride -and $CxxOverride))
    {
        Write-Host "  WARNING: Both CcOverride and CxxOverride must be specified; compiler override will be ignored." -ForegroundColor Yellow
    }
}
if ($UseLibCXX)
{
    Write-Host "  Use libc++ : yes"
}
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
# Run all configured standards
# ----------------------------------------------------------------------

Write-Heading "Running test suites"

$Generator = if ($Platform -eq "windows") { "Visual Studio 18 2026" } else { "Ninja" }

foreach ($std in $Standards)
{
    # All standards: header-only mode
    Invoke-BuildAndTest -Generator $Generator -CppStandard $std -UseModules $false `
        -CcOverride $CcOverride -CxxOverride $CxxOverride -UseLibCXX:$UseLibCXX

    # C++20+: also test module mode
    if ([int]$std -ge 20)
    {
        Invoke-BuildAndTest -Generator $Generator -CppStandard $std -UseModules $true `
            -CcOverride $CcOverride -CxxOverride $CxxOverride -UseLibCXX:$UseLibCXX
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
