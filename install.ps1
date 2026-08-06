param([switch]$Elevated)

$ErrorActionPreference = 'Stop'
$repository = 'brunojorri/mo-effector'
$rawRoot = "https://raw.githubusercontent.com/$repository/main"

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    $bootstrap = Join-Path ([IO.Path]::GetTempPath()) 'install-mo-effector-native.ps1'
    Invoke-WebRequest -UseBasicParsing -Uri "$rawRoot/install.ps1" -OutFile $bootstrap
    $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$bootstrap`" -Elevated"
    $process = Start-Process powershell.exe -Verb RunAs -ArgumentList $arguments -Wait -PassThru
    exit $process.ExitCode
}

if (Get-Process AfterFX -ErrorAction SilentlyContinue) {
    throw 'Close After Effects before installing MO Effector Native.'
}

$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) ("mo-effector-native-" + [Guid]::NewGuid().ToString('N'))
$downloadedPlugin = Join-Path $temporaryRoot 'MO Effector Native.aex'
$downloadedChecksum = Join-Path $temporaryRoot 'MO Effector Native.aex.sha256'

try {
    New-Item -ItemType Directory -Force -Path $temporaryRoot | Out-Null
    Write-Host 'Downloading MO Effector Native v1.0...' -ForegroundColor Cyan
    Invoke-WebRequest -UseBasicParsing -Uri "$rawRoot/dist/MO%20Effector%20Native.aex" -OutFile $downloadedPlugin
    Invoke-WebRequest -UseBasicParsing -Uri "$rawRoot/dist/MO%20Effector%20Native.aex.sha256" -OutFile $downloadedChecksum

    $expectedHash = (Get-Content -Raw -LiteralPath $downloadedChecksum).Trim().Split(' ')[0].ToUpperInvariant()
    $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $downloadedPlugin).Hash.ToUpperInvariant()
    if ($actualHash -ne $expectedHash) {
        throw 'The downloaded plugin failed SHA-256 verification.'
    }

    $adobeRoot = Join-Path $env:ProgramFiles 'Adobe'
    $installations = @()
    if (Test-Path -LiteralPath $adobeRoot) {
        $installations = @(Get-ChildItem -LiteralPath $adobeRoot -Directory -Filter 'Adobe After Effects 2026*' | Where-Object {
            Test-Path -LiteralPath (Join-Path $_.FullName 'Support Files\AfterFX.exe')
        })
    }
    if (-not $installations.Count) {
        throw 'Adobe After Effects 2026 was not found.'
    }

    foreach ($installation in $installations) {
        $targetDirectory = Join-Path $installation.FullName 'Support Files\Plug-ins\MO Tools'
        $target = Join-Path $targetDirectory 'MO Effector Native.aex'
        New-Item -ItemType Directory -Force -Path $targetDirectory | Out-Null
        Copy-Item -LiteralPath $downloadedPlugin -Destination $target -Force
        if ((Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash.ToUpperInvariant() -ne $expectedHash) {
            throw "Installed plugin failed verification: $target"
        }
        Write-Host "Installed in $($installation.Name)." -ForegroundColor DarkGray
    }

    # Remove the retired CEP extension. Existing projects are not modified.
    $legacyExtension = Join-Path $env:APPDATA 'Adobe\CEP\extensions\com.docato.moeffector'
    if (Test-Path -LiteralPath $legacyExtension) {
        Remove-Item -LiteralPath $legacyExtension -Recurse -Force
        Write-Host 'Removed the retired CEP extension.' -ForegroundColor DarkGray
    }

    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    foreach ($installation in $installations) {
        $preset = Join-Path $installation.FullName 'Support Files\PresetEffects.xml'
        if (Test-Path -LiteralPath $preset) {
            $xml = Get-Content -Raw -LiteralPath $preset
            if ($xml -match 'Pseudo/MO Cloner Controls') {
                Copy-Item -LiteralPath $preset -Destination "$preset.before-mo-native-$stamp.bak" -Force
                $xml = [regex]::Replace(
                    $xml,
                    '(?s)\s*<Effect matchname="Pseudo/MO Cloner Controls".*?</Effect>\s*',
                    "`r`n",
                    1)
                [IO.File]::WriteAllText($preset, $xml, [Text.UTF8Encoding]::new($false))
                Write-Host 'Removed the retired pseudo-effect registration (backup created).' -ForegroundColor DarkGray
            }
        }
    }

    Write-Host ''
    Write-Host 'MO Effector Native v1.0 installed successfully.' -ForegroundColor Green
    Write-Host 'Open After Effects and choose Effect > MO Tools > MO Effector Native.'
} finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}
