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

# Settings
A settings.ini is created that is used to define the timer as well as which mouse buttons to monitor. Note that if you change any settings you will have to restart the program before they take effect.

# Licence
Do whatever you want with this code and program.
