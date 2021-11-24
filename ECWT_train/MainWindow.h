#pragma once

#include <atlbase.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <windows.h>

#include "BaseWindow.h"
#include "dAWin.h"
#include "ECWT.h"
#include "Lib.h"
#include "PWvlet.h"

class MainWindow : public BaseWindow<MainWindow>
{
    static  regStat registered;

    float yScale;

    dAWin graphs[3];
    HWND demoButton;
    HWND BgButton;
    HWND BrButton;
    HWND AbButton;
    HWND degreeLB;
    HWND cWCondLB;
    HWND degreeLBL;
    HWND cWCondLBL;
    HWND LSzEB;
    HWND LSzEBL;
    HWND WSEdit;
    HWND WSEditLL;
    HWND WSEditLT;
    HWND WSHEdit;
    HWND WSHEditL;
    HWND GoFTh;
	HWND GoFThL;
    HWND WGoFL;
    HWND memL;

    static const COLORREF bgCol;
    static const HBRUSH hbrBg;
    static const COLORREF bgColD;
    static const HBRUSH hbrBgD;

    PWSTR pFilePath;
    int minL;
    int stepL;
    int MaxL;
    int wShift;

    Lib<PWvlet> lib;

    void    OnPaint();
    void    Resize();
    void    OnBrowse();
    void    OnProcess(int);
    LRESULT OnCmd(WPARAM, LPARAM);
	LRESULT OnCreate();
    LRESULT ChangeStatCol(WPARAM, LPARAM);
    LRESULT OnDegChange(WPARAM, LPARAM);
    LRESULT OnLBNSel(WPARAM, LPARAM);
    LRESULT OnWSChange(WPARAM, LPARAM);
    LRESULT OnGTChange(WPARAM, LPARAM);
    LRESULT OnWShChange(WPARAM, LPARAM);
	LRESULT ChangeEditTCol(WPARAM, LPARAM);
    LRESULT OnLGrow(WPARAM);
    LRESULT OnDGVal(WPARAM);
    LRESULT OnySc(LPARAM);
    LRESULT OnyRsc(LPARAM);
    LRESULT OnWvltDraw(LPARAM);
    LRESULT OnDWDraw(LPARAM);

    bool EnabCond(HWND contr)
    {
        if ((contr == demoButton) || (contr == BgButton))
            return (pFilePath != 0) && (lib.LSize != 0) && (lib.n != 0) && (lib.cNo != -1) && (minL > (lib.n + 1)) &&
            (MaxL >= minL) && (wShift > 0) && (lib.GoFThresh>= 0) && (lib.GoFThresh < 1);
        if ((contr == WSEdit) || (contr == WSEditLL) || (contr == WSEditLT))
            return (lib.n != 0);
        if ((contr == WSHEdit) || (contr == WSHEditL))
            return (lib.n != 0) && (minL > (lib.n + 1)) && (MaxL >= minL);
        return true;
    }

    bool GFEditRed, WSEditRed, WSHEditRed;
    static HWND CreateCntrl(LPCWSTR wCName,
        LPCWSTR wTitle,
        DWORD mask,
        HWND par_hwnd,
        int CntrlNum);
    static void WSEValid(wchar_t* inp, int& m, int& s, int& M);

public:
    MainWindow(): pFilePath(0), minL(0), stepL(1), MaxL(0), wShift(0),
        WSEditRed(true), WSHEditRed(true)
    {
    }
    static BOOL CALLBACK PlaceCntrl(HWND, LPARAM);
    static BOOL CALLBACK BToTop(HWND, LPARAM);
    PCWSTR  ClassName() const { return L"ECWT Unsupervised Training"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL BaseWindow::Create(PCWSTR, DWORD, DWORD, int, int, int, int, HWND, HMENU);
    friend COLORREF SetBkColor(HDC, COLORREF);
};


