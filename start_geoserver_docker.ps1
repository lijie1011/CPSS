# ============================================================
# 重启电脑后运行此脚本：验证 Docker 并启动 GeoServer 容器
# 用法：右键"使用 PowerShell 运行"，或 powershell -ExecutionPolicy Bypass -File start_geoserver_docker.ps1
# ============================================================

$DOCKER = 'D:\app\docker\resources\bin\docker.exe'
$GEO_DIR = 'D:\CPSS\src\3dParty\geoserver\geoserver'

Write-Output '=== 1. 检查 Docker 引擎 ==='
& $DOCKER version --format 'Client={{.Client.Version}}' 2>&1 | Select-Object -First 1
$engine = & $DOCKER info --format '{{.ServerVersion}}' 2>&1
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($engine)) {
    Write-Output 'Docker 引擎未启动，正在启动 Docker Desktop...'
    Start-Process -FilePath 'D:\app\docker\Docker Desktop.exe' | Out-Null
    Write-Output '等待 60 秒...'
    Start-Sleep -Seconds 60
    $engine = & $DOCKER info --format '{{.ServerVersion}}' 2>&1
}
if ($LASTEXITCODE -ne 0) {
    Write-Output 'ERROR: Docker 引擎仍无法启动，请确认已重启电脑。'
    exit 1
}
Write-Output ("Docker 引擎 OK: Server=" + $engine)

Write-Output '=== 2. 启动 GeoServer 容器 ==='
$existing = & $DOCKER ps -a --filter name=geoserver --format '{{.Names}}' 2>&1
if ($existing -contains 'geoserver') {
    Write-Output '容器已存在，启动...'
    & $DOCKER start geoserver
} else {
    Write-Output '创建新容器...'
    & $DOCKER run -d --name geoserver --restart always `
        -p 8082:8080 `
        --mount "type=bind,src=$GEO_DIR\geoserverData,target=/opt/geoserver_data" `
        --mount "type=bind,src=$GEO_DIR\web.xml,target=/usr/local/tomcat/webapps/geoserver/WEB-INF/web.xml,readonly" `
        --mount "type=bind,src=$GEO_DIR\WenQuanYi Zen Hei Sharp Regular,target=/usr/share/fonts/truetype/custom" `
        -e 'CATALINA_OPTS=-Xms4g -Xmx8g -XX:+UseG1GC' `
        docker.osgeo.org/geoserver:2.27.x
}

Write-Output '=== 3. 等待服务就绪 ==='
Write-Output '等待 30 秒...'
Start-Sleep -Seconds 30
& $DOCKER ps --filter name=geoserver --format 'table {{.Names}}\t{{.Status}}\t{{.Ports}}'

Write-Output '=== 4. 验证 HTTP 访问 ==='
try {
    $r = Invoke-WebRequest -Uri 'http://localhost:8082/geoserver/rest/about/version.json' -UseBasicParsing -TimeoutSec 10
    Write-Output ("GeoServer HTTP OK: " + $r.StatusCode)
} catch {
    Write-Output ("GeoServer HTTP 尚未就绪: " + $_.Exception.Message)
}

Write-Output ''
Write-Output '完成后访问: http://localhost:8082/geoserver'
