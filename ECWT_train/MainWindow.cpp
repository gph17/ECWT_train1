#include "DBMem.h"

#include <shobjidl.h>
#include <string>

#include "MainWindow.h"
#include "util.h"

#define BRBUTTON 1
#define BGBUTTON 2
#define DEBUTTON 3
#define ABBUTTON 4

regStat ButtonB::registered = DONE;
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
            EnableWindow(BrButton.m_hwnd, FALSE);
            ShowWindow(BrButton.m_hwnd, FALSE);
            EnableWindow(BgButton.m_hwnd, TRUE);
            EnableWindow(demoButton.m_hwnd, TRUE);
        }
    }

}

void MainWindow::OnProcess(int butType)
{
    //put common stuff before or after separate code for the two process buttons - setting up processing, setting 
    //  it off, setting up progress bar, enabling process interruption
    ShowWindow(BgButton.m_hwnd, SW_HIDE);
    ShowWindow(demoButton.m_hwnd, SW_HIDE);
    ShowWindow(AbButton.m_hwnd, SW_SHOW);
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

	EndPaint(m_hwnd, &ps);
}

void MainWindow::Resize()
{
    if (dAWin::registered == DONE)
    {
        int i;
        for (i = 0; i < 3; i++)
        {
            graphs[i].Resize();
        }

        demoButton.Resize();
        BgButton.Resize();
        BrButton.Resize();
        AbButton.Resize();
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
                (HMENU)(int)(100 + i));
            if (FAILED(graphs[i].GraphSetUp()))
                exit(-1);
        }
        if (!demoButton.Create(L"Build Library (demo)",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            NULL, 25, (17 * (rec.bottom - rec.top))/20, 100, 50,
            m_hwnd,
            (HMENU)DEBUTTON))
            return(-1);
        EnableWindow(demoButton.m_hwnd, FALSE);
        if (!BgButton.Create(L"Build Library (background)",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            NULL, 0, 0, 100, 50,
            m_hwnd,
            (HMENU)BGBUTTON))
            return(-1);
        EnableWindow(BgButton.m_hwnd, FALSE);
        if (!BrButton.Create(L"Browse for source",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            NULL, 0, 0, 100, 50,
            m_hwnd,
            (HMENU)BRBUTTON))
            return(-1);
        if (!AbButton.Create(L"Abort",
            WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,
            NULL, 0, 0, 100, 50,
            m_hwnd,
            (HMENU)ABBUTTON))
            return(-1);
        return 1;
    }
    else
        return -1;
}