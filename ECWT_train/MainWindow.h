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

    static const COLORREF bgCol;
    static const HBRUSH hbrBg;
    static const COLORREF bgColD;
    static const HBRUSH hbrBgD;

    PWSTR pFilePath;
    int LSz;
    int deg;
    int cNo;
    int wCNo;
    int minL;
    int stepL;
    int MaxL;
    int wShift;
    double GoFThresh;

    Lib<ECWT<PWvlet>> lib;

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

    bool EnabCond(HWND contr)
    {
        if ((contr == demoButton) || (contr == BgButton))
            return (pFilePath != 0) && (LSz != 0) && (deg != 0) && (cNo != -1) && (minL > (deg + 1)) &&
            (MaxL >= minL) && (wShift > 0) && (GoFThresh>= 0) && (GoFThresh < 1);
        if ((contr == WSEdit) || (contr == WSEditLL) || (contr == WSEditLT))
            return (deg != 0);
        if ((contr == WSHEdit) || (contr == WSHEditL))
            return (deg != 0) && (minL > (deg + 1)) && (MaxL >= minL);
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
    MainWindow(): pFilePath(0), LSz(0), deg(0), cNo(-1), wCNo(-1), minL(0), stepL(1), MaxL(0), wShift(0),
        WSEditRed(true), WSHEditRed(true), GoFThresh(-1)
    {
    }
    static BOOL CALLBACK PlaceCntrl(HWND, LPARAM);
    static BOOL CALLBACK BToTop(HWND, LPARAM);
    PCWSTR  ClassName() const { return L"ECWT Unsupervised Training"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL BaseWindow::Create(PCWSTR, DWORD, DWORD, int, int, int, int, HWND, HMENU);
    friend COLORREF SetBkColor(HDC, COLORREF);
};


