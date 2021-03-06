#include <Eigen/Dense>
#include <shobjidl.h>
#include <sstream>
#include <string>
#include <thread>
#include <windowsx.h>

#include "MainWindow.h"
#include "PWvlet.h"
#include "util.h"

using namespace std;
using namespace Eigen;

#define BRBUTTON 1
#define BGBUTTON 2
#define DEBUTTON 3
#define ABBUTTON 4
#define DELB 5
#define CWLB 6
#define DELBL 7
#define CWLBL 8
#define NEDIT 9
#define NEDITL 10
#define WSEDIT 11

#define WSEDITLL 12
#define WSEDITLT 13
#define WSHEDIT 14
#define WSHEDITL 15
#define GOFTH 16
#define GOFTHL 17
#define WGOFL 18
#define MEML 19

#define  MATHDONE (WM_USER + 2)


regStat dAWin::registered = regStat::NEEDSPECIAL;
regStat MainWindow::registered = regStat::NEEDPLAIN;

const COLORREF MainWindow::bgCol = RGB(160, 150, 150);
const HBRUSH MainWindow::hbrBg = CreateSolidBrush(MainWindow::bgCol);
const COLORREF MainWindow::bgColD = RGB(200, 200, 200);
const HBRUSH MainWindow::hbrBgD = CreateSolidBrush(MainWindow::bgColD);

void MainWindow::OnBrowse()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        if (SUCCEEDED(hr))
        {
            const COMDLG_FILTERSPEC rgFilterSpec[] = { {L"Comma Separated Variable files (*.csv)", L"*.csv"}};
            hr = pFileOpen->SetFileTypes(1, rgFilterSpec);
            if (SUCCEEDED(hr))
            {
                pFileOpen->SetOkButtonLabel(L"Select");
                hr = pFileOpen->Show(NULL);
                if (SUCCEEDED(hr))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath);
                        pItem->Release();
                    }
                }
            }
        }
    }
    CoUninitialize();
    if (SUCCEEDED(hr))
    {
        if (csvCheck(pFilePath))
        {
            //display file path, inactivate Browse Button, activate other buttons
            wstring str1 = pFilePath;
            wchar_t pStr2[128];
            GetWindowText(m_hwnd, pStr2, 128);
            wstring str2 = pStr2;
            wstring str3 = str2 + L": " + str1;
            SetWindowText(m_hwnd, str3.c_str());
            if (EnabCond(BgButton))
            {
                EnableWindow(BgButton, TRUE);
                EnableWindow(demoButton, TRUE);
            }
            else
            {
                EnableWindow(BgButton, FALSE);
                EnableWindow(demoButton, FALSE);
            }
        }
        else
        {
            if (pFilePath != 0)
            {
                delete[] pFilePath;
                pFilePath = 0;
            }
            SetWindowText(m_hwnd, L"ECWT - Unsupervised training");
            EnableWindow(BgButton, FALSE);
            EnableWindow(demoButton, FALSE);
        }
    }
}

