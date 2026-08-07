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
Write-Output "Win: L=$($rect.Left) T=$($rect.Top) R=$($rect.Right) B=$($rect.Bottom) W=$($rect.Right-$rect.Left) H=$($rect.Bottom-$rect.Top)"

[Win32]::SetForegroundWindow($proc.MainWindowHandle)
[Win32]::ShowWindow($proc.MainWindowHandle, 3)
Start-Sleep -Milliseconds 800

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Save before screenshot
$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bmp1 = New-Object System.Drawing.Bitmap($bounds.Width, $bounds.Height)
$g1 = [System.Drawing.Graphics]::FromImage($bmp1)
$g1.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
$bmp1.Save("D:\ZCHH\CPSS\f1_before.png")
$g1.Dispose(); $bmp1.Dispose()
Write-Output "Saved f1_before.png"

# Try clicking multiple points along bottom toolbar
$winW = $rect.Right - $rect.Left
$winH = $rect.Bottom - $rect.Top
$toolbarY = $rect.Bottom - 25

# Click at 3 different X positions along the toolbar
$positions = @(
    @{ x = $rect.Left + 120; y = $toolbarY },
    @{ x = $rect.Left + 250; y = $toolbarY },
    @{ x = $rect.Left + 380; y = $toolbarY },
    @{ x = $rect.Left + 500; y = $toolbarY }
)

foreach ($pos in $positions) {
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($pos.x, $pos.y)
    Start-Sleep -Milliseconds 150
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 40
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
    Write-Output "  Clicked ($($pos.x), $($pos.y))"
    Start-Sleep -Milliseconds 400

    # Check if plugin is showing by taking mini screenshot
    $bmpMid = New-Object System.Drawing.Bitmap($bounds.Width, $bounds.Height)
    $gMid = [System.Drawing.Graphics]::FromImage($bmpMid)
    $gMid.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    $midName = "D:\ZCHH\CPSS\f1_click_x$($pos.x).png"
    $bmpMid.Save($midName)
    $gMid.Dispose(); $bmpMid.Dispose()
    Write-Output "  Saved $midName"
}

Write-Output "All clicks done"
