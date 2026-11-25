# copy_tiles_from_docker.ps1 - Copy all tiles from GeoServer blobstore to mapdata
# Run: powershell.exe -ExecutionPolicy Bypass -File D:\CPSS\tools\copy_tiles_from_docker.ps1

$ErrorActionPreference = 'Stop'
$CONTAINER = 'geoserver'
$GWC_DIR = '/opt/geoserver_data/gwc/world_dom_xyz/world_dom/EPSG_4326'
$DEST = 'D:\CPSS\src\3dParty\mapdata\EPSG_4326'

function Log($m) { Write-Output ("[" + (Get-Date -Format 'HH:mm:ss') + "] " + $m) }

# Count before
$before = @(Get-ChildItem -LiteralPath $DEST -Recurse -File -Filter '*.png' -ErrorAction SilentlyContinue).Count
Log "Local tiles before: $before"

# Copy from container
Log 'Copying from Docker container...'
$tmp = 'D:\CPSS\_tile_tmp3'
if (Test-Path $tmp) { Remove-Item -LiteralPath $tmp -Recurse -Force }
docker cp "${CONTAINER}:${GWC_DIR}" $tmp
if (-not (Test-Path $tmp)) { throw 'docker cp failed' }

$srcTotal = @(Get-ChildItem -LiteralPath $tmp -Recurse -File -Filter '*.png').Count
Log "Container tiles: $srcTotal"

# Merge copy: use -Path (supports wildcards) not -LiteralPath
$copied = 0
Get-ChildItem -LiteralPath $tmp -Directory | ForEach-Object {
    $z = $_.Name
    $dstZ = Join-Path $DEST $z
    New-Item -ItemType Directory -Path $dstZ -Force | Out-Null
    Get-ChildItem -LiteralPath $_.FullName -Directory | ForEach-Object {
        $x = $_.Name
        $dstX = Join-Path $dstZ $x
        New-Item -ItemType Directory -Path $dstX -Force | Out-Null
        $pngs = Get-ChildItem -LiteralPath $_.FullName -File -Filter '*.png'
        foreach ($f in $pngs) {
            Copy-Item -Path $f.FullName -Destination $dstX -Force
            $copied++
        }
    }
}
Log "Copied: $copied files"

Remove-Item -LiteralPath $tmp -Recurse -Force

# Verify per zoom level
Log 'Verifying zoom levels:'
foreach ($z in 1..10) {
    $zd = Join-Path $DEST "$z"
    $cnt = if (Test-Path $zd) { @(Get-ChildItem -LiteralPath $zd -Recurse -File -Filter '*.png').Count } else { 0 }
    Write-Output ("  z$z : " + $cnt + " tiles")
}

$after = @(Get-ChildItem -LiteralPath $DEST -Recurse -File -Filter '*.png').Count
Log "Local tiles after: $after (was $before)"