void MainWindow::OnProcess(int butType)
{
    //put common stuff before or after separate code for the two process buttons - setting up processing, setting 
    //  it off, setting up progress bar, enabling process interruption
    ShowWindow(BgButton, SW_HIDE);
    ShowWindow(demoButton, SW_HIDE);
    ShowWindow(BrButton, SW_HIDE);
    ShowWindow(AbButton, SW_SHOW);
    SendMessage(WSEdit, EM_SETREADONLY, (WPARAM)TRUE, NULL);
    SendMessage(WShEdit, EM_SETREADONLY, (WPARAM)TRUE, NULL);
    SendMessage(LSzEB, EM_SETREADONLY, (WPARAM)TRUE, NULL);
    SendMessage(GoFTh, EM_SETREADONLY, (WPARAM)TRUE, NULL);
    ListBox_Enable(degreeLB, FALSE);
    ListBox_Enable(cWCondLB, FALSE);
    RECT rec;
    GetClientRect(GetAncestor(AbButton, GA_PARENT), &rec);
    int y = (9 * (rec.bottom - rec.top)) / 10;
    SetWindowPos(AbButton, HWND_TOP, 25, y, 100, 50, NULL);
    int i;
    for (i = minL; i < MaxL; i += stepL)
    {
        if (lib.WLs.size() == 0)
            lib.WLs.clear();
		lib.WLs.push_back(i);
    }

    if (butType == BGBUTTON)
    {
        //hide drawing areas
        for (i = 0; i < 3; i++)
            ShowWindow(graphs[i].m_hwnd, SW_HIDE);
        thread t([this]()
            {
                lib.build(pFilePath, wShift, Window());
                SendMessage(m_hwnd, MATHDONE, NULL, NULL);
            });
        t.detach();
    }
    else
        if (butType == DEBUTTON)
        {
			thread t([this]()
				{
					lib.build(pFilePath, wShift, Window(), 500ms);
                    SendMessage(m_hwnd, MATHDONE, NULL, NULL);
				});
			t.detach();
        }
}

void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	FillRect(hdc, &ps.rcPaint, hbrBg);
    EnumChildWindows(m_hwnd, BToTop, NULL);
	EndPaint(m_hwnd, &ps);
}

