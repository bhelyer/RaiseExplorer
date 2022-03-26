/* A program that brings all explorer windows to the top at once
 * whenever CTRL+SHIFT+E is hit.
 *
 * Compile with: cl /O2 /W4 /WX /EHsc /MP /DUNICODE /Fe:RaiseExplorer.exe RaiseExplorer.cpp /link /subsystem:windows user32.lib ole32.lib
 */
#include <vector>
#include <sstream>
#include <algorithm>
#include <ExDisp.h>
#include <Windows.h>
#include <ShlObj_core.h>

 /* This isn't included in the same file as the definition of IVirtualDesktopManager.
  * I don't know enough about COM to say where it should be, but declare it here.
  */
const CLSID CLSID_VirtualDesktopManager = {
    0xAA509086, 0x5CA9, 0x4C25, { 0x8f, 0x95, 0x58, 0x9d, 0x3c, 0x07, 0xb4, 0x8a }
};

// Block until the hotkey registered at the top of wWinMain has been hit.
void waitForHotkey() {
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            return;
        }
    }
}

// Construct a vector of all explorer windows that are on the current virtual desktop.
std::vector<HWND> getExplorerWindows() {
    std::vector<HWND> explorerWindows;

    IVirtualDesktopManager* desktopManager = nullptr;
    (void)CoCreateInstance(CLSID_VirtualDesktopManager, NULL, CLSCTX_ALL, IID_IVirtualDesktopManager, (void**)&desktopManager);

    IShellWindows* shellWindows;
    const HRESULT creationResult = CoCreateInstance(
        CLSID_ShellWindows, NULL, CLSCTX_ALL,
        IID_IShellWindows, (void**)&shellWindows
    );
    if (SUCCEEDED(creationResult)) {
        VARIANT v;
        V_VT(&v) = VT_I4;
        IDispatch* dispatch;
        for (V_I4(&v) = 0; shellWindows->Item(v, &dispatch) == S_OK; V_I4(&v)++) {
            IWebBrowserApp* wbapp;
            if (SUCCEEDED(dispatch->QueryInterface(IID_IWebBrowserApp, (void**)&wbapp))) {
                HWND hwnd;
                if (SUCCEEDED(wbapp->get_HWND((LONG_PTR*)&hwnd))) {
                    BOOL onDesktop = TRUE;
                    if (desktopManager != nullptr) {
                        if (!SUCCEEDED(desktopManager->IsWindowOnCurrentVirtualDesktop(hwnd, &onDesktop))) {
                            onDesktop = FALSE;
                        }
                    }
                    if (onDesktop == TRUE) {
                        explorerWindows.emplace_back(hwnd);
                    }
                }
                wbapp->Release();
            }
            dispatch->Release();
        }
        shellWindows->Release();
    }

    if (desktopManager != nullptr) {
        desktopManager->Release();
    }

    return explorerWindows;
}

// Display an error in a message box and die.
void error(const wchar_t* msg) {
    MessageBox(nullptr, msg, L"RaiseExplorer", MB_ICONERROR | MB_OK);
    ExitProcess(1);
}

// Bring the given window to the front of the z order.
void bringToFront(HWND hwnd) {
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }
    if (!SetForegroundWindow(hwnd)) {
        error(L"Failed to bring window to front.");
    }
    Sleep(10);  // If we don't do this, only the last call will do anything.
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    // Initialise COM, so we can use IShellWindows.
    if (CoInitializeEx(nullptr, COINIT_MULTITHREADED) != S_OK) {
        error(L"Couldn't initialise COM.");
    }

    // Set up WM_HOTKEY to be sent on CTRL+SHIFT+E.
    if (!RegisterHotKey(nullptr, 1, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'E')) {
        error(L"Couldn't register hotkey.");
    }

    while (true) {
        waitForHotkey();
        const auto windows = getExplorerWindows();
        std::for_each(std::begin(windows), std::end(windows), bringToFront);
    }

    // Never reached! Kill the process to exit.

    return 0;
}