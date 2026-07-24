#!/usr/bin/env pwsh

<#
.SYNOPSIS
    CI install test runner for the ccc C++ library.

.DESCRIPTION
    Builds and installs the ccc library, then verifies that a minimal
    downstream project can consume it via find_package(ccc).

    Tests every C++ standard from 11 to 23. For C++20 and above, both
    header-only and C++20 module modes are exercised.

    CMake automatically detects the toolchain. Use CcOverride and CxxOverride
    to override the C and C++ compilers respectively.

    CMake variables used:
        CCC_INSTALL=ON
        CCC_TEST_CPP_STANDARD=<11|14|17|20|23>
        CCC_USE_CPP_MODULES=<ON|OFF>

.PARAMETER BuildRoot
    Root directory for CI build artifacts (default: build-inst-build-tst).

.PARAMETER InstallRoot
    Root directory for CI install artifacts (default: build-inst-tst).

.PARAMETER Standards
    C++ standards to test. Defaults to all: 11, 14, 17, 20, 23.

.PARAMETER Clean
    Remove existing build and install directories before starting.

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
    [string]$BuildRoot = "build-inst-build-tst",
    [string]$InstallRoot = "build-inst-tst",
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
# Minimal downstream project files (hard-coded raw strings)
# ----------------------------------------------------------------------
$ConsumerCMakeLists = @'
cmake_minimum_required(VERSION 3.28)
project(ccc_install_test CXX)

find_package(ccc REQUIRED)

add_executable(ccc_install_test main.cc)
target_link_libraries(ccc_install_test PRIVATE ccc::ccc)
'@

$HeaderMainCc = @'
#include <ccc/utility.hh>

int main() {
    (void)ccc::in_place;
    return 0;
}
'@

$ModuleMainCc = @'
import ccc;

int main() {
    (void)ccc::in_place;
    return 0;
}
'@

