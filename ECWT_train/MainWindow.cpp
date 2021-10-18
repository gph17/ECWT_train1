#include "DBMem.h"

#include <shobjidl.h>
#include <sstream>
#include <string>
#include <windowsx.h>

#include "MainWindow.h"
#include "PWvlet.h"
#include "util.h"

using namespace std;

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


regStat dAWin::registered = NEEDSPECIAL;
regStat MainWindow::registered = NEEDPLAIN;

const COLORREF MainWindow::bgCol = RGB(180, 150, 150);
const HBRUSH MainWindow::hbrBg = CreateSolidBrush(MainWindow::bgCol);

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
            std::wstring str1 = pFilePath;
            wchar_t pStr2[128];
            GetWindowText(m_hwnd, pStr2, 128);
            std::wstring str2 = pStr2;
            std::wstring str3 = str2 + L": " + str1;
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
    RECT rec;
    GetClientRect(GetAncestor(AbButton, GA_PARENT), &rec);
    int y = (9 * (rec.bottom - rec.top)) / 10;
    SetWindowPos(AbButton, HWND_TOP, 25, y, 100, 50, NULL);


    int i;
    if (butType == BGBUTTON)
    {
        //hide drawing areas
        for (i = 0; i < 3; i++)
            ShowWindow(graphs[i].m_hwnd, SW_HIDE);
    }
    else
        if (butType == DEBUTTON)
        {
            //enable display of new ECWTs in drawing areas
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
    if (dAWin::registered == DONE)
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
		lpMMI->ptMinTrackSize.x = 800;
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
            (&graphs[i])->Create(L"dAWin",
                WS_CHILD | WS_BORDER | WS_VISIBLE, NULL,
				i * (chwidth + 10) + 10, (rec.bottom - rec.top) / 6, chwidth, cheight,
				m_hwnd,
				(HMENU)(100 + i));
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

        if (!(WSEditLT = CreateCntrl(L"STATIC",
            L"m:M or m:s:M",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WSEDITLT)))
            return(-1);
        EnableWindow(WSEditLT, FALSE);

        if (!(WSEdit = CreateCntrl(L"EDIT",
            NULL,
            WS_VISIBLE | ES_LEFT | ES_READONLY,
            m_hwnd,
            WSEDIT)))
            return(-1);

        if (!(WSHEditL = CreateCntrl(L"STATIC",
            L"Window Shift",
            WS_VISIBLE | SS_LEFT,
            m_hwnd,
            WSHEDITL)))
            return(-1);
        EnableWindow(WSHEditL, FALSE);

        if (!(WSHEdit = CreateCntrl(L"EDIT",
            NULL,
            WS_VISIBLE | ES_LEFT | ES_READONLY,
            m_hwnd,
            WSHEDIT)))
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
        }
		SetWindowPos(hwnd, HWND_TOP, x, y, width, height, NULL);
	}
	return TRUE;
}

LRESULT MainWindow::ChangeStatCol(WPARAM wParam, LPARAM lParam)
{
    if (((HWND)lParam == WSEdit) || ((HWND)lParam == WSHEdit))
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    HDC hdcStatic = (HDC)wParam;
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
    if ((HWND)lParam == WSHEdit)
    {
        if (WSHEditRed)
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
        (HMENU)CntrlNum,
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
        SendMessage((HWND)lParam, LB_GETTEXT, (WPARAM)ind, (LPARAM)val);
        if ((LBId == DELB) && (LB_MESS == LBN_SELCHANGE))
        {
            deg = _wtoi(val);
            set<pair<int, int>> vals = PWvlet::getValid(deg);
            wchar_t cWStr[9];
            wchar_t num[4];
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
            vS >> cNo;
            vS.ignore(2);
            vS >> wCNo;
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
    if (LB_MESS == EN_CHANGE)
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
            LSz = (int)round(dLSz);
        if (LSz > 5e4)
        {
            LSz = 0;
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
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        _CrtDumpMemoryLeaks();
        exit(-1);//do something more sophisticated?
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
    }
	return 0;
}

LRESULT MainWindow::OnWSChange(WPARAM wParam, LPARAM lParam)
{

/*    int EBId = LOWORD(wParam);
    UINT EB_MESS = HIWORD(wParam);
    if (EB_MESS == LBN_SELCHANGE)
        if ()
    {

    }
*/        return 0;
}
