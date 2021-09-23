#include "pch.h"
#include "mousehook.h"

static unsigned long    g_ulCaughtCount = 0;
static int              g_nMonitorButtons = 0;
static int              g_nTimeoutMilliseconds = 0;
static DWORD            g_dwLMBPreviousTicks = 0;
static DWORD            g_dwRMBPreviousTicks = 0;
static DWORD            g_dwMMBPreviousTicks = 0;

HHOOK hkKey = NULL;
HINSTANCE hInstHookDll = NULL;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved) {
    switch (reasonForCall) {
        case DLL_PROCESS_ATTACH:
            hInstHookDll = (HINSTANCE)hModule;
            break;
    }
    return TRUE;
}

void __declspec(dllexport) __cdecl SetMouseHook(INT nMonitorButtons, INT nTimeoutMilliseconds) {
    if (hkKey == NULL)
        hkKey = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstHookDll, 0);

    g_nMonitorButtons = nMonitorButtons;
    g_nTimeoutMilliseconds = nTimeoutMilliseconds;
}

void __declspec(dllexport) __cdecl RemoveMouseHook(void) {
    if (hkKey != NULL)
        UnhookWindowsHookEx(hkKey);

    hkKey = NULL;

    g_ulCaughtCount = 0;
}

unsigned long __declspec(dllexport) __cdecl GetMouseHookCaughtCount(void) {
    return g_ulCaughtCount;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN && (g_nMonitorButtons & MHK_LEFT_MOUSE_BUTTON)) {
            if (CheckTicks(&g_dwLMBPreviousTicks)) {
                // Ignore the click if it falls within the timeout period
                return 1;
            }
        }
        if (wParam == WM_RBUTTONDOWN && (g_nMonitorButtons & MHK_RIGHT_MOUSE_BUTTON)) {
            if (CheckTicks(&g_dwRMBPreviousTicks)) {
                return 1;
            }
        }
        if (wParam == WM_MBUTTONDOWN && (g_nMonitorButtons & MHK_MIDDLE_MOUSE_BUTTON)) {
            if (CheckTicks(&g_dwMMBPreviousTicks)) {
                return 1;
            }
        }
    }

    return CallNextHookEx(hkKey, nCode, wParam, lParam);
}

BOOL CheckTicks(DWORD *dwPreviousTicks) {
    DWORD dwTicks = GetTickCount();
    DWORD dwTickDelta = dwTicks - *dwPreviousTicks;

    *dwPreviousTicks = dwTicks;

    if (dwTickDelta <= (DWORD)g_nTimeoutMilliseconds) {
        g_ulCaughtCount++;
        return TRUE;
    }
    else {
        return FALSE;
    }
       
}