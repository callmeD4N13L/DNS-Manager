# ---------------------------------------------------------------------------
# generate-icons.ps1
#
# Generates the application icons for DnsManager:
#   - resources/icons/app/app-icon-256.png
#   - resources/icons/app/app-icon-32.png
#   - resources/win/app.ico   (256px PNG embedded in an ICO container)
#
# Requires: Windows PowerShell 5.1 with System.Drawing.
# ---------------------------------------------------------------------------

[CmdletBinding()]
param()

Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$pngDir = Join-Path $root 'resources\icons\app'
$winDir = Join-Path $root 'resources\win'

New-Item -ItemType Directory -Force -Path $pngDir | Out-Null
New-Item -ItemType Directory -Force -Path $winDir | Out-Null

# Colors (must stay in sync with qml/theme/Theme.qml)
$bgColor   = [System.Drawing.Color]::FromArgb(255, 23, 24, 28)   # #17181c
$accent    = [System.Drawing.Color]::FromArgb(255, 76, 194, 255) # #4cc2ff

function New-RoundRectPath {
    param([float]$x, [float]$y, [float]$w, [float]$h, [float]$r)
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = $r * 2
    $p.AddArc($x, $y, $d, $d, 180, 90)
    $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $p.CloseFigure()
    return $p
}

function New-AppIcon {
    param([int]$Size)

    $bmp = New-Object System.Drawing.Bitmap($Size, $Size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.Clear([System.Drawing.Color]::Transparent)

    # Rounded-square tile
    $tilePath = New-RoundRectPath 0 0 $Size $Size ($Size * 0.22)
    $bgBrush  = New-Object System.Drawing.SolidBrush($bgColor)
    $g.FillPath($bgBrush, $tilePath)

    # Outer ring (DNS "orbit")
    $ringD  = $Size * 0.62
    $ringX  = ($Size - $ringD) / 2
    $ringY  = ($Size - $ringD) / 2
    $pen    = New-Object System.Drawing.Pen($accent, [math]::Max(1, $Size * 0.06))
    $pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $pen.EndCap   = [System.Drawing.Drawing2D.LineCap]::Round
    $g.DrawEllipse($pen, $ringX, $ringY, $ringD, $ringD)

    # Center dot
    $dotD   = $Size * 0.16
    $dotX   = ($Size - $dotD) / 2
    $dotY   = ($Size - $dotD) / 2
    $dotBrush = New-Object System.Drawing.SolidBrush($accent)
    $g.FillEllipse($dotBrush, $dotX, $dotY, $dotD, $dotD)

    $pen.Dispose()
    $bgBrush.Dispose()
    $dotBrush.Dispose()
    $tilePath.Dispose()
    $g.Dispose()
    return $bmp
}

function Save-Png {
    param([System.Drawing.Bitmap]$bmp, [string]$Path)
    $bmp.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    Write-Host "Generated $Path"
}

function New-IcoFromPng {
    param([string]$PngPath, [string]$IcoPath)

    $png  = [System.IO.File]::ReadAllBytes($PngPath)
    $fs   = [System.IO.File]::Create($IcoPath)
    $bw   = New-Object System.IO.BinaryWriter($fs)

    $bw.Write([uint16]0)         # reserved
    $bw.Write([uint16]1)         # type: icon
    $bw.Write([uint16]1)         # image count
    $bw.Write([byte]0)           # width  (0 == 256)
    $bw.Write([byte]0)           # height (0 == 256)
    $bw.Write([byte]0)           # color count
    $bw.Write([byte]0)           # reserved
    $bw.Write([uint16]1)         # planes
    $bw.Write([uint16]32)        # bits per pixel
    $bw.Write([uint32]$png.Length)  # data size
    $bw.Write([uint32]22)        # offset to data
    $bw.Write($png)

    $bw.Flush()
    $bw.Close()
    $fs.Close()
    Write-Host "Generated $IcoPath"
}

$icon256 = New-AppIcon 256
$icon32  = New-AppIcon 32

Save-Png $icon256 (Join-Path $pngDir 'app-icon-256.png')
Save-Png $icon32  (Join-Path $pngDir 'app-icon-32.png')
New-IcoFromPng (Join-Path $pngDir 'app-icon-256.png') (Join-Path $winDir 'app.ico')

$icon256.Dispose()
$icon32.Dispose()
