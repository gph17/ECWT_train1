#include "DBMem.h"

#include <shobjidl.h>
#include <string>

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


regStat dAWin::registered = NEEDSPECIAL;
regStat MainWindow::registered = NEEDPLAIN;

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
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        pItem->Release();
                    }
                }
            }
        }
    }
    CoUninitialize();
    if (SUCCEEDED(hr))
    {
        if (csvCheck(pszFilePath))
        {
            //display file path, inactivate Browse Button, activate other buttons
            std::wstring str1 = pszFilePath;
            wchar_t pStr2[128];
            GetWindowText(m_hwnd, pStr2, 128);
            std::wstring str2 = pStr2;
            std::wstring str3 = str2 + L": " + str1;
            SetWindowText(m_hwnd, str3.c_str());
            EnableWindow(BrButton, FALSE);
            ShowWindow(BrButton, FALSE);
            EnableWindow(BgButton, TRUE);
            EnableWindow(demoButton, TRUE);
        }
    }
}

void MainWindow::OnProcess(int butType)
{
    //put common stuff before or after separate code for the two process buttons - setting up processing, setting 
    //  it off, setting up progress bar, enabling process interruption
    ShowWindow(BgButton, SW_HIDE);
    ShowWindow(demoButton, SW_HIDE);
    ShowWindow(AbButton, SW_SHOW);
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

	HBRUSH brush = CreateSolidBrush(RGB(180, 150, 150));
	FillRect(hdc, &ps.rcPaint, brush);
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
    int i;
    switch (uMsg)
    {
	case WM_CREATE:
		return OnCreate();

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        ValidateRect(m_hwnd, NULL);
        return 0;

    case WM_SIZE:
        Resize();
        return 0;

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 800;
        lpMMI->ptMinTrackSize.y = 600;
    }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ABBUTTON)
        {
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
            _CrtDumpMemoryLeaks();
            exit(-1);//do something more sophisticated?
        }
        else
            if (LOWORD(wParam) == BRBUTTON)
            {
                OnBrowse();
            }
            else
                if (LOWORD(wParam) == BGBUTTON)
                {
                    OnProcess(BGBUTTON);    //do less
                }
                else
                {
                    OnProcess(DEBUTTON);    //do more
                }
        return 0;
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
		if (!(demoButton = CreateWindow(L"BUTTON",
			L"Build Library (demo)",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
			25, (9 * (rec.bottom - rec.top)) / 10, 100, 50,
			m_hwnd,
			(HMENU)DEBUTTON,
			(HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
			NULL)))
			return(-1);
		EnableWindow(demoButton, FALSE);
		if (!(BgButton = CreateWindow(L"BUTTON",
			L"Build Library (background)",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
			135, (9 * (rec.bottom - rec.top)) / 10, 100, 50,
			m_hwnd,
			(HMENU)BGBUTTON,
			(HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
			NULL)))
			return(-1);
		EnableWindow(BgButton, FALSE);
		if (!(BrButton = CreateWindow(L"BUTTON",
			L"Browse",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            25, ((rec.bottom - rec.top)) / 25, 100, 50,
            m_hwnd,
			(HMENU)BRBUTTON,
			(HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
			NULL)))
			return(-1);
        SendMessage(BrButton, WM_PAINT, NULL, NULL);
        if (!(AbButton = CreateWindow(L"BUTTON",
            L"Abort",
            WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            25, (9 * (rec.bottom - rec.top)) / 10, 100, 50,
            m_hwnd,
            (HMENU)ABBUTTON,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL)))
            return(-1);
        int chwidth2 = (rec.right - rec.left - 40) / 3; //width of graph
        if (!(degreeLB = CreateWindow(L"LISTBOX",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS,
            chwidth2 + 100, (rec.bottom - rec.top) / 25, 50, 50,
            m_hwnd,
            (HMENU)DELB,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL)))
            return(-1);
        SendMessage(degreeLB, WM_PAINT, NULL, NULL);
        set<int> ns = PWvlet::getValid();
        wchar_t num[4];
        for (int item : ns)
        {
            _itow_s(item, num, 10);
            SendMessage(degreeLB, LB_ADDSTRING, NULL, (LPARAM)num);
        }
        if (!(degreeLBL = CreateWindow(L"STATIC",
            L"Degree",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTER,
            chwidth2 + 20, (rec.bottom - rec.top) / 25, 50, 50,
            m_hwnd,
            (HMENU)DELBL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL)))
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
		int x = 25, y = (9 * cRH) / 10;
		int width = 100;
		switch (bNo)
		{
		case BGBUTTON:
			x = 135;
			break;
		case BRBUTTON:
			y = cRH / 25;
			break;
		case DELB:
		{
			int chwidth = (rec->right - rec->left - 40) / 3;
			x = chwidth + 100;
			y = cRH / 25;
			width = 50;
			break;
		}
		case DELBL:
		{
			int chwidth = (rec->right - rec->left - 40) / 3;
			x = chwidth + 20;
			y = cRH / 25;
			width = 50;
		}
		}
        SetWindowPos(hwnd, HWND_TOP, x, y, width, 50, NULL);
    }
    return TRUE;
}

