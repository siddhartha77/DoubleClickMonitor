extern "C" {
	void __declspec(dllexport) __cdecl SetMouseHook(INT nMonitorButtons, INT nTimeoutMilliseconds);
	void __declspec(dllexport) __cdecl RemoveMouseHook(void);
	unsigned long __declspec(dllexport) __cdecl GetMouseHookCaughtCount(void);
}

#define MHK_LEFT_MOUSE_BUTTON 0x1
#define MHK_RIGHT_MOUSE_BUTTON 0x10
#define MHK_MIDDLE_MOUSE_BUTTON 0x100

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL CheckTicks(DWORD* dwPreviousTicks);