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

$proc = Get-Process cpss -ErrorAction SilentlyContinue
if (-not $proc) { Write-Output "cpss not running"; exit }

$hwnd = $proc.MainWindowHandle
[Win32]::ShowWindow($hwnd, [Win32]::SW_MAXIMIZE)
Start-Sleep -Milliseconds 600
[Win32]::SetForegroundWindow($hwnd)
Start-Sleep -Milliseconds 400

$rect = New-Object RECT
[Win32]::GetWindowRect($hwnd, [ref]$rect)
Write-Output "Window: L=$($rect.Left) T=$($rect.Top) W=$($rect.Right-$rect.Left) H=$($rect.Bottom-$rect.Top)"

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds

# Save initial screenshot
$bmp0 = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
$g0 = [System.Drawing.Graphics]::FromImage($bmp0)
$g0.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bmp0.Save("D:\ZCHH\CPSS\dock_00_initial.png")
$g0.Dispose(); $bmp0.Dispose()
Write-Output "Saved dock_00_initial.png"

# Find plugin buttons by enumerating child windows
$buttons = @()
$enumProc = [Win32+EnumWindowsProc]{
    param($hWnd, $lParam)
    if ([Win32]::IsWindowVisible($hWnd)) {
        $sb = New-Object System.Text.StringBuilder 256
        [void][Win32]::GetWindowText($hWnd, $sb, 256)
        $text = $sb.ToString()
        if ($text -like "*Data Flow*" -or $text -like "*Hello*" -or $text -like "*Sample*") {
            $r = New-Object RECT
            [Win32]::GetWindowRect($hWnd, [ref]$r)
            $cx = ($r.Left + $r.Right) / 2
            $cy = ($r.Top + $r.Bottom) / 2
            $buttons += @{ text=$text; x=$cx; y=$cy }
            Write-Output "  Button: '$text' at ($cx,$cy)"
        }
    }
    return $true
}
[Win32]::EnumChildWindows($hwnd, $enumProc, [IntPtr]::Zero)

if ($buttons.Count -eq 0) {
    Write-Output "No buttons found - trying estimated position"
    # Click bottom toolbar center
    $clickX = $rect.Left + ($rect.Right - $rect.Left) / 2 - 150
    $clickY = $rect.Bottom - 35
    $buttons = @(@{ text="estimated"; x=$clickX; y=$clickY })
}

# Click first plugin button (Data Flow Monitor)
$btn = $buttons[0]
Write-Output "Clicking '$($btn.text)' at ($($btn.x), $($btn.y))"
[System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($btn.x, $btn.y)
Start-Sleep -Milliseconds 300
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 60
[Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 800

$bmp1 = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
$g1 = [System.Drawing.Graphics]::FromImage($bmp1)
$g1.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bmp1.Save("D:\ZCHH\CPSS\dock_01_after_first.png")
$g1.Dispose(); $bmp1.Dispose()
Write-Output "Saved dock_01_after_first.png"

# Click second plugin button (Hello)
if ($buttons.Count -ge 2) {
    $btn2 = $buttons[1]
    Write-Output "Clicking '$($btn2.text)' at ($($btn2.x), $($btn2.y))"
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point($btn2.x, $btn2.y)
    Start-Sleep -Milliseconds 300
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 60
    [Win32]::mouse_event([Win32]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 800

    $bmp2 = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
    $g2 = [System.Drawing.Graphics]::FromImage($bmp2)
    $g2.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
    $bmp2.Save("D:\ZCHH\CPSS\dock_02_after_second.png")
    $g2.Dispose(); $bmp2.Dispose()
    Write-Output "Saved dock_02_after_second.png"
}

Copy-Item "D:\ZCHH\CPSS\src\build\bin\Debug\trace.log" "D:\ZCHH\CPSS\dock_trace.log" -ErrorAction SilentlyContinue
Write-Output "Done!"
