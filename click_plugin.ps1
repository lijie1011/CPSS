Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr hWnd, int x, int y, int w, int h, bool repaint);
    [DllImport("user32.dll")] public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, IntPtr dwExtraInfo);
    public const uint MOUSEEVENTF_LEFTDOWN = 0x0002;
    public const uint MOUSEEVENTF_LEFTUP = 0x0004;
}
public struct RECT { public int Left, Top, Right, Bottom; }
"@

$proc = Get-Process cpss -ErrorAction SilentlyContinue
if (-not $proc) { Write-Output "cpss not running"; exit }

$rect = New-Object RECT
[Win32]::GetWindowRect($proc.MainWindowHandle, [ref]$rect)
Write-Output "Window: Left=$($rect.Left) Top=$($rect.Top) Right=$($rect.Right) Bottom=$($rect.Bottom) W=$($rect.Right-$rect.Left) H=$($rect.Bottom-$rect.Top)"

[Win32]::SetForegroundWindow($proc.MainWindowHandle)
[Win32]::ShowWindow($proc.MainWindowHandle, 3)
Start-Sleep -Milliseconds 800

$winW = $rect.Right - $rect.Left
$winH = $rect.Bottom - $rect.Top

$bottomBarY = $rect.Bottom - 30
$firstBtnX = $rect.Left + 80
Write-Output "Clicking at ($firstBtnX, $bottomBarY)"

Add-Type -AssemblyName System.Windows.Forms
[System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($firstBtnX, $bottomBarY)
Start-Sleep -Milliseconds 200
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 50
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 500
Write-Output "Click done"
