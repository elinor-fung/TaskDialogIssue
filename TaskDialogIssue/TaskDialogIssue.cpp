// TaskDialogIssue.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>

#include <commctrl.h>
#include <iostream>

namespace
{
    // Create an activation context using a manifest that uses comctl32.dll version 6
    bool enable_visual_styles(HANDLE* handle, ULONG_PTR* cookie)
    {
        wchar_t buf[MAX_PATH];
        UINT len = ::GetWindowsDirectoryW(buf, MAX_PATH);
        if (len == 0 || len >= MAX_PATH)
            return false;

        std::wstring manifest(buf);
        manifest.append(L"\\WindowsShell.Manifest");

        ACTCTXW actctx = { sizeof(ACTCTXW), 0, manifest.c_str() };
        HANDLE context_handle = ::CreateActCtxW(&actctx);
        if (context_handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        ULONG_PTR cookie_local;
        if (::ActivateActCtx(context_handle, &cookie_local) == FALSE)
        {
            ::ReleaseActCtx(context_handle);
            return false;
        }

        *handle = context_handle;
        *cookie = cookie_local;
        return true;
    }

    void show_task_dialog()
    {
        HMODULE comctl32 = ::LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (comctl32 == nullptr)
            return;

        typedef HRESULT(WINAPI* task_dialog_indirect)(
            const TASKDIALOGCONFIG* pTaskConfig,
            int* pnButton,
            int* pnRadioButton,
            BOOL* pfVerificationFlagChecked);

        task_dialog_indirect task_dialog_indirect_func = (task_dialog_indirect)::GetProcAddress(comctl32, "TaskDialogIndirect");
        if (task_dialog_indirect_func == nullptr)
        {
            ::FreeLibrary(comctl32);
            return;
        }

        TASKDIALOGCONFIG config = { 0 };
        config.cbSize = sizeof(config);
        config.hInstance = nullptr;
        config.dwCommonButtons = TDCBF_CANCEL_BUTTON;
        config.pszMainIcon = TD_WARNING_ICON;
        config.pszMainInstruction = L"Main instruction";
        config.pszContent = L"Content";
        config.pszExpandedInformation = L"Details!";

        int nButtonPressed = 0;
        task_dialog_indirect_func(&config, &nButtonPressed, NULL, NULL);
    }
}

int main()
{
    auto prevCtx = ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HANDLE handle;
    ULONG_PTR cookie;
    if (enable_visual_styles(&handle, &cookie))
    {
        show_task_dialog();

        ::DeactivateActCtx(0, cookie);
        ::ReleaseActCtx(handle);
    }

    ::SetThreadDpiAwarenessContext(prevCtx);
}
