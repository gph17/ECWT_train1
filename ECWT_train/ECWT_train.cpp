#include <atlbase.h>
#include <shobjidl.h>
#include <windows.h>

#include "Lib.h"
#include "MainWindow.h"

using namespace std;
using namespace Eigen;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    MainWindow win;

    if (!win.Create(L"ECWT - Unsupervised training", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}



