# ---------------------------------------------------------------------------
# package.ps1
#
# Builds a release distribution of DnsManager and produces a portable ZIP
# (and, optionally, an NSIS installer via CPack).
#
# Pipeline:
#   1. build the Release configuration from CMake presets
#   2. stage the executable + README + LICENSE
#   3. run windeployqt (Qt DLLs, plugins, QML) into the staging folder
#   4. pack everything into dist\DnsManager-<version>-windows-x64.zip
#
# Examples:
#   .\scripts\package.ps1                                          # vs2022 Release zip
#   .\scripts\package.ps1 -Preset ninja-release                    # Ninja build
#   .\scripts\package.ps1 -Packager all                            # zip + NSIS
#   .\scripts\package.ps1 -SkipBuild                               # repack existing build
#
# Requires: Windows PowerShell 5.1, CMake, a built Qt kit, and windeployqt
# (found on PATH, in $env:QT_ROOT, or auto-detected under C:\Qt).
# ---------------------------------------------------------------------------

[CmdletBinding()]
param(
    [string]$Preset = 'vs2022-release',
    [string]$Config = 'Release',
    [ValidateSet('zip', 'nsis', 'all')]
    [string]$Packager = 'zip',
    [string]$OutDir = 'dist',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repo = Split-Path -Parent $PSScriptRoot

function Write-Step { param([string]$Message) Write-Host "==> $Message" -ForegroundColor Cyan }

# --- Resolve version from CMakeLists.txt --------------------------------
$version = '1.0.0'
$cmakeText = Get-Content -Raw (Join-Path $repo 'CMakeLists.txt')
$m = [regex]::Match($cmakeText, 'VERSION\s+(\d+\.\d+\.\d+)')
if ($m.Success) { $version = $m.Groups[1].Value }
Write-Step "DnsManager version: $version"

# --- Locate windeployqt --------------------------------------------------
$windeployqt = $null
$cmd = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($cmd) { $windeployqt = $cmd.Source }

if (-not $windeployqt -and $env:QT_ROOT) {
    $candidate = Join-Path $env:QT_ROOT 'bin\windeployqt.exe'
    if (Test-Path $candidate) { $windeployqt = $candidate }
}

if (-not $windeployqt -and (Test-Path 'C:\Qt')) {
    $qtRoots = Get-ChildItem 'C:\Qt' -Directory | Where-Object { $_.Name -match '^\d' }
    $qtRoot = $qtRoots | Sort-Object Name -Descending | Select-Object -First 1
    if ($qtRoot) {
        foreach ($kit in 'msvc2022_64', 'mingw_64', 'clang_64') {
            $candidate = Join-Path $qtRoot.FullName "$kit\bin\windeployqt.exe"
            if (Test-Path $candidate) { $windeployqt = $candidate; break }
        }
    }
}

if (-not $windeployqt) {
    throw 'windeployqt.exe not found. Install it, put it on PATH, set $env:QT_ROOT, or install Qt under C:\Qt.'
}
Write-Step "windeployqt: $windeployqt"

# --- Build ----------------------------------------------------------------
$buildDir = Join-Path $repo "build\$Preset"
if (-not $SkipBuild) {
    Write-Step "Building preset $Preset"
    Push-Location $repo
    try {
        & cmake --build --preset $Preset --config $Config
        if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE" }
    }
    finally { Pop-Location }
}

# Locate the executable (multi-config VS vs single-config Ninja/MinGW).
$exe = Join-Path $buildDir "$Config\DnsManager.exe"
if (-not (Test-Path $exe)) { $exe = Join-Path $buildDir 'DnsManager.exe' }
if (-not (Test-Path $exe)) { throw "Built executable not found under $buildDir" }
Write-Step "Executable: $exe"

# --- Stage -----------------------------------------------------------------
$stage = Join-Path $repo 'build\package-stage'
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force -Path $stage | Out-Null

Copy-Item $exe (Join-Path $stage 'DnsManager.exe')
foreach ($file in 'README.md', 'LICENSE') {
    $src = Join-Path $repo $file
    if (Test-Path $src) { Copy-Item $src $stage }
}

# --- Deploy Qt runtime -----------------------------------------------------
Write-Step 'Deploying Qt runtime with windeployqt'
$deployArgs = @(
    '--no-translations',
    '--no-system-d3d-compiler',
    "--qmldir", (Join-Path $repo 'qml')
)
if ($Config -eq 'Debug') { $deployArgs += '--debug' } else { $deployArgs += '--release' }

Push-Location $stage
try {
    & $windeployqt @deployArgs 'DnsManager.exe' | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE" }
}
finally { Pop-Location }

# --- Pack ZIP --------------------------------------------------------------
New-Item -ItemType Directory -Force -Path (Join-Path $repo $OutDir) | Out-Null
$zipBase = "DnsManager-$version-windows-x64"
if ($Packager -in @('zip', 'all')) {
    $zipPath = Join-Path $repo (Join-Path $OutDir "$zipBase.zip")
    if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
    Write-Step "Creating ZIP: $zipPath"
    Compress-Archive -Path "$stage\*" -DestinationPath $zipPath -CompressionLevel Optimal
}

# --- NSIS via CPack ---------------------------------------------------------
if ($Packager -in @('nsis', 'all')) {
    Write-Step 'Building NSIS installer with CPack'
    $cpackConfig = Join-Path $buildDir 'CPackConfig.cmake'
    if (-not (Test-Path $cpackConfig)) {
        Write-Warning "No $cpackConfig - configure with -DDNSMGR_BUILD_NSIS=ON first; skipping NSIS."
    }
    else {
        & cpack --config $cpackConfig -C $Config
        if ($LASTEXITCODE -ne 0) { throw "cpack failed with exit code $LASTEXITCODE" }
        foreach ($artifact in Get-ChildItem $buildDir -Filter "DnsManager-*.exe" -File) {
            Copy-Item $artifact.FullName (Join-Path $repo (Join-Path $OutDir $artifact.Name))
        }
    }
}

Write-Host ''
Write-Step "Packaging complete. Artifacts in $(Join-Path $repo $OutDir)"
