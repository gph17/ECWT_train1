#ifndef UNICODE
#define UNICODE
#endif 

#include "DBMem.h"

#include <atlbase.h>
#include <shobjidl.h>
#include <windows.h>

#include "Lib.h"
#include "MainWindow.h"

using namespace std;
using namespace Eigen;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
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


    /*	list<int> W{90, 95, 100, 105, 110};
        Lib<PWvlet> Lib1("C:\\Users\\gph3\\dummyCPP.csv", 7, 0, 0, 0.999, W, 100);
        Lib1.dumpGoFStatsIn();
        Lib1.dumpGoFStats();*/

    return 0;
}



/*    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CComPtr<IFileOpenDialog> pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);
            if (SUCCEEDED(hr))
            {
                CComPtr<IShellItem> pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        MessageBoxW(NULL, pFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pFilePath);
                    }

                }
            }
        }
        CoUninitialize();
    }


*/