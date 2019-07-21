#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"
#include <stdio.h>
#include <stdarg.h>
#include <cassert>
#include "dwmapi.h"

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

const unsigned short WMA_DIALOGACTION		 = WM_APP+1;

HINSTANCE hInst;
HWND hMain, hLvl1, hLvl2;
bool lvl1_shownDlg;

void PrintDebug(LPCTSTR pszFormat, ...)
{
    TCHAR szMsg[512];
    va_list pArg;

    va_start(pArg, pszFormat);
    _vsntprintf_s(szMsg, ARRAYSIZE(szMsg), ARRAYSIZE(szMsg) - 1, pszFormat, pArg);
    va_end(pArg);

    OutputDebugString(szMsg);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            SendMessage(hDlg, WM_CLOSE, 0, 0);
            return TRUE;
        case IDOK:
            SendMessage(hDlg, WM_CLOSE, 0, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
void lvl2_OnWindowPosChanging(WINDOWPOS* lpwndpos);

INT_PTR CALLBACK lvl2_DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (DialogProc(hDlg, uMsg, wParam, lParam)) {
        return TRUE;
    }
    assert(!hLvl2 || hDlg == hLvl2);
    switch (uMsg)
    {
    case WM_DESTROY:
        hLvl2 = nullptr;
        return TRUE;

    case WM_WINDOWPOSCHANGING:
        lvl2_OnWindowPosChanging((WINDOWPOS*)lParam);
        return TRUE;
    }
    return FALSE;
}

void lvl1_OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    if ((lpwndpos->flags & SWP_NOZORDER) == 0)
        PrintDebug(
            _T(" NZO LVL1 phwind: 0x%p, hwnd : 0x%p, hwndInsertAfter : 0x%p, flags : 0x%X\n")
            , GetParent(lpwndpos->hwnd)
            , lpwndpos->hwnd
            , lpwndpos->hwndInsertAfter
            , lpwndpos->flags);
    if ((lpwndpos->flags & SWP_NOOWNERZORDER) == 0)
        PrintDebug(
            _T("NOZO LVL1 phwind: 0x%p, hwnd : 0x%p, hwndInsertAfter : 0x%p, flags : 0x%X\n")
            , GetParent(lpwndpos->hwnd)
            , lpwndpos->hwnd
            , lpwndpos->hwndInsertAfter
            , lpwndpos->flags);
}

void lvl2_OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    lvl1_OnWindowPosChanging(lpwndpos);
}

void lvl1_OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
    if (!lvl1_shownDlg && (lpwndpos->flags & SWP_SHOWWINDOW)) {
        lvl1_shownDlg = true;
        PostMessage(hLvl1, WMA_DIALOGACTION, 0, 0);
    }
}

INT_PTR CALLBACK lvl1_DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (DialogProc(hDlg, uMsg, wParam, lParam)) {
        return TRUE;
    }
    assert(!hLvl1 || hDlg == hLvl1);
    switch (uMsg)
    {
    case WM_DESTROY:
        hLvl1 = nullptr;
        lvl1_shownDlg = false;
        return TRUE;

    case WM_WINDOWPOSCHANGING:
        lvl1_OnWindowPosChanging((WINDOWPOS*)lParam);
        return TRUE;

    case WM_WINDOWPOSCHANGED:
        lvl1_OnWindowPosChanged((WINDOWPOS*)lParam);
        return TRUE;

    case WMA_DIALOGACTION:
        // Open 2nd level dlg
        hLvl2 = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_LVL_2), hLvl1, lvl2_DialogProc, 0);
        ShowWindow(hLvl2, SW_SHOW);
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK main_DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (DialogProc(hDlg, uMsg, wParam, lParam)) {
        return TRUE;
    }
    assert(!hMain || hDlg == hMain);
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DLG_LVL1:
            // Open 1st level dlg
            hLvl1 = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_LVL_1), hMain, lvl1_DialogProc, 0);
            //DWORD attrib = TRUE;
            //DwmSetWindowAttribute(hLvl1, DWMWA_TRANSITIONS_FORCEDISABLED, &attrib, sizeof(attrib));
            ShowWindow(hLvl1, SW_SHOW);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hDlg;
    MSG msg;
    BOOL ret;
    ::hInst = hInst;

    InitCommonControls();
    hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, main_DialogProc, 0);
    ShowWindow(hDlg, nCmdShow);

    while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
        if(ret == -1)
            return -1;

        if(!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}