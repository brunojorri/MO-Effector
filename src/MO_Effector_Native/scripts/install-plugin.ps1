param([switch]$Elevated)

$ErrorActionPreference = 'Stop'

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    $arguments = @(
        '-NoProfile',
        '-ExecutionPolicy', 'Bypass',
        '-File', ('"' + $PSCommandPath + '"'),
        '-Elevated'
    )
    $process = Start-Process powershell.exe -Verb RunAs -ArgumentList $arguments -Wait -PassThru -WindowStyle Hidden
    exit $process.ExitCode
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $projectRoot 'build\plugin\Release\MO Effector Native.aex'
$afterEffectsRoot = 'C:\Program Files\Adobe\Adobe After Effects 2026\Support Files'
$targetDirectory = Join-Path $afterEffectsRoot 'Plug-ins\MO Tools'
$target = Join-Path $targetDirectory 'MO Effector Native.aex'

if (Get-Process AfterFX -ErrorAction SilentlyContinue) {
    throw 'Close After Effects before installing MO Effector Native.'
}
if (-not (Test-Path -LiteralPath $source)) {
    throw "Compile the Release plugin before installing: $source"
}
if (-not (Test-Path -LiteralPath (Join-Path $afterEffectsRoot 'AfterFX.exe'))) {
    throw 'Adobe After Effects 2026 was not found.'
}

New-Item -ItemType Directory -Force -Path $targetDirectory | Out-Null
Copy-Item -LiteralPath $source -Destination $target -Force

$sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $source).Hash
$targetHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash
if ($sourceHash -ne $targetHash) {
    throw 'The installed plugin did not pass checksum verification.'
}
