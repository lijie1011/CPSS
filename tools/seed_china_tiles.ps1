# seed_china_tiles.ps1 - China tiles zoom 1-8, all levels
# Run: powershell.exe -ExecutionPolicy Bypass -File D:\CPSS\tools\seed_china_tiles.ps1

$ErrorActionPreference = 'Stop'
$BASE = 'http://localhost:8082/geoserver/gwc/rest/seed/world_dom.json'
$AUTH = 'admin:geoserver'
$CONTAINER = 'geoserver'
$GWC_DIR = '/opt/geoserver_data/gwc/world_dom_xyz/world_dom/EPSG_4326'
$DEST = 'D:\CPSS\src\3dParty\mapdata\EPSG_4326'
$EXPECT_MIN = 6000

function Log($m) { Write-Output ("[" + (Get-Date -Format 'HH:mm:ss') + "] " + $m) }

$token = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($AUTH))
$headers = @{ 'Authorization' = "Basic $token"; 'Content-Type' = 'application/json' }

# 1. Check container
Log 'Checking GeoServer container...'
if (-not (docker ps --filter "name=$CONTAINER" --format "{{.Names}}")) {
    throw "Container $CONTAINER is not running!"
}
Log 'Container OK'

# 2. Submit seed - ALL zoom levels 1 to 8
$body = @{
    seedRequest = @{
        name = 'world_dom'
        srs = @{ number = 4326 }
        zoomStart = 1
        zoomStop = 8
        format = 'image/png'
        type = 'seed'
        threadCount = 16
        bounds = @{ coords = @{ double = @(73.0, 18.0, 135.0, 54.0) } }
    }
} | ConvertTo-Json -Depth 8

Log 'Submitting seed: China 73-135E / 18-54N, zoom 1-8, 16 threads'
$resp = Invoke-RestMethod -Uri $BASE -Method POST -Headers $headers -Body $body -TimeoutSec 60
Log ('Response: ' + ($resp | ConvertTo-Json -Compress))

# 3. Wait - report every 15s, stable 3 times means done
Log 'Waiting for tiles (every 15s, Ctrl+C to abort)...'
$sw = [Diagnostics.Stopwatch]::StartNew()
$stable = 0
$last = -1
while ($sw.Elapsed.TotalSeconds -lt 14400) {
    Start-Sleep -Seconds 15
    $cnt = [int](docker exec $CONTAINER sh -c "find $GWC_DIR -name '*.png' 2>/dev/null | wc -l" 2>$null)
    if ($cnt -ge $EXPECT_MIN) {
        if ($cnt -eq $last) { $stable++ } else { $stable = 0 }
        $last = $cnt
        Write-Host ("`r[" + (Get-Date -Format 'HH:mm:ss') + "] tiles: " + $cnt + " (stable " + $stable + "/3)   ") -NoNewline
        if ($stable -ge 3) { Write-Host ''; Log 'Seed complete'; break }
    } else {
        $last = -1; $stable = 0
        Write-Host ("`r[" + (Get-Date -Format 'HH:mm:ss') + "] tiles: " + $cnt + " / expect>=" + $EXPECT_MIN + "   ") -NoNewline
    }
}
Write-Host ''
$sw.Stop()
Log ("Elapsed: " + [int]$sw.Elapsed.TotalMinutes + " min")

# 4. Copy tiles from container
Log 'Copying tiles to mapdata...'
if (Test-Path -LiteralPath $DEST) { Remove-Item -LiteralPath $DEST -Recurse -Force }
New-Item -ItemType Directory -Path $DEST -Force | Out-Null

$tmp = 'D:\CPSS\_tile_tmp'
if (Test-Path $tmp) { Remove-Item -LiteralPath $tmp -Recurse -Force }
docker cp "${CONTAINER}:${GWC_DIR}" $tmp
if (-not (Test-Path $tmp)) { throw 'docker cp failed' }

Get-ChildItem -LiteralPath $tmp -Directory | ForEach-Object {
    $srcZ = $_.FullName
    $dstZ = Join-Path $DEST $_.Name
    New-Item -ItemType Directory -Path $dstZ -Force | Out-Null
    Get-ChildItem -LiteralPath $srcZ -Directory | ForEach-Object {
        $srcX = $_.FullName
        $dstX = Join-Path $dstZ $_.Name
        New-Item -ItemType Directory -Path $dstX -Force | Out-Null
        foreach ($f in (Get-ChildItem -LiteralPath $srcX -File -Filter '*.png')) {
            Copy-Item -Path $f.FullName -Destination $dstX -Force
        }
    }
}
Remove-Item -LiteralPath $tmp -Recurse -Force
Log 'Copy done'

# 5. Verify each zoom level 1-8
Log 'Verifying zoom levels:'
$missing = @()
foreach ($z in 1..8) {
    $zd = Join-Path $DEST "$z"
    $cnt = 0
    if (Test-Path $zd) { $cnt = @(Get-ChildItem -LiteralPath $zd -Recurse -File -Filter '*.png' -ErrorAction SilentlyContinue).Count }
    if ($cnt -eq 0) { $missing += $z }
    Write-Output ("  z" + $z + " : " + $cnt + " tiles  " + $(if ($cnt -gt 0) { 'OK' } else { 'MISSING' }))
}
$total = @(Get-ChildItem -LiteralPath $DEST -Recurse -File -Filter '*.png').Count
Log ("Total tiles: " + $total)
if ($missing.Count -eq 0) {
    Log '=== DONE: all zoom levels 1-8 present! ==='
} else {
    Log ('=== WARNING: missing levels: ' + ($missing -join ',') + ' ===')
}
