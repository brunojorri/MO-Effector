$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $projectRoot 'build'
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer (vswhere.exe) was not found.'
}

$visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) {
    throw 'Install the Desktop development with C++ workload in Visual Studio.'
}

$developerCommand = Join-Path $visualStudio 'Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path -LiteralPath $developerCommand)) {
    throw "VsDevCmd.bat was not found in $visualStudio."
}

New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null
$compilerArguments = @(
    '/nologo', '/std:c++20', '/EHsc', '/W4', '/WX', '/permissive-',
    ('/I"' + (Join-Path $projectRoot 'include') + '"'),
    ('"' + (Join-Path $projectRoot 'src\Core.cpp') + '"'),
    ('"' + (Join-Path $projectRoot 'tests\CoreTests.cpp') + '"'),
    ('/Fe:"' + (Join-Path $buildRoot 'mo_core_tests.exe') + '"')
) -join ' '

$command = '"' + $developerCommand + '" -arch=x64 -host_arch=x64 >nul && cl.exe ' + $compilerArguments
Push-Location $buildRoot
try {
    & $env:ComSpec /d /s /c $command
    if ($LASTEXITCODE -ne 0) { throw "C++ compilation failed with exit code $LASTEXITCODE." }
} finally {
    Pop-Location
}

& (Join-Path $buildRoot 'mo_core_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "Core tests failed with exit code $LASTEXITCODE." }
