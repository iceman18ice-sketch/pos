"""Force Sales Manager to front using VB AppActivate, then capture menus."""
import subprocess

# Use a VBScript-based approach to force foreground since SetForegroundWindow
# is blocked by the calling process not being the foreground thread
ps_script = r'''
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Mouse5 {
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int X, int Y);
    [DllImport("user32.dll")] 
    public static extern void mouse_event(uint f, uint dx, uint dy, uint d, UIntPtr e);
    [DllImport("user32.dll")] public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);
    [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);
    [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
    
    public static void ForceSetForeground(IntPtr hWnd) {
        IntPtr fg = GetForegroundWindow();
        uint fgThread = GetWindowThreadProcessId(fg, out _);
        uint curThread = GetCurrentThreadId();
        
        if (fgThread != curThread) {
            AttachThreadInput(curThread, fgThread, true);
            SetForegroundWindow(hWnd);
            AttachThreadInput(curThread, fgThread, false);
        } else {
            SetForegroundWindow(hWnd);
        }
        ShowWindow(hWnd, 5); // SW_SHOW
    }
}
"@

function ClickAt($x, $y) {
    [Mouse5]::SetCursorPos($x, $y)
    Start-Sleep -Milliseconds 150
    [Mouse5]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 30
    [Mouse5]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
}

function Screenshot($path) {
    $sw = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width
    $sh = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height
    $bmp = New-Object System.Drawing.Bitmap($sw, $sh)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen(0, 0, 0, 0, (New-Object System.Drawing.Size($sw, $sh)))
    $g.Dispose()
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}

function BringToFront($hwnd) {
    [Mouse5]::ForceSetForeground($hwnd)
    Start-Sleep -Milliseconds 300
}

$proc = Get-Process -Name "SalesManager" -ErrorAction SilentlyContinue | Select -First 1
if (-not $proc) { Write-Host "NOT FOUND"; exit 1 }
$hwnd = $proc.MainWindowHandle
Write-Host "SalesManager HWND=$hwnd PID=$($proc.Id)"

$outDir = "c:\Users\HP\.gemini\antigravity\scratch\pos-system\sm_full"
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Force -Path $outDir | Out-Null }

# Use AppActivate to bring Sales Manager to front
try {
    [Microsoft.VisualBasic.Interaction]::AppActivate($proc.Id)
} catch { Write-Host "AppActivate failed: $_" }
Start-Sleep -Milliseconds 500
BringToFront $hwnd
Start-Sleep -Milliseconds 500

# Verify foreground
$fg = [Mouse5]::GetForegroundWindow()
Write-Host "Foreground after activate: $fg (want $hwnd)"

# Take initial screenshot
Screenshot "$outDir\s00_initial.png"
Write-Host "Captured initial state"

# Now capture each menu
$menuY = 40
$menus = @(
    @{N="s01_ملف"; X=1340},
    @{N="s02_العملاء"; X=1200},
    @{N="s03_المخازن"; X=1100},
    @{N="s04_المشتريات"; X=1025},
    @{N="s05_المبيعات"; X=960},
    @{N="s06_الحسابات"; X=895},
    @{N="s07_الموظفين"; X=830},
    @{N="s08_الصيانة"; X=767},
    @{N="s09_التقارير"; X=705},
    @{N="s10_اتصل"; X=640}
)

foreach ($m in $menus) {
    Write-Host "Menu: $($m.N)"
    
    # Force to front before each click
    BringToFront $hwnd
    try { [Microsoft.VisualBasic.Interaction]::AppActivate($proc.Id) } catch {}
    Start-Sleep -Milliseconds 300
    
    ClickAt $m.X $menuY
    Start-Sleep -Milliseconds 1500
    
    Screenshot "$outDir\$($m.N).png"
    
    [System.Windows.Forms.SendKeys]::SendWait("{ESC}")
    Start-Sleep -Milliseconds 500
    Write-Host "  Done"
}

# Navigate to Home page
Write-Host "`nCapturing Home tile page..."
BringToFront $hwnd
try { [Microsoft.VisualBasic.Interaction]::AppActivate($proc.Id) } catch {}
Start-Sleep -Milliseconds 300
ClickAt 1310 71
Start-Sleep -Milliseconds 800
Screenshot "$outDir\s00_home.png"

Write-Host "`nAll done!"
'''

result = subprocess.run(
    ['powershell', '-ExecutionPolicy', 'Bypass', '-Command', ps_script],
    capture_output=True, text=True, timeout=180
)
print(result.stdout)
if result.stderr:
    lines = [l for l in result.stderr.split('\n') if l.strip()]
    if lines:
        print("WARN:", '\n'.join(lines[:5]))
