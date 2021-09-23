# Double Click Monitor
Eliminate double clicks due to failing mouse switches.

# Binaries
Pre-compiled binaries have been uploaded [here](https://github.com/siddhartha77/DoubleClickMonitor/releases).

# Compiling
This solution was created with Microsoft Visual Studio Community 2019.

There are two projects:
* **MouseHook**: The DLL hook that will caption the low level mouse clicks.
* **DoubleClickMonitor**: The systray application.

# How Does It Work?
A DLL intercepts all mouse clicks with [LowLevelMouseProc](https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644986(v=vs.85)). If two clicks are detected within a timer range, the second click is discarded. The Double Click Monitor systray application hooks into the DLL and sets the timer and other settings.

# Will This Set Off Anti-Cheat Software?
I tested Apex Legends, which uses Easy Anti-Cheat, and everything worked fine.

# Settings
A settings.ini is created that is used to define the timer as well as which mouse buttons to monitor. Note that if you change any settings you will have to restart the program before they take effect.

# Bugs
When quitting, the icon won't disappear. I know it's annoying, but it's a Windows "feature". Unless the executable is signed, the icon is tied to the executable path, meaning if you move the program, it won't load. You can [read more about it here](https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona#nim_delete-0x00000002).

# Licence
Do whatever you want with this code and program.
