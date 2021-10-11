#pragma once

#include <atlbase.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <windows.h>

#include "BaseWindow.h"
#include "ButtonB.h"
#include "dAWin.h"

class MainWindow : public BaseWindow<MainWindow>
{
    static  regStat registered;

    dAWin graphs[3];
    ButtonB demoButton;
    ButtonB BgButton;
    ButtonB BrButton;
    ButtonB AbButton;

    PWSTR pszFilePath;

    void    OnPaint();
    void    Resize();
    void    OnBrowse();
    void    OnProcess(int);
    LRESULT OnCreate();

public:

    MainWindow()
    {
    }

    PCWSTR  ClassName() const { return L"ECWT Unsupervised Training"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL BaseWindow::Create(PCWSTR, DWORD, DWORD, int, int, int, int, HWND, HMENU);
};


