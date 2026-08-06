param([switch]$Elevated)

$ErrorActionPreference = 'Stop'
$rawScript = 'https://raw.githubusercontent.com/brunojorri/mo-effector/main/scripts/uninstall.ps1'

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    $bootstrap = Join-Path ([IO.Path]::GetTempPath()) 'uninstall-mo-effector-native.ps1'
    Invoke-WebRequest -UseBasicParsing -Uri $rawScript -OutFile $bootstrap
    $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$bootstrap`" -Elevated"
    $process = Start-Process powershell.exe -Verb RunAs -ArgumentList $arguments -Wait -PassThru
    exit $process.ExitCode
}

if (Get-Process AfterFX -ErrorAction SilentlyContinue) {
    throw 'Close After Effects before uninstalling MO Effector Native.'
}

$adobeRoot = Join-Path $env:ProgramFiles 'Adobe'
if (Test-Path -LiteralPath $adobeRoot) {
    Get-ChildItem -LiteralPath $adobeRoot -Directory -Filter 'Adobe After Effects 2026*' | ForEach-Object {
        $plugin = Join-Path $_.FullName 'Support Files\Plug-ins\MO Tools\MO Effector Native.aex'
        if (Test-Path -LiteralPath $plugin) {
            Remove-Item -LiteralPath $plugin -Force
            Write-Host "Removed from $($_.Name)." -ForegroundColor DarkGray
        }
    }
}

$legacyExtension = Join-Path $env:APPDATA 'Adobe\CEP\extensions\com.docato.moeffector'
if (Test-Path -LiteralPath $legacyExtension) {
    Remove-Item -LiteralPath $legacyExtension -Recurse -Force
}

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
if (Test-Path -LiteralPath $adobeRoot) {
    Get-ChildItem -LiteralPath $adobeRoot -Directory -Filter 'Adobe After Effects 2026*' | ForEach-Object {
        $preset = Join-Path $_.FullName 'Support Files\PresetEffects.xml'
        if (Test-Path -LiteralPath $preset) {
            $xml = Get-Content -Raw -LiteralPath $preset
            if ($xml -match 'Pseudo/MO Cloner Controls') {
                Copy-Item -LiteralPath $preset -Destination "$preset.before-mo-uninstall-$stamp.bak" -Force
                $xml = [regex]::Replace(
                    $xml,
                    '(?s)\s*<Effect matchname="Pseudo/MO Cloner Controls".*?</Effect>\s*',
                    "`r`n",
                    1)
                [IO.File]::WriteAllText($preset, $xml, [Text.UTF8Encoding]::new($false))
            }
        }
    }
}

Write-Host 'MO Effector Native was removed. Restart After Effects.' -ForegroundColor Green