void MainWindow::Resize()
{
    if (dAWin::registered == regStat::DONE)
    {
        int i;
        for (i = 0; i < 3; i++)
            graphs[i].Resize();

        RECT rec;
        GetClientRect(m_hwnd, &rec);
        LPARAM lParam = (LPARAM)(&rec);
        EnumChildWindows(m_hwnd, PlaceCntrl, lParam);
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
    case WVLTDRAW:
        return OnWvltDraw(lParam);
    case DATAWDRAW:
        return OnDWDraw(lParam);
    case ADJ_YSCL:
        return OnySc(lParam);
    case ADJ_YRSCL:
        return OnyRsc(lParam);//put this inside drawing function for wvlets
    case MATHDONE:
    {
        wchar_t pStr1[128];
        GetWindowText(m_hwnd, pStr1, 128);
        wstring str1 = pStr1;
        wstring str2 = str1 + L" - DONE";
        return SetWindowText(m_hwnd, str2.c_str());
    }
    case DISPLAYGVAL:
        return OnDGVal(wParam, lParam);
    case DISPLAYLSIZE:
        return OnLGrow(wParam);
    case WM_CTLCOLOREDIT:
        return ChangeEditTCol(wParam, lParam);
    case WM_CTLCOLORSTATIC:
        return ChangeStatCol(wParam, lParam);
    case WM_CREATE:
		return OnCreate();

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT:
	{
		OnPaint();
		ValidateRect(m_hwnd, NULL);
		return 0;
	}
	case WM_SIZE:
	{
		Resize();
		return 0;
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 900;
		lpMMI->ptMinTrackSize.y = 600;
		return 0;
	}
	case WM_COMMAND:
	{
        OnCmd(wParam, lParam);
		return 0;
	}
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::OnDGVal(WPARAM wparam, LPARAM lparam)
{
    wchar_t buf[120];
    double* tmp = reinterpret_cast<double*>(wparam);
    if (*(reinterpret_cast<int*>(lparam))== 1)
        std::swprintf(buf, 120, L"Best GoF = %f", *tmp);
    else
        std::swprintf(buf, 120, L"Worst GoF = %f", *tmp);
    return SetWindowText(WGoFL, buf);
}

LRESULT MainWindow::OnLGrow(WPARAM wparam)
{
    wchar_t buf[120], buf0[120];
    buf[119] = 0;
    buf0[119] = 0;
    _itow_s((int)wparam, buf0, 10);
    wcscpy_s(buf, L"Members: ");
    wcscat_s(buf, buf0);
    return SetWindowText(memL, buf);
}

LRESULT MainWindow::OnCreate()
{
    ShowWindow(m_hwnd, SW_SHOW);
    RECT rec;
    int i;
    if (GetClientRect(m_hwnd, &rec))
    {
        int cheight = (3 * (rec.bottom - rec.top)) / 5;
        int chwidth = (rec.right - rec.left - 40) / 3;
        for (i = 0; i < 3; i++)
        {
            int i1 = i + 100;
            (&graphs[i])->Create(L"dAWin",
                WS_CHILD | WS_BORDER | WS_VISIBLE, NULL,
				i * (chwidth + 10) + 10, (rec.bottom - rec.top) / 6, chwidth, cheight,
				m_hwnd,
				(HMENU)(UINT64)i1);
			if (FAILED(graphs[i].GraphSetUp()))
				exit(-1);
		}
        
        //top row controls
        if (!(BrButton = CreateCntrl(L"BUTTON",
            L"Browse for Source",
            WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_MULTILINE,
            m_hwnd,
            BRBUTTON)))
            return(-1);

        if (!(LSzEBL = CreateCntrl(L"STATIC",
            L"Base Library Size",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            NEDITL)))
            return(-1);

        if (!(LSzEB = CreateCntrl(L"EDIT",
            NULL, 
            WS_VISIBLE |ES_LEFT,
            m_hwnd,
            NEDIT)))
            return(-1);
        SendMessage(LSzEB, EM_SETLIMITTEXT, (WPARAM)7, NULL);

        if (!(degreeLBL = CreateCntrl(L"STATIC",
            L"Degree",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            DELBL)))
            return(-1);

        if (!(degreeLB = CreateCntrl(L"LISTBOX",
            NULL,
            WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOTIFY,
            m_hwnd,
            DELB)))
            return(-1);
        set<int> ns = PWvlet::getValid();
        wchar_t num[4];
        for (int item : ns)
        {
            _itow_s(item, num, 10);
            SendMessage(degreeLB, LB_ADDSTRING, NULL, (LPARAM)num);
        }

        if (!(cWCondLBL = CreateCntrl(L"STATIC",
            L"Conditions",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            CWLBL)))
            return(-1);

        if (!(cWCondLB = CreateCntrl(L"LISTBOX",
            NULL,
            WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOTIFY,
            m_hwnd,
            CWLB)))
            return(-1);

        if (!(GoFThL = CreateCntrl(L"STATIC",
            L"GoF \u0398",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            GOFTHL)))
            return(-1);

        if (!(WGoFL = CreateCntrl(L"STATIC",
            L"Worst GoF: 0",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WGOFL)))
            return(-1);

        if (!(GoFTh = CreateCntrl(L"EDIT",
            NULL,
            WS_VISIBLE | ES_LEFT,
            m_hwnd,
            GOFTH)))
            return(-1);

        //bottom row controls
		if (!(demoButton = CreateCntrl(L"BUTTON",
			L"Build Library (demo)",
			WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_MULTILINE,
			m_hwnd,
			DEBUTTON)))
			return(-1);
		EnableWindow(demoButton, FALSE);

        if (!(BgButton = CreateCntrl(L"BUTTON",
			L"Build Library (background)",
			WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_MULTILINE,
			m_hwnd,
			BGBUTTON)))
			return(-1);
		EnableWindow(BgButton, FALSE);
        
        if (!(AbButton = CreateCntrl(L"BUTTON",
            L"Abort",
            WS_TABSTOP | BS_DEFPUSHBUTTON | BS_MULTILINE,
            m_hwnd,
            ABBUTTON)))
            return(-1);

        if (!(WSEditLL = CreateCntrl(L"STATIC",
            L"Window Lengths",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WSEDITLL)))
            return(-1);
        EnableWindow(WSEditLL, FALSE);

        if (!(WSEdit = CreateCntrl(L"EDIT",
            NULL,
            WS_VISIBLE | ES_LEFT | ES_READONLY,
            m_hwnd,
            WSEDIT)))
            return(-1);

        if (!(WSEditLT = CreateCntrl(L"STATIC",
            L"m:M or m:s:M",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WSEDITLT)))
            return(-1);
        EnableWindow(WSEditLT, FALSE);

        if (!(WShEditL = CreateCntrl(L"STATIC",
            L"Window Shift",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WSHEDITL)))
            return(-1);
        EnableWindow(WShEditL, FALSE);

        if (!(WShEdit = CreateCntrl(L"EDIT",
            NULL,
            WS_VISIBLE | ES_LEFT | ES_READONLY,
            m_hwnd,
            WSHEDIT)))
            return(-1);

        if (!(memL = CreateCntrl(L"STATIC",
            L"Members: 0",
            WS_VISIBLE | ES_LEFT,
            m_hwnd,
            MEML)))
            return(-1);
        return 1;
    }
    else
        return -1;
}

BOOL CALLBACK MainWindow::BToTop(HWND hwnd, LPARAM lParam)
{
    int item = GetDlgCtrlID(hwnd);
    if (item < 100)
        if (IsWindowVisible(hwnd))
        {
            SendMessage(hwnd,
                WM_PAINT,
                NULL,
                NULL);
        }
    return TRUE;
}

BOOL CALLBACK MainWindow::PlaceCntrl(HWND hwnd, LPARAM lParam)
{
	if (IsWindowVisible(hwnd))
	{
		int bNo = GetDlgCtrlID(hwnd);
		RECT* rec = (RECT*)lParam;
		int cRH = rec->bottom - rec->top;
		int x = 25, y = (9 * cRH) / 10, yTop = cRH/ 25;
        int width = 100;
        int height = 50;
        switch (bNo)
		{
            //top row controls
        case BRBUTTON:
        {
            y = yTop;
            break;
        }
        case NEDITL:
        {
            x = 150;
            y = yTop;
            width = 90;
            break;
        }
        case NEDIT:
        {
            x = 245;
            y = yTop + 10;
            width = 80;
            height = 20;
            break;
        }
        case DELBL:
        {
            x = 350;
            y = yTop;
            width = 70;
            break;
        }
        case DELB:
        {
            x = 410;
            y = yTop;
            width = 50;
            break;
        }
        case CWLBL:
        {
            x = 490;
            y = yTop;
            width = 90;
            break;
        }
        case CWLB:
        {
            x = 570;
            y = yTop;
            width = 65;
            break;
        }
        case GOFTHL:
        {
            x = 665;
            y = yTop;
            width = 65;
            break;
        }
        case WGOFL:
        {
            x = 665;
            y = yTop + 25;
            width = 200;
            height = 40;
            break;
        }
        case GOFTH:
        {
            x = 745;
            y = yTop;
            width = 80;
            height = 20;
            break;
        }

        //bottom row controls
        case BGBUTTON:
        {
            x = 135;
            break;
        }
        case WSEDITLL:
        {
            x = 260;
            y += 10;
            width = 60;
            height = 80;
            break;
        }
        case WSEDITLT:
        {
            x = 330;
            y -= 10;
            height = 20;
            break;
        }
        case WSEDIT:
        {
            x = 330;
            y += 10;
            height = 20;
            break;
        }
        case WSHEDITL:
        {
            x = 465;
            break;
        }
        case WSHEDIT:
        {
            x = 570;
            height = 20;
            break;
        }
        case MEML:
        {
            x = 720;
            width = 200;
            height = 40;
            break;
        }
        }
		SetWindowPos(hwnd, HWND_TOP, x, y, width, height, NULL);
	}
	return TRUE;
}

LRESULT MainWindow::ChangeStatCol(WPARAM wParam, LPARAM lParam)
{
    HDC hdcStatic = (HDC)wParam;
    if (((HWND)lParam == WSEdit) || ((HWND)lParam == WShEdit) || ((HWND)lParam == LSzEB) || ((HWND)lParam == GoFTh))
    {
        SetBkColor(hdcStatic, MainWindow::bgColD);
        return (LRESULT)MainWindow::hbrBgD;
    }
    SetBkColor(hdcStatic, MainWindow::bgCol);
    return (LRESULT)MainWindow::hbrBg;
}

LRESULT MainWindow::ChangeEditTCol(WPARAM wParam, LPARAM lParam)
{
    if ((HWND)lParam == WSEdit)
    {
        if (WSEditRed)
            SetTextColor((HDC)wParam, RGB(255, 0, 0));
        else
            SetTextColor((HDC)wParam, RGB(0, 0, 0));

        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    if ((HWND)lParam == WShEdit)
    {
        if (WShEditRed)
            SetTextColor((HDC)wParam, RGB(255, 0, 0));
        else
            SetTextColor((HDC)wParam, RGB(0, 0, 0));

        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    if ((HWND)lParam == GoFTh)
    {
        if (GFEditRed)
            SetTextColor((HDC)wParam, RGB(255, 0, 0));
        else
            SetTextColor((HDC)wParam, RGB(0, 0, 0));

        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    return 0;
}

HWND MainWindow::CreateCntrl(LPCWSTR wCName,
    LPCWSTR wTitle,
    DWORD mask,
    HWND par_hwnd,
    int CntrlNum)
{
    return CreateWindow(wCName,
        wTitle,
        mask | WS_CHILD,
        0, 0, 0, 0,
        par_hwnd,
        (HMENU)(UINT64)CntrlNum,
        (HINSTANCE)GetWindowLongPtr(par_hwnd, GWLP_HINSTANCE),
        NULL);
}

LRESULT MainWindow::OnLBNSel(WPARAM wParam, LPARAM lParam)
{
    int LBId = LOWORD(wParam);
    UINT LB_MESS = HIWORD(wParam);
    wchar_t val[9];
    if (LB_MESS == LBN_SELCHANGE)
    {
        int ind = LOWORD(SendMessage((HWND)lParam, LB_GETCURSEL, NULL, NULL));
        LRESULT Len = SendMessage((HWND)lParam, LB_GETTEXT, (WPARAM)ind, (LPARAM)val);
        val[Len] = 0;
        if ((LBId == DELB) && (LB_MESS == LBN_SELCHANGE))
        {
            lib.n = _wtoi(val);
            set<pair<int, int>> vals = PWvlet::getValid(lib.n);
            wchar_t cWStr[9];
            wchar_t num[4];
            cWStr[8] = 0;
            num[3] = 0;
            ListBox_ResetContent(cWCondLB);
            for (auto item : vals)
            {
                wcscpy_s(cWStr, L"(");
                _itow_s(item.first, num, 10);
                wcscat_s(cWStr, num);
                wcscat_s(cWStr, L", ");
                _itow_s(item.second, num, 10);
                wcscat_s(cWStr, num);
                wcscat_s(cWStr, L")");
                SendMessage(cWCondLB, LB_ADDSTRING, NULL, (LPARAM)cWStr);
            }
            EnableWindow(WSEditLL, TRUE);
            EnableWindow(WSEditLT, TRUE);
            SendMessage(WSEdit, EM_SETREADONLY, (WPARAM)FALSE, NULL);
        }
        if (LBId == CWLB)
        {
            wstringstream vS(val);
            vS.ignore(1);
            vS >> lib.cNo;
            vS.ignore(2);
            vS >> lib.wCNo;
            if (EnabCond(demoButton))
            {
                EnableWindow(demoButton, TRUE);
                EnableWindow(BgButton, TRUE);
            }
            else
            {
                EnableWindow(demoButton, FALSE);
                EnableWindow(BgButton, FALSE);
            }
        }
    }
    return 0;
}

LRESULT MainWindow::OnDegChange(WPARAM wParam, LPARAM lParam)
{
    int LBId = LOWORD(wParam);
    UINT LB_MESS = HIWORD(wParam);
    wchar_t val[9];
    if (LB_MESS == EN_UPDATE)
    {
        Edit_GetText((HWND)lParam, val, 9);
        size_t len = wcslen(val);
        wchar_t* ptre = wcsstr(val, L"e");
        wchar_t* ptrE = wcsstr(val, L"E");
        if (len == 0)
            return 0;
        if (len == 1)
            if (!iswdigit(val[0]) && (val[0] != L'.'))
            {
                val[0] = '\0';
                SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
                return 0;
            }
        if (ptre && ptrE)
        {
			val[len - 1] = '\0';
			SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
            return 0;
        }
        if (!ptre && !ptrE)
            if (!iswdigit(val[len - 1]) && (val[len - 1] != L'.'))
            {
                val[len - 1] = '\0';
                SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
                return 0;
            }
        if (ptre && (ptre != &(val[len - 1])))
			if (!iswdigit(val[len - 1]))
            {
                val[len - 1] = '\0';
                SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
                return 0;
            }
        if (ptrE && (ptrE != &(val[len - 1])))
            if (!iswdigit(val[len - 1]))
            {
                val[len - 1] = '\0';
                SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
                return 0;
            }

            wstringstream wS(val);
            double dLSz;
            wS >> dLSz;
            lib.LSize = (int)round(dLSz);
        if (lib.LSize > 5e4)
        {
            lib.LSize = 0;
            val[0] = '\0';
            SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)val);
            return 0;
        }

        if (EnabCond(BgButton))
        {
            EnableWindow(BgButton, TRUE);
            EnableWindow(demoButton, TRUE);
        }
        else
        {
            EnableWindow(BgButton, FALSE);
            EnableWindow(demoButton, FALSE);
        }
    }
    return 0;
}

LRESULT MainWindow::OnCmd(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
    switch (id)
    {
    case ABBUTTON:
    {
        return SendMessage(m_hwnd, WM_DESTROY, NULL, NULL);
    }
    case BRBUTTON:
    {
        OnBrowse();
        break;
    }
    case BGBUTTON:
    {
        OnProcess(BGBUTTON);    //do less
        break;
    }
    case DEBUTTON:
    {
        OnProcess(DEBUTTON);    //do more
        break;
    }
    case DELB:
    case CWLB:
    {
        OnLBNSel(wParam, lParam);
        break;
    }
    case NEDIT:
    {
        OnDegChange(wParam, lParam);
        break;
    }
    case WSEDIT:
    {
        OnWSChange(wParam, lParam);
        break;
    }
    case WSHEDIT:
    {
        OnWShChange(wParam, lParam);
        break;
    }
    case GOFTH:
    {
        OnGTChange(wParam, lParam);
        break;
    }
    }
	return 0;
}

LRESULT MainWindow::OnWSChange(WPARAM wParam, LPARAM lParam)
{
    int code = HIWORD(wParam);
    if (code == EN_UPDATE)
    {
        HWND hwnd = (HWND)lParam;
        int len = Edit_GetTextLength(hwnd);
        if (len == 0)
            return 0;
        wchar_t* str = new wchar_t[len + 1ULL];
        Edit_GetText(hwnd, str, len + 1);
        //ensure last char is acceptable
        if ((!isdigit(str[len - 1])) && (str[len - 1] != L':'))
        {
            str[len - 1] = '\0';
            Edit_SetText(hwnd, str);
            SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            return 0;
        }
        //last input digit or  colon
        if (len == 1)
        {
            if ((str[0] == L':') || (str[0] == L'0'))
            {
                str[0] = '\0';
                Edit_SetText(hwnd, str);
                SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                return 0;
            }
        }
        //first input not 0 or colon
        else
        {
            if (((str[len - 1] == L':') || (str[len - 1] == L'0')) && (str[len - 2] == L':'))
            {
                str[len - 1] = '\0';
                Edit_SetText(hwnd, str);
                SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                return 0;
            }
        }
        //last input not a colon or 0 preceded by a colon
        if (str[len - 1] == ':')
        {
            int cCount = 0;
            for (int i = 0; (i < (len - 1)) && (cCount < 2); i++)
            {
                if (str[i] == L':')
                    cCount++;
            }
            if (cCount == 2)
            {
                str[len - 1] = '\0';
                Edit_SetText(hwnd, str);
                SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                return 0;
            }

        }
        //check completeness/compatibility with degree value
        MainWindow::WSEValid(str, minL, stepL, MaxL);
        if (minL && (MaxL >= minL))
        {
            //Enable WShEdit label, make WSHedit writable, change WSEdit text colour to black
            EnableWindow(WShEditL, TRUE);
            SendMessage(WShEdit, EM_SETREADONLY, (WPARAM)FALSE, NULL);
            WSEditRed = false;
            ChangeEditTCol(wParam, lParam);
        }
        else
        {
            //Disable WShEdit label, make WSHedit unwritable, change WSEdit text colour to red
            EnableWindow(WShEditL, FALSE);
            SendMessage(WShEdit, EM_SETREADONLY, (WPARAM)TRUE, NULL);
            WSEditRed = true;
            ChangeEditTCol(wParam, lParam);
        }
        if (str != 0)
            delete[] str;
    }
    return 0;
}

LRESULT MainWindow::OnWShChange(WPARAM wParam, LPARAM lParam)
{
    int code = HIWORD(wParam);
    if (code == EN_UPDATE)
    {
        HWND hwnd = (HWND)lParam;
        int len = Edit_GetTextLength(hwnd);
        if (len == 0)
            return 0;
        wchar_t* str = new wchar_t[len + 1ULL];
        Edit_GetText(hwnd, str, len + 1);
        //ensure last char is acceptable
        if ((len == 1) && (!isdigit(str[0]) || (str[0] == L'0')))
        {
            str[len - 1] = '\0';
            Edit_SetText(hwnd, str);
            SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            return 0;
        }
        if (!isdigit(str[0]))
        {
            str[len - 1] = '\0';
            Edit_SetText(hwnd, str);
            SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            return 0;
        }
        wstringstream wst(str);
        wst >> wShift;
        //check compatibility with minL value
        if (wShift <= minL)
        {
            //Change WShEdit text colour to black, possibly enable demoButton and BgButton
            WShEditRed = false;
            ChangeEditTCol(wParam, lParam);
            if (EnabCond(demoButton))
            {
                EnableWindow(demoButton, TRUE);
                EnableWindow(BgButton, TRUE);
            }
        }
        else
        {
            //Change WShEdit text colour to red, disable demoButton and BgButton
            WShEditRed = true;
            ChangeEditTCol(wParam, lParam);
            EnableWindow(demoButton, FALSE);
            EnableWindow(BgButton, FALSE);
        }
        if (str != 0)
            delete[] str;
    }
    return 0;
}

LRESULT MainWindow::OnGTChange(WPARAM wParam, LPARAM lParam)
{
    int code = HIWORD(wParam);
    if (code == EN_UPDATE)
    {
        HWND hwnd = (HWND)lParam;
        int len = Edit_GetTextLength(hwnd);
        if (len == 0)
        {
            lib.GoFThresh = -1;
            EnableWindow(demoButton, FALSE);
            EnableWindow(BgButton, FALSE);
            return 0;
        }
        wchar_t* str = new wchar_t[(UINT64)len + 1ULL];
        Edit_GetText(hwnd, str, len + 1);
        //ensure last char is acceptable
        if ((str[len - 1] != L'.') && !isdigit(str[len - 1]))
        {
            str[len - 1] = '\0';
            Edit_SetText(hwnd, str);
            SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            return 0;
        }
        if ((len == 1) && (str[0] != L'.') && (str[0] != L'0'))
        {
            str[len - 1] = '\0';
            Edit_SetText(hwnd, str);
            SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            return 0;
        }
        if (len== 2)
            if ((str[0]== L'0') && (str[1]!= L'.'))
			{
				str[len - 1] = '\0';
				Edit_SetText(hwnd, str);
				SendMessage(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
				return 0;
			}
        if (str[len - 1] == L'.')
        {
            GFEditRed = true;
            lib.GoFThresh = -1;
        }
        else
        {
            GFEditRed = false;
            wstringstream wst(str);
            wst >> lib.GoFThresh;
        }
        ChangeEditTCol(wParam, lParam);
        //check compatibility with minL value
		if (EnabCond(demoButton))
		{
			EnableWindow(demoButton, TRUE);
			EnableWindow(BgButton, TRUE);
		}
        else
        {
            EnableWindow(demoButton, FALSE);
            EnableWindow(BgButton, FALSE);
        }
        if (str != 0)
            delete[] str;
    }
    return 0;
}

void MainWindow::WSEValid(wchar_t* inp, int& m, int& s, int& M)
{
    wstringstream sInp(inp);
    sInp >> m;
    bool ebit = sInp.eof(), gbit = sInp.good();
    if (!(ebit || gbit) || m <= 0)
    {
        m = 0;
        s = 1;
        M = 0;
        return;
    }
    wchar_t ch;
    sInp >> ch;
    gbit = sInp.good();
    if (!gbit || ch != L':')
    {
        m = 0;
        s = 1;
        M = 0;
        return;
    }
    sInp >> s;
    gbit = sInp.good(), ebit = sInp.eof();
    if (!(gbit || ebit) || s < 1)
    {
        m = 0;
        s = 1;
        M = 0;
        return;
    }
    sInp >> ch;
    ebit = sInp.eof();
    if (ebit)
    {
        M = s;
        s = 1;
        return;
    }
    gbit = sInp.good();
    if (!gbit || ch != L':')
    {
        m = 0;
        s = 1;
        M = 0;
        return;
    }
    sInp >> M;
    gbit = sInp.good(), ebit = sInp.eof();
    if (!(gbit || ebit) || M < (m + s) || M > 5e4)
    {
        m = 0;
        s = 1;
        M = 0;
        return;
    }
    sInp >> ch;
    ebit = sInp.eof();
    if (ebit)
        return;
    m = 0;
    s = 1;
    M = 0;
}

LRESULT MainWindow::OnySc(LPARAM lparam)
{
    yScale = *(reinterpret_cast<float*>(lparam));
    return 0;
}

LRESULT MainWindow::OnyRsc(LPARAM lparam)
{
    yScale = max(*(reinterpret_cast<float*>(lparam)), yScale);
    return 0;
}

LRESULT MainWindow::OnWvltDraw(LPARAM lParam)
{
    VectorXf* curves = reinterpret_cast<VectorXf*>(lParam);
    int i;
    LRESULT res;
    for (i = 0; i < 3; i++)
    {
        *(curves + i) /= yScale;
        res = SendMessage(graphs[i].m_hwnd, WVLTDRAW, NULL, reinterpret_cast<LPARAM>(curves + i));
    }
    return res; //look at this later?
}

LRESULT MainWindow::OnDWDraw(LPARAM lParam)
{
    VectorXf* pwConst = reinterpret_cast<VectorXf*>(lParam);
    int i;
    LRESULT res;
    for (i = 0; i < 3; i++)
    {
        *(pwConst + i) /= yScale;
        res = SendMessage(graphs[i].m_hwnd, DATAWDRAW, NULL, reinterpret_cast<LPARAM>(pwConst + i));
    }
    return res;
}
