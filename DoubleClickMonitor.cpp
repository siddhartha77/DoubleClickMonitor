// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#include "resource.h"
#include <windows.h>
#include <WinBase.h>
#include <shellapi.h>
#include <commctrl.h>
#include <comdef.h>
#include <strsafe.h>

#include "mousehook\mousehook.h"

HINSTANCE   g_hInst = NULL;
BOOL        g_bEnabled = TRUE;
INT         g_nMonitorButtons = 0;
INT         g_nTimeoutMilliseconds = 0;

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

wchar_t const szWindowClass[] = L"DoubleClickMonitor";

// Use a guid to uniquely identify our icon
class __declspec(uuid("2EC38AD7-3376-4C48-A8A9-31CC116B628C")) MouseIcon;

// Forward declarations of functions included in this code module:
void                RegisterWindowClass(PCWSTR pszClassName, PCWSTR pszMenuName, WNDPROC lpfnWndProc);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                ShowContextMenu(HWND hwnd, POINT pt);
BOOL                AddNotificationIcon(HWND hwnd);
BOOL                DeleteNotificationIcon();
BOOL                LoadSettings(HWND hwnd);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int nCmdShow) {
    g_hInst = hInstance;

    // Only allow one instance
    HWND hwExistingHandle = FindWindow(szWindowClass, NULL);

    if (hwExistingHandle) {
        MessageBox(hwExistingHandle, L"Double Click Monitor is already running.", NULL, MB_ICONEXCLAMATION);
        return -1;
    }        

    RegisterWindowClass(szWindowClass, NULL, WndProc);

    // Create the main window, but don't show it
    WCHAR szTitle[100];
    LoadString(hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));
    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 250, 200, NULL, NULL, g_hInst, NULL);
    if (hwnd) {
        if (!LoadSettings(hwnd))
            return -1;

        nCmdShow = 0;
        ShowWindow(hwnd, nCmdShow);
        SetMouseHook(g_nMonitorButtons, g_nTimeoutMilliseconds);
        
        // Main message loop:
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

void RegisterWindowClass(PCWSTR pszClassName, PCWSTR pszMenuName, WNDPROC lpfnWndProc) {
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = lpfnWndProc;
    wcex.hInstance = g_hInst;
    wcex.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTIFICATIONICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = pszMenuName;
    wcex.lpszClassName = pszClassName;
    RegisterClassEx(&wcex);
}

BOOL AddNotificationIcon(HWND hwnd) {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    // add the icon, setting the icon, tooltip, and callback message.
    // the icon will be identified with the GUID
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = __uuidof(MouseIcon);
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    HRESULT hr = LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);

    if (FAILED(hr)) {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();

        MessageBox(hwnd, errMsg, NULL, MB_ICONERROR);
    }

    LoadString(g_hInst, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon() {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(MouseIcon);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL LoadSettings(HWND hwnd) {
    UINT uLeftMouseButton = 1;
    UINT uRightMouseButton = 0;
    UINT uMiddleMouseButton = 0;
    UINT uTimeoutMilliseconds = 80;

    LPCTSTR lpButtonsSection = L"monitor_buttons";
    LPCTSTR lpTimeoutSection = L"timeout";

    WCHAR szSettingsFilePath[128];
    WCHAR szSettingsError[128];    

    LoadString(g_hInst, IDS_SETTINGS_FILE_PATH, szSettingsFilePath, 128);
    LoadString(g_hInst, IDS_SETTINGS_ERROR, szSettingsError, 128);

    GetFileAttributes(szSettingsFilePath);
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szSettingsFilePath) && GetLastError() == ERROR_FILE_NOT_FOUND) {
        if (!WritePrivateProfileSection(lpButtonsSection, L"left=1\0right=0\0middle=0\0", szSettingsFilePath) || !WritePrivateProfileSection(lpTimeoutSection, L"milliseconds=80\0", szSettingsFilePath)) {
            MessageBox(hwnd, szSettingsError, NULL,  MB_ICONERROR);
            return FALSE;
        }
    }

    uLeftMouseButton = GetPrivateProfileInt(lpButtonsSection, L"left", uLeftMouseButton, szSettingsFilePath);
    uRightMouseButton = GetPrivateProfileInt(lpButtonsSection, L"right", uRightMouseButton, szSettingsFilePath);
    uMiddleMouseButton = GetPrivateProfileInt(lpButtonsSection, L"middle", uMiddleMouseButton, szSettingsFilePath);
    g_nMonitorButtons = uLeftMouseButton * MHK_LEFT_MOUSE_BUTTON + uRightMouseButton * MHK_RIGHT_MOUSE_BUTTON + uMiddleMouseButton * MHK_MIDDLE_MOUSE_BUTTON;
    g_nTimeoutMilliseconds = GetPrivateProfileInt(lpTimeoutSection, L"milliseconds", uTimeoutMilliseconds, szSettingsFilePath);

    return TRUE;
}

void ShowContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenu) {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu) {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);

            if (g_bEnabled) {
                MENUITEMINFO mi = { 0 };
                mi.cbSize = sizeof(MENUITEMINFO);
                mi.fMask = MIIM_STATE;
                mi.fState = MF_CHECKED;
                SetMenuItemInfo(hSubMenu, IDM_ENABLED, FALSE, &mi);
            }

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            }
            else {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // add the notification icon
            if (!AddNotificationIcon(hwnd)) {
                WCHAR szError[128];

                LoadString(g_hInst, IDS_ERROR_LOADING_APPLICATION, szError, 128);                
                MessageBox(hwnd, szError, NULL, MB_ICONERROR);

                return -1;
            }
            break;
        case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                case IDM_ENABLED:
                    if (g_bEnabled) {
                        RemoveMouseHook();
                    }
                    else {
                        SetMouseHook(g_nMonitorButtons, g_nTimeoutMilliseconds);
                    }
                    g_bEnabled = !g_bEnabled;

                    break;
                case IDM_STATUS:
                {
                    WCHAR szTitle[128];
                    WCHAR szMessage[128];

                    LoadString(g_hInst, IDS_STATUS_BOX, szTitle, 128);

                    if (g_bEnabled) {                        
                        TCHAR pszDest[256];
                        size_t cchDest = 256;
                        
                        LoadString(g_hInst, IDS_STATUS, szMessage, 128);
                        StringCchPrintf(pszDest, cchDest, szMessage, GetMouseHookCaughtCount());

                        MessageBox(hwnd, pszDest, szTitle, MB_ICONINFORMATION);
                    }
                    else {                        
                        LoadString(g_hInst, IDS_NOT_ENABLED, szMessage, 128);
                        int nResult = MessageBox(hwnd, szMessage, szTitle, MB_ICONINFORMATION+MB_YESNO);

                        if (nResult == IDYES) {
                            g_bEnabled = TRUE;
                            SetMouseHook(g_nMonitorButtons, g_nTimeoutMilliseconds);
                        }
                    }                    

                    break;
                }
                case IDM_SETTINGS:
                    WCHAR szSettingsFilePath[128];

                    LoadString(g_hInst, IDS_SETTINGS_FILE_PATH, szSettingsFilePath, 128);
                    ShellExecute(0, 0, szSettingsFilePath, 0, 0, SW_SHOW);

                    break;
                case IDM_ABOUT:
                    WCHAR szWebsite[128];

                    LoadString(g_hInst, IDS_WEBSITE, szWebsite, 128);
                    ShellExecute(0, 0, szWebsite, 0, 0, SW_SHOW);

                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;

                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;

        case WMAPP_NOTIFYCALLBACK:
            switch (LOWORD(lParam)) {
                case WM_CONTEXTMENU:
                {
                    POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
                    ShowContextMenu(hwnd, pt);
                }
                break;
            }
            break;

        case WM_DESTROY:
            DeleteNotificationIcon();
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
