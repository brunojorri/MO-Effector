param([ValidateSet('Debug', 'Release')][string]$Configuration = 'Release')

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$repositoryRoot = Split-Path -Parent (Split-Path -Parent $projectRoot)
$defaultSdk = Join-Path $repositoryRoot 'sdk\ae25.6_61.64bit.AfterEffectsSDK'
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'

if (-not $env:AE_SDK_ROOT) { $env:AE_SDK_ROOT = $defaultSdk }
if (-not (Test-Path -LiteralPath (Join-Path $env:AE_SDK_ROOT 'Examples\Headers\AE_Effect.h'))) {
    throw "AE_SDK_ROOT does not point to a valid After Effects SDK: $env:AE_SDK_ROOT"
}
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer was not found.' }

$visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) { throw 'Visual Studio C++ build tools were not found.' }
$msbuild = Join-Path $visualStudio 'MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuild)) { throw "MSBuild was not found in $visualStudio." }

$project = Join-Path $projectRoot 'plugin\MOEffectorNative.vcxproj'
& $msbuild $project /t:Build /p:Configuration=$Configuration /p:Platform=x64 /m /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Plugin build failed with exit code $LASTEXITCODE." }

$output = Join-Path $projectRoot "build\plugin\$Configuration\MO Effector Native.aex"
if (-not (Test-Path -LiteralPath $output)) { throw 'The AEX output was not created.' }
Get-Item -LiteralPath $output | Select-Object FullName, Length, LastWriteTime
