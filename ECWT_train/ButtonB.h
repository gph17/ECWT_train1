#pragma once

#include <atlbase.h>
#include <windows.h>

#include "BaseWindow.h"

class ButtonB :
    public BaseWindow<ButtonB>
{
    static regStat registered;
public:
    void Resize();
    ButtonB()
    {

    }

    PCWSTR  ClassName() const { return L"BUTTON"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    friend class MainWindow;
    friend BOOL BaseWindow::Create(PCWSTR, DWORD, DWORD, int, int, int, int, HWND, HMENU);
};


