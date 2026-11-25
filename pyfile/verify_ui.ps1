# 验证 UI：枚举 cpss 主窗口的插件按钮并点击，截图保存到工程根目录
# 所有路径基于脚本所在目录（工程根），不写死盘符
Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, IntPtr dwExtraInfo);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr hWndParent, EnumWindowsProc lpEnumFunc, IntPtr lParam);
    [DllImport("user32.dll", CharSet = CharSet.Auto)] public static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    public const uint MOUSEEVENTF_LEFTDOWN = 0x0002;
    public const uint MOUSEEVENTF_LEFTUP = 0x0004;
    public const int SW_MAXIMIZE = 3;
}
public struct RECT { public int Left, Top, Right, Bottom; }
"@

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# 工程根目录 = 脚本所在目录；截图与日志输出也放这里
$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$exePath = Join-Path $root "src\build\bin\Release\cpss.exe"
$logPath = Join-Path $root "src\build\bin\Release\cpss.log"

$proc = Get-Process cpss -ErrorAction SilentlyContinue
if (-not $proc) {
    if (Test-Path $exePath) {
        Write-Output "cpss 未运行，正在启动: $exePath"
        Start-Process $exePath
        Start-Sleep -Seconds 5
        $proc = Get-Process cpss -ErrorAction SilentlyContinue
    }
    if (-not $proc) { Write-Output "cpss 启动失败"; exit 1 }
}

$hwnd = $proc.MainWindowHandle
[Win32]::ShowWindow($hwnd, [Win32]::SW_MAXIMIZE) | Out-Null
Start-Sleep -Milliseconds 600
[Win32]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Milliseconds 400

$rect = New-Object RECT
[Win32]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
Write-Output "Window: L=$($rect.Left) T=$($rect.Top) R=$($rect.Right) B=$($rect.Bottom)"

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds

function Save-Screenshot($path) {
    $bmp = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
    $bmp.Save($path)
    $g.Dispose(); $bmp.Dispose()
    Write-Output "Saved $path"
}

# BEFORE 截图
Save-Screenshot (Join-Path $root "verify_before.png")

# 枚举子窗口，查找插件按钮（标题含 Data Flow / Hello / Sample）
$buttonInfo = @()
$enumProc = [Win32+EnumWindowsProc]{
    param($hWnd, $lParam)
    if ([Win32]::IsWindowVisible($hWnd)) {
        $sb = New-Object System.Text.StringBuilder 256
        [void][Win32]::GetWindowText($hWnd, $sb, 256)
        $text = $sb.ToString()
        if ($text -like "*Data Flow*" -or $text -like "*Hello*" -or $text -like "*Sample*") {
            $childRect = New-Object RECT
            [Win32]::GetWindowRect($hWnd, [ref]$childRect)
            $cx = ($childRect.Left + $childRect.Right) / 2
            $cy = ($childRect.Top + $childRect.Bottom) / 2
            Write-Output "  Found button '$text' at ($cx, $cy)"
            $buttonInfo += @{ text = $text; x = $cx; y = $cy }
        }
    }
    return $true
}
[Win32]::EnumChildWindows($hwnd, $enumProc, [IntPtr]::Zero) | Out-Null

if ($buttonInfo.Count -eq 0) {
    Write-Output "未找到插件按钮（标题匹配失败），跳过点击"
} else {
    $btn = $buttonInfo[0]
    Write-Output "Clicking first button '$($btn.text)' at ($($btn.x), $($btn.y))"
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($btn.x, $btn.y)
    Start-Sleep -Milliseconds 300
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 80
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 800
    Save-Screenshot (Join-Path $root "verify_after.png")
}

# 复制实际日志 cpss.log（不是已不存在的 trace.log）
if (Test-Path $logPath) {
    Copy-Item $logPath (Join-Path $root "verify_cpss.log") -Force
    Write-Output "日志已复制: verify_cpss.log"
} else {
    Write-Output "未找到日志: $logPath"
}
Write-Output "Done!"
