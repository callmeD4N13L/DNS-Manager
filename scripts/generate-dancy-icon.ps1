# ---------------------------------------------------------------------------
# generate-dancy-icon.ps1
#
# Draws the self-generated "Dancy" project icon (teal network-globe badge
# with an abstract dancer + "Dancy" wordmark) and stores it as
# assets/Dancy.png (1024x1024, transparent background).
#
# This is a clean-room recreation used as the project icon source of truth.
# To swap in designer artwork later, overwrite assets/Dancy.png and re-run:
#   .\scripts\generate-icons.ps1 -SourceImage assets\Dancy.png
#
# Requires: Windows PowerShell 5.1 with System.Drawing.
# ---------------------------------------------------------------------------

Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$outPath = Join-Path $root 'assets\Dancy.png'

$S = 1024
$cx = 512.0
$cy = 512.0

$deep  = [System.Drawing.Color]::FromArgb(255, 14, 90, 117)   # #0E5A75 outer ring
$mid   = [System.Drawing.Color]::FromArgb(255, 30, 127, 160)  # #1E7FA0 orbits
$light = [System.Drawing.Color]::FromArgb(255, 143, 216, 236) # #8FD8EC dancer
$pale  = [System.Drawing.Color]::FromArgb(255, 201, 239, 249) # #C9EFF9 highlights

$bmp = New-Object System.Drawing.Bitmap -ArgumentList @($S, $S, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb))
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.Clear([System.Drawing.Color]::Transparent)

function New-Pen($color, $w) {
    $p = New-Object System.Drawing.Pen -ArgumentList @($color, $w)
    $p.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $p.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $p.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round
    return $p
}

function New-Pt($x, $y) {
    return New-Object System.Drawing.PointF -ArgumentList @([float]$x, [float]$y)
}

# --- outer badge rings -------------------------------------------------------
$ring = New-Pen $deep 46
$g.DrawEllipse($ring, 42, 42, ($S - 84), ($S - 84))
$ring.Dispose()

$ring2 = New-Pen $mid 16
$g.DrawEllipse($ring2, 118, 118, ($S - 236), ($S - 236))
$ring2.Dispose()

# --- orbit ellipses (rotated) --------------------------------------------------
foreach ($angle in @(35, -35)) {
    $state = $g.Save()
    $m = New-Object System.Drawing.Drawing2D.Matrix
    $m.RotateAt([float]$angle, (New-Pt $cx $cy))
    $g.Transform = $m
    $orbit = New-Pen $mid 13
    $g.DrawEllipse($orbit, 150, 250, ($S - 300), ($S - 500))
    $orbit.Dispose()
    $g.Restore($state)
}

# --- network web: nodes on a circle + chords ---------------------------------
$nodes = @()
$n = 12
$nr = 300.0
for ($i = 0; $i -lt $n; $i++) {
    $a = [math]::PI * 2 * $i / $n - [math]::PI / 2
    $nx = $cx + $nr * [math]::Cos($a)
    $ny = ($cy - 20) + $nr * [math]::Sin($a)
    $nodes += New-Pt $nx $ny
}
$web = New-Pen $mid 7
for ($i = 0; $i -lt $n; $i++) {
    $g.DrawLine($web, $nodes[$i], $nodes[($i + 1) % $n])
    $g.DrawLine($web, $nodes[$i], $nodes[($i + 4) % $n])
}
$web.Dispose()
$nodeFill = New-Object System.Drawing.SolidBrush -ArgumentList @($light)
$nodeEdge = New-Pen $deep 6
foreach ($pt in $nodes) {
    $g.FillEllipse($nodeFill, ($pt.X - 20), ($pt.Y - 20), 40, 40)
    $g.DrawEllipse($nodeEdge, ($pt.X - 20), ($pt.Y - 20), 40, 40)
}
$nodeFill.Dispose()
$nodeEdge.Dispose()

# --- abstract dancer (flowing ribbon body) -------------------------------------
$body = New-Pen $pale 48
$g.DrawBezier($body, (New-Pt 566 452), (New-Pt 496 528), (New-Pt 648 566), (New-Pt 560 648))
$body.Dispose()

$torso = New-Pen $light 44
$g.DrawBezier($torso, (New-Pt 560 648), (New-Pt 500 706), (New-Pt 524 762), (New-Pt 560 824))
$torso.Dispose()

# head
$headBrush = New-Object System.Drawing.SolidBrush -ArgumentList @($pale)
$g.FillEllipse($headBrush, 530, 336, 72, 72)
$headBrush.Dispose()

# raised arm
$arm = New-Pen $light 30
$g.DrawBezier($arm, (New-Pt 566 500), (New-Pt 620 470), (New-Pt 660 440), (New-Pt 712 408))
$arm.Dispose()

# extended leg
$leg = New-Pen $light 34
$g.DrawLine($leg, (New-Pt 560 668), (New-Pt 688 742))
$leg.Dispose()

# skirt swirl around the waist
$swirl = New-Pen $light 26
$g.DrawArc($swirl, 392, 556, 336, 150, 10, 300)
$swirl.Dispose()

# --- "Dancy" wordmark across the lower badge ----------------------------------
$font = New-Object System.Drawing.Font -ArgumentList @('Segoe UI', 148, ([System.Drawing.FontStyle]::Bold))
$fmt = New-Object System.Drawing.StringFormat
$fmt.Alignment = [System.Drawing.StringAlignment]::Center
$fmt.LineAlignment = [System.Drawing.StringAlignment]::Center
$rectW = $S - 120
$textRect = New-Object System.Drawing.RectangleF -ArgumentList @([float]60, [float]760, [float]$rectW, [float]190)
$edge = New-Object System.Drawing.SolidBrush -ArgumentList @($deep)
foreach ($o in @(-5, 5)) {
    $rx = New-Object System.Drawing.RectangleF -ArgumentList @([float](60 + $o), [float]760, [float]$rectW, [float]190)
    $g.DrawString('Dancy', $font, $edge, $rx, $fmt)
    $ry = New-Object System.Drawing.RectangleF -ArgumentList @([float]60, [float](760 + $o), [float]$rectW, [float]190)
    $g.DrawString('Dancy', $font, $edge, $ry, $fmt)
}
$fill = New-Object System.Drawing.SolidBrush -ArgumentList @($pale)
$g.DrawString('Dancy', $font, $fill, $textRect, $fmt)
$fill.Dispose()
$edge.Dispose()
$font.Dispose()
$fmt.Dispose()

$g.Dispose()
$bmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Host "Generated $outPath"
