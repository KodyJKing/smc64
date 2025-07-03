# Fake a VK_F9 key press to close any injected instances.
# Todo: Replace this with a more robust IPC mechanism.
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

public class Keyboard {
    [DllImport("user32.dll")]
    public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);

    // public const int VK_END = 0x23;
    public const int VK_F9 = 0x78;
    public const int KILL_KEY = VK_F9;
    public const int KEYEVENTF_KEYUP = 0x0002;

    public static void PressKillKey() {
        keybd_event(KILL_KEY, 0, 0, UIntPtr.Zero);
        keybd_event(KILL_KEY, 0, KEYEVENTF_KEYUP, UIntPtr.Zero);
    }
}
"@
[Keyboard]::PressKillKey()