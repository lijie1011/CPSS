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
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    public const uint MOUSEEVENTF_LEFTDOWN = 0x0002;
    public const uint MOUSEEVENTF_LEFTUP = 0x0004;
    public const uint MOUSEEVENTF_MOVE = 0x0001;
    public const int SW_MAXIMIZE = 3;
    public const int SW_RESTORE = 9;
}
public struct RECT { public int Left, Top, Right, Bottom; }
public struct POINT { public int X, Y; }
"@

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$proc = Get-Process cpss -ErrorAction SilentlyContinue
if (-not $proc) { Write-Output "cpss not running"; exit }

$hwnd = $proc.MainWindowHandle
Write-Output "Main window handle: $hwnd"

# Maximize window
[Win32]::ShowWindow($hwnd, [Win32]::SW_MAXIMIZE)
Start-Sleep -Milliseconds 800
[Win32]::SetForegroundWindow($hwnd)
Start-Sleep -Milliseconds 500

$rect = New-Object RECT
[Win32]::GetWindowRect($hwnd, [ref]$rect)
Write-Output "Window rect: L=$($rect.Left) T=$($rect.Top) R=$($rect.Right) B=$($rect.Bottom)"
$winW = $rect.Right - $rect.Left
$winH = $rect.Bottom - $rect.Top
Write-Output "Window size: ${winW}x${winH}"

# Take BEFORE screenshot
$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bmp1 = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
$g1 = [System.Drawing.Graphics]::FromImage($bmp1)
$g1.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bmp1.Save("D:\ZCHH\CPSS\verify_before.png")
$g1.Dispose(); $bmp1.Dispose()
Write-Output "Saved verify_before.png"

# Enumerate child windows to find toolbar buttons
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
            $buttonInfo += @{ text = $text; x = $cx; y = $cy; rect = $childRect }
        }
    }
    return $true
}
[Win32]::EnumChildWindows($hwnd, $enumProc, [IntPtr]::Zero)

if ($buttonInfo.Count -eq 0) {
    Write-Output "No toolbar buttons found via EnumChildWindows"
    Write-Output "Trying click at estimated position..."
    # Toolbar is at bottom of window, buttons are in center-ish
    $clickX = $rect.Left + ($winW / 2) - 100
    $clickY = $rect.Bottom - 35
} else {
    $firstBtn = $buttonInfo[0]
    $clickX = $firstBtn.x
    $clickY = $firstBtn.y
    Write-Output "Clicking first button '$($firstBtn.text)' at ($clickX, $clickY)"
}

# Perform click
[System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($clickX, $clickY)
Start-Sleep -Milliseconds 300
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 80
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
Write-Output "Clicked at ($clickX, $clickY)"

Start-Sleep -Milliseconds 800

# Take AFTER screenshot
$bmp2 = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
$g2 = [System.Drawing.Graphics]::FromImage($bmp2)
$g2.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bmp2.Save("D:\ZCHH\CPSS\verify_after.png")
$g2.Dispose(); $bmp2.Dispose()
Write-Output "Saved verify_after.png"

# Also save trace.log copy
Copy-Item "D:\ZCHH\CPSS\src\build\bin\Debug\trace.log" "D:\ZCHH\CPSS\verify_trace.log" -ErrorAction SilentlyContinue
Write-Output "Done!"
