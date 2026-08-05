param([switch]$Elevated)

$ErrorActionPreference = 'Stop'
$repository = 'brunojorri/mo-effector'
$rawInstaller = "https://raw.githubusercontent.com/$repository/main/install.ps1"

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    $bootstrap = Join-Path ([IO.Path]::GetTempPath()) 'install-mo-effector.ps1'
    Invoke-WebRequest -UseBasicParsing -Uri $rawInstaller -OutFile $bootstrap
    $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$bootstrap`" -Elevated"
    Start-Process powershell.exe -Verb RunAs -ArgumentList $arguments -Wait
    return
}

$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) ("mo-effector-" + [Guid]::NewGuid().ToString('N'))
$archive = Join-Path $temporaryRoot 'repository.zip'
$expanded = Join-Path $temporaryRoot 'expanded'

try {
    New-Item -ItemType Directory -Force -Path $temporaryRoot, $expanded | Out-Null
    Write-Host 'Downloading MO Effector...' -ForegroundColor Cyan
    Invoke-WebRequest -UseBasicParsing -Uri "https://github.com/$repository/archive/refs/heads/main.zip" -OutFile $archive
    Expand-Archive -LiteralPath $archive -DestinationPath $expanded -Force

    $repositoryRoot = Get-ChildItem -LiteralPath $expanded -Directory | Select-Object -First 1
    if (-not $repositoryRoot) { throw 'The downloaded repository could not be opened.' }

    $extensionSource = Join-Path $repositoryRoot.FullName 'src\MO_Effector_CEP'
    $pseudoDefinition = Join-Path $extensionSource 'host\MOClonerControls.xml'
    if (-not (Test-Path -LiteralPath $extensionSource)) { throw 'The CEP extension was not found in the repository.' }
    if (-not (Test-Path -LiteralPath $pseudoDefinition)) { throw 'The MO Cloner Controls definition was not found.' }

    $extensionTarget = Join-Path $env:APPDATA 'Adobe\CEP\extensions\com.docato.moeffector'
    if (Test-Path -LiteralPath $extensionTarget) { Remove-Item -LiteralPath $extensionTarget -Recurse -Force }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $extensionTarget) | Out-Null
    Copy-Item -LiteralPath $extensionSource -Destination $extensionTarget -Recurse -Force

    $fragment = Get-Content -Raw -LiteralPath $pseudoDefinition
    $adobeRoot = Join-Path $env:ProgramFiles 'Adobe'
    $presetFiles = @()
    if (Test-Path -LiteralPath $adobeRoot) {
        $presetFiles = @(Get-ChildItem -LiteralPath $adobeRoot -Directory -Filter 'Adobe After Effects *' | ForEach-Object {
            $candidate = Join-Path $_.FullName 'Support Files\PresetEffects.xml'
            if (Test-Path -LiteralPath $candidate) { Get-Item -LiteralPath $candidate }
        })
    }
    if (-not $presetFiles.Count) { throw 'No compatible After Effects installation was found.' }

    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    foreach ($presetFile in $presetFiles) {
        $backup = "$($presetFile.FullName).before-mo-effector-$stamp.bak"
        Copy-Item -LiteralPath $presetFile.FullName -Destination $backup -Force
        $xml = Get-Content -Raw -LiteralPath $presetFile.FullName
        if ($xml -match 'Pseudo/MO Cloner Controls') {
            $xml = [regex]::Replace($xml, '(?s)<Effect matchname="Pseudo/MO Cloner Controls".*?</Effect>', [System.Text.RegularExpressions.MatchEvaluator]{ param($match) $fragment }, 1)
        } else {
            $xml = $xml.Replace('</Effects>', "`r`n$fragment`r`n</Effects>")
        }
        [IO.File]::WriteAllText($presetFile.FullName, $xml, [Text.UTF8Encoding]::new($false))
        if ((Get-Content -Raw -LiteralPath $presetFile.FullName) -notmatch 'Pseudo/MO Cloner Controls') { throw "Registration failed for $($presetFile.FullName)." }
        Write-Host "Registered controls in $($presetFile.Directory.Parent.Name)." -ForegroundColor DarkGray
    }

    foreach ($csxsVersion in 9..13) {
        $registryPath = "HKCU:\Software\Adobe\CSXS.$csxsVersion"
        New-Item -Path $registryPath -Force | Out-Null
        New-ItemProperty -Path $registryPath -Name PlayerDebugMode -Value '1' -PropertyType String -Force | Out-Null
    }

    Write-Host ''
    Write-Host 'MO Effector installed successfully.' -ForegroundColor Green
    Write-Host 'Restart After Effects, then open Window > Extensions > MO Effector.'
} finally {
    if (Test-Path -LiteralPath $temporaryRoot) { Remove-Item -LiteralPath $temporaryRoot -Recurse -Force }
}
