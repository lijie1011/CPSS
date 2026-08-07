Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
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
[Win32]::SetForegroundWindow($proc.MainWindowHandle)
Start-Sleep -Milliseconds 500

$btnX = $rect.Left + 150
$btnY = $rect.Bottom - 55

Add-Type -AssemblyName System.Windows.Forms
for ($i = 0; $i -lt 5; $i++) {
    $x = $btnX + $i * 120
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($x, $btnY)
    Start-Sleep -Milliseconds 150
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 30
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
    Write-Output "Clicked ($x, $btnY)"
    Start-Sleep -Milliseconds 300
}
Write-Output "Done clicking"