# ----------------------------------------------------------------------
# Paths
# ----------------------------------------------------------------------
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/.."
$BuildRootFull = Join-Path $ProjectRoot $BuildRoot
$InstallRootFull = Join-Path $ProjectRoot $InstallRoot

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
# Core install-and-consume function
# ----------------------------------------------------------------------
function Invoke-InstallTest
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
    $installDir = Join-Path $InstallRootFull $dirname
    $consumerDir = Join-Path $buildDir "consumer"

    Write-Subheading $label
    Write-Host "    Build dir    : $buildDir"
    Write-Host "    Install dir  : $installDir"
    Write-Host "    Consumer dir : $consumerDir"

    # Clean on request
    if ($Clean)
    {
        if (Test-Path $buildDir)
        {
            Write-Host "    Cleaning build dir..."
            Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue
        }
        if (Test-Path $installDir)
        {
            Write-Host "    Cleaning install dir..."
            Remove-Item -Recurse -Force $installDir -ErrorAction SilentlyContinue
        }
    }

    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    New-Item -ItemType Directory -Force -Path $installDir | Out-Null
    New-Item -ItemType Directory -Force -Path $consumerDir | Out-Null

    # ------------------------------------------------------------------
    # Build and install the ccc library
    # ------------------------------------------------------------------
    $cmakeConfigArgs = @(
        "-G", $Generator,
        "-S", $ProjectRoot,
        "-B", $buildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_INSTALL_PREFIX=$installDir",
        "-DCMAKE_CXX_STANDARD=$CppStandard",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
        "-DCCC_INSTALL=ON",
        "-DCCC_BUILD_TESTS=OFF",
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
        $cmakeConfigArgs += "-DCMAKE_CXX_FLAGS=-stdlib=libc++"
        Write-Host "    CXXFLAGS   = -stdlib=libc++"
    }

    # --- Configure ---
    Write-Host "    Configuring library..."
    $cfgOutput = & cmake @cmakeConfigArgs 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($cfgOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    LIBRARY CONFIGURE FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "library configure: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($cfgOutput | Out-String).TrimEnd() }

    # --- Build ---
    Write-Host "    Building library..."
    $bldOutput = & cmake --build $buildDir --config $BuildType 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($bldOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    LIBRARY BUILD FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "library build: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($bldOutput | Out-String).TrimEnd() }

    # --- Install ---
    Write-Host "    Installing library..."
    $instOutput = & cmake @instArgs 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($instOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    LIBRARY INSTALL FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "library install: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($instOutput | Out-String).TrimEnd() }

    # ------------------------------------------------------------------
    # Build and run the minimal downstream project
    # ------------------------------------------------------------------
    $consumerSource = if ($UseModules) { $ModuleMainCc } else { $HeaderMainCc }
    Set-Content -Path (Join-Path $consumerDir "CMakeLists.txt") -Value $ConsumerCMakeLists -NoNewline
    Set-Content -Path (Join-Path $consumerDir "main.cc") -Value $consumerSource -NoNewline

    $consumerBuildDir = Join-Path $consumerDir "build"
    New-Item -ItemType Directory -Force -Path $consumerBuildDir | Out-Null

    $consumerConfigArgs = @(
        "-G", $Generator,
        "-S", $consumerDir,
        "-B", $consumerBuildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_PREFIX_PATH=$installDir",
        "-DCMAKE_CXX_STANDARD=$CppStandard",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    )
    if ($Generator -like "Visual Studio*")
    {
        $consumerConfigArgs += "-A", "x64"
    }
    if ($CcOverride -and $CxxOverride)
    {
        $consumerConfigArgs += "-DCMAKE_C_COMPILER=$CcOverride"
        $consumerConfigArgs += "-DCMAKE_CXX_COMPILER=$CxxOverride"
    }
    if ($UseLibCXX)
    {
        $consumerConfigArgs += "-DCMAKE_CXX_FLAGS=-stdlib=libc++"
    }

    # --- Consumer configure ---
    Write-Host "    Configuring consumer..."
    $ccfgOutput = & cmake @consumerConfigArgs 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($ccfgOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    CONSUMER CONFIGURE FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "consumer configure: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($ccfgOutput | Out-String).TrimEnd() }

    # --- Consumer build ---
    Write-Host "    Building consumer..."
    $cbldOutput = & cmake --build $consumerBuildDir --config $BuildType 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($cbldOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    CONSUMER BUILD FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "consumer build: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($cbldOutput | Out-String).TrimEnd() }

    # --- Consumer run ---
    Write-Host "    Running consumer..."
    $exeName = "ccc_install_test"
    if ($IsWindows)
    {
        $exeName += ".exe"
    }
    if ($Generator -like "Visual Studio*")
    {
        $exePath = Join-Path $consumerBuildDir "$BuildType/$exeName"
    }
    else
    {
        $exePath = Join-Path $consumerBuildDir $exeName
    }

    $runOutput = & $exePath 2>&1
    if ($LASTEXITCODE -ne 0)
    {
        $msg = ($runOutput | Out-String).Trim()
        $short = if ($msg.Length -gt 200) { $msg.Substring(0, 200) + "..." } else { $msg }
        Write-Host "    CONSUMER RUN FAILED" -ForegroundColor Red
        Write-Host $msg -ForegroundColor Red
        Add-Result -Config $label -Passed $false -Message "consumer run: $short"
        return
    }
    if ($paramVerboseOn) { Write-Host ($runOutput | Out-String).TrimEnd() }

    Write-Host "    PASSED" -ForegroundColor Green
    Add-Result -Config $label -Passed $true -Message ""
}

# ----------------------------------------------------------------------
# Entry point
# ----------------------------------------------------------------------

Write-Host ""
Write-Host "  ccc Install Test Runner (CI)" -ForegroundColor Cyan
Write-Host "  Project     : $ProjectRoot"
Write-Host "  Platform    : $Platform"
Write-Host "  Standards   : $( $Standards -join ', ' )"
Write-Host "  BuildType   : $BuildType"
Write-Host "  BuildRoot   : $BuildRootFull"
Write-Host "  InstallRoot : $InstallRootFull"
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
$needsNinja = $Platform -ne "windows"
$moduleStandards = $Standards | Where-Object { [int]$_ -ge 20 }
if ($moduleStandards.Count -gt 0)
{
    $needsNinja = $true
}
if ($needsNinja)
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

Write-Heading "Running install test suites"

$Generator = if ($Platform -eq "windows") { "Visual Studio 18 2026" } else { "Ninja" }

foreach ($std in $Standards)
{
    # All standards: header-only mode
    Invoke-InstallTest -Generator $Generator -CppStandard $std -UseModules $false `
        -CcOverride $CcOverride -CxxOverride $CxxOverride -UseLibCXX:$UseLibCXX

    # C++20+: also test module mode
    if ([int]$std -ge 20)
    {
        Invoke-InstallTest -Generator "Ninja" -CppStandard $std -UseModules $true `
            -CcOverride $CcOverride -CxxOverride $CxxOverride -UseLibCXX:$UseLibCXX
    }
}

# ----------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------

Write-Host ""
Write-Host ("=" * 72)
Write-Host "  INSTALL TEST SUMMARY"
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
    Write-Host "  SOME INSTALL TESTS FAILED." -ForegroundColor Red
    exit 1
}

Write-Host "  All install tests passed." -ForegroundColor Green
exit 0
