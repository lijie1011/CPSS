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
[Win32]::ShowWindow($proc.MainWindowHandle, 3)
Start-Sleep -Milliseconds 800

$winW = $rect.Right - $rect.Left
$winH = $rect.Bottom - $rect.Top

# Bottom toolbar buttons (3 buttons), click around center-bottom of window
$centerX = $rect.Left + $winW / 2
$btnY = $rect.Bottom - 25

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Screenshot before click
$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap($bounds.Width, $bounds.Height)
$g = [System.Drawing.Graphics]::FromImage($bitmap)
$g.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
$bitmap.Save("D:\ZCHH\CPSS\screen_before.png")
$g.Dispose(); $bitmap.Dispose()
Write-Output "Saved screen_before.png"

# Click first plugin button (Data Flow Monitor)
# Toolbar buttons typically start from left side
$clickX = $rect.Left + 200
[System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($clickX, $btnY)
Start-Sleep -Milliseconds 200
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 50
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
Write-Output "Clicked at ($clickX, $btnY)"
Start-Sleep -Milliseconds 500

# Screenshot after click
$bitmap2 = New-Object System.Drawing.Bitmap($bounds.Width, $bounds.Height)
$g2 = [System.Drawing.Graphics]::FromImage($bitmap2)
$g2.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
$bitmap2.Save("D:\ZCHH\CPSS\screen_after.png")
$g2.Dispose(); $bitmap2.Dispose()
Write-Output "Saved screen_after.png"
