param([switch]$Elevated)

$ErrorActionPreference = 'Stop'
$rawScript = 'https://raw.githubusercontent.com/brunojorri/mo-effector/main/scripts/uninstall.ps1'

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    $bootstrap = Join-Path ([IO.Path]::GetTempPath()) 'uninstall-mo-effector.ps1'
    Invoke-WebRequest -UseBasicParsing -Uri $rawScript -OutFile $bootstrap
    Start-Process powershell.exe -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$bootstrap`" -Elevated" -Wait
    return
}

$extensionTarget = Join-Path $env:APPDATA 'Adobe\CEP\extensions\com.docato.moeffector'
if (Test-Path -LiteralPath $extensionTarget) { Remove-Item -LiteralPath $extensionTarget -Recurse -Force }

$adobeRoot = Join-Path $env:ProgramFiles 'Adobe'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
if (Test-Path -LiteralPath $adobeRoot) {
    Get-ChildItem -LiteralPath $adobeRoot -Directory -Filter 'Adobe After Effects *' | ForEach-Object {
        $preset = Join-Path $_.FullName 'Support Files\PresetEffects.xml'
        if (Test-Path -LiteralPath $preset) {
            $xml = Get-Content -Raw -LiteralPath $preset
            if ($xml -match 'Pseudo/MO Cloner Controls') {
                Copy-Item -LiteralPath $preset -Destination "$preset.before-mo-uninstall-$stamp.bak" -Force
                $xml = [regex]::Replace($xml, '(?s)\s*<Effect matchname="Pseudo/MO Cloner Controls".*?</Effect>\s*', "`r`n", 1)
                [IO.File]::WriteAllText($preset, $xml, [Text.UTF8Encoding]::new($false))
            }
        }
    }
}

Write-Host 'MO Effector was removed. Restart After Effects.' -ForegroundColor Green
