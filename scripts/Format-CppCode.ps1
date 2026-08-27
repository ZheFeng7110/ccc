#!/usr/bin/env pwsh

<#
.SYNOPSIS
    Format all the C/C++ code. Just run `Format-CppCode.ps1` simply.
#>

[CmdletBinding()]
param ()

$beginCwd = Get-Location
Write-Debug "`$beginCwd=$beginCwd"

$workingDir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Write-Debug "`$workingDir=$workingDir"

$clangFormatExe = Get-Command clang-format -ErrorAction Stop

Set-Location -Path $workingDir

$formatFailed = $false
try {
    Get-ChildItem -Recurse -Path "include", "modules", "test" `
        -Include "*.c", "*.cc", "*.h", "*.hh", "*.ccm" -Exclude "target/**" | `
        ForEach-Object {
            & $clangFormatExe -i -style=file $_.FullName 2>&1
            if ($LASTEXITCODE -ne 0) { $global:formatFailed = $true }
        }
}
finally {
    Set-Location -Path $beginCwd
    if ($formatFailed) { exit 1 }
}
