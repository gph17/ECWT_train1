#pragma once

#include <atlbase.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <windows.h>

#include "BaseWindow.h"
#include "dAWin.h"

class MainWindow : public BaseWindow<MainWindow>
{
    static  regStat registered;

    dAWin graphs[3];
    HWND demoButton;
    HWND BgButton;
    HWND BrButton;
    HWND AbButton;
    HWND degreeLB;
    HWND cWCondLB;
    HWND degreeLBL;
    HWND cWCondLBL;

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
    static BOOL CALLBACK PlaceCntrl(HWND, LPARAM);
    static BOOL CALLBACK BToTop(HWND, LPARAM);
    PCWSTR  ClassName() const { return L"ECWT Unsupervised Training"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL BaseWindow::Create(PCWSTR, DWORD, DWORD, int, int, int, int, HWND, HMENU);
};


