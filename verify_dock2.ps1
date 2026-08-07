Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, IntPtr dwExtraInfo);
    public const uint MOUSEEVENTF_LEFTDOWN = 0x0002;
    public const uint MOUSEEVENTF_LEFTUP = 0x0004;
    public const int SW_MAXIMIZE = 3;
}
public struct RECT { public int Left, Top, Right, Bottom; }
"@

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Kill all cpss
Get-Process cpss -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 800

# Clear trace.log
Remove-Item "D:\ZCHH\CPSS\src\build\bin\Debug\trace.log" -ErrorAction SilentlyContinue

# Start fresh
Start-Process "D:\ZCHH\CPSS\src\build\bin\Debug\cpss.exe"
Start-Sleep -Seconds 8

$proc = Get-Process cpss -ErrorAction SilentlyContinue
if (-not $proc) { Write-Output "cpss not running!"; exit }

$hwnd = $proc.MainWindowHandle
[Win32]::ShowWindow($hwnd, [Win32]::SW_MAXIMIZE)
Start-Sleep -Milliseconds 600
[Win32]::SetForegroundWindow($hwnd)
Start-Sleep -Milliseconds 400

$rect = New-Object RECT
[Win32]::GetWindowRect($hwnd, [ref]$rect)
$winW = $rect.Right - $rect.Left
$winH = $rect.Bottom - $rect.Top
Write-Output "Window: ${winW}x${winH}"

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds

# Screenshot 1: initial state
$bmp = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bmp.Save("D:\ZCHH\CPSS\dock_01_initial.png")
$g.Dispose(); $bmp.Dispose()
Write-Output "Saved dock_01_initial.png"

# Click bottom toolbar at 3 positions sequentially
$baseY = $rect.Bottom - 35
$toolbarStartX = $rect.Left + ($winW / 2) - 200
$stepX = 120

for ($i = 0; $i -lt 3; $i++) {
    $x = $toolbarStartX + ($i * $stepX)
    $y = $baseY
    Write-Output "Click $i at ($x, $y)"
    
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($x, $y)
    Start-Sleep -Milliseconds 300
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 60
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 900

    $bmp = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
    $fname = "D:\ZCHH\CPSS\dock_02_after_click_${i}.png"
    $bmp.Save($fname)
    $g.Dispose(); $bmp.Dispose()
    Write-Output "Saved $fname"
}

Copy-Item "D:\ZCHH\CPSS\src\build\bin\Debug\trace.log" "D:\ZCHH\CPSS\dock_trace2.log" -ErrorAction SilentlyContinue
Write-Output "Done!"
