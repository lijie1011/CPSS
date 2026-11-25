# seed_eastsea.ps1 - Fill missing East China Sea tiles zoom 1-8
# Run: powershell.exe -ExecutionPolicy Bypass -File D:\CPSS\tools\seed_eastsea.ps1

$ErrorActionPreference = 'Stop'
$BASE = 'http://localhost:8082/geoserver/gwc/rest/seed/world_dom.json'
$AUTH = 'admin:geoserver'
$CONTAINER = 'geoserver'
$GWC_DIR = '/opt/geoserver_data/gwc/world_dom_xyz/world_dom/EPSG_4326'
$DEST = 'D:\CPSS\src\3dParty\mapdata\EPSG_4326'

function Log($m) { Write-Output ("[" + (Get-Date -Format 'HH:mm:ss') + "] " + $m) }
$token = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($AUTH))
$h = @{ 'Authorization' = "Basic $token"; 'Content-Type' = 'application/json' }

if (-not (docker ps --filter "name=$CONTAINER" --format "{{.Names}}")) { throw 'GeoServer container not running' }

# Count East Sea tiles before
$lon0=118; $lon1=132; $lat0=18; $lat1=38
function CountEastSea($root) {
    $total = 0
    foreach ($z in 1..8) {
        $zd = Join-Path $root "$z"
        if (-not (Test-Path $zd)) { continue }
        $n=[math]::Pow(2,$z)
        $c0=[math]::Floor(($lon0+180)/360*2*$n); $c1=[math]::Floor(($lon1+180)/360*2*$n)
        $r0=[math]::Floor((90-$lat1)/180*$n); $r1=[math]::Floor((90-$lat0)/180*$n)
        for ($c=$c0; $c -le $c1; $c++) {
            for ($r=$r0; $r -le $r1; $r++) {
                if (Test-Path (Join-Path $zd "$c\$r.png")) { $total++ }
            }
        }
    }
    return $total
}
$before = CountEastSea $DEST
Log "East Sea tiles before: $before"

# Submit seed for East Sea, zoom 1-8, larger bounds
$body = @{
    seedRequest = @{
        name = 'world_dom'
        srs = @{ number = 4326 }
        zoomStart = 1
        zoomStop = 8
        format = 'image/png'
        type = 'seed'
        threadCount = 16
        bounds = @{ coords = @{ double = @(115.0, 15.0, 135.0, 42.0) } }
    }
} | ConvertTo-Json -Depth 8

Log 'Submitting seed: East Sea 115-135E / 15-42N, zoom 1-8'
$null = Invoke-RestMethod -Uri $BASE -Method POST -Headers $h -Body $body -TimeoutSec 60

# Wait: poll GWC status - all thread counters should show finished (value[4]=1 or no entries)
Log 'Waiting for GWC seed to finish (checking status every 20s)...'
$sw = [Diagnostics.Stopwatch]::StartNew()
while ($sw.Elapsed.TotalSeconds -lt 14400) {
    Start-Sleep -Seconds 20
    try {
        $st = Invoke-RestMethod -Uri $BASE -Headers $h -TimeoutSec 30
        $running = 0
        if ($st.'long-array-array') {
            foreach ($row in $st.'long-array-array') {
                # format: [completed, total, failed, threadId, status(1=done,2=running)]
                if ($row[4] -eq 2) { $running++ }
            }
        }
        $cnt = [int](docker exec $CONTAINER sh -c "find $GWC_DIR -name '*.png' 2>/dev/null | wc -l" 2>$null)
        Write-Host ("`r[" + (Get-Date -Format 'HH:mm:ss') + "] running_threads=" + $running + " total_tiles=" + $cnt + "   ") -NoNewline
        if ($running -eq 0 -and $sw.Elapsed.TotalSeconds -gt 60) { Write-Host ''; Log 'Seed finished'; break }
    } catch { Write-Host '.' }
}
Write-Host ''

# Copy tiles (merge, not overwrite whole dir - keep other regions)
Log 'Copying tiles (merge)...'
$tmp = 'D:\CPSS\_tile_tmp2'
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

$after = CountEastSea $DEST
Log "East Sea tiles after: $after (was $before)"
if ($after -gt $before) { Log 'SUCCESS: East Sea tiles added' } else { Log 'WARNING: no improvement' }
