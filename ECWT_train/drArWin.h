#pragma once

#include <atlbase.h>
#include <d2d1.h>
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d2d1")
#include <d2d1helper.h>
#include <dwrite.h>
#include <malloc.h>
#include <math.h>
#include <memory>
#include <stdlib.h>
#include <wchar.h>
#include <wincodec.h>
#include <windows.h>

#include "BaseWindow.h"
#include "MainWindow.h"


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class dArWin : public BaseWindow<dArWin>
{
    HRESULT OnCreate();
    void OnPaint();
    void Resize();
    static regStat registered;
    static IDWriteFactory* m_pWriteFactory;
    static IDWriteTextFormat* pTextFormat;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pBlackBrush;
    ID2D1SolidColorBrush* m_pBlueBrush;
    ID2D1SolidColorBrush* m_pRedBrush;
public:

    dArWin():    m_pRenderTarget(NULL), 
                m_pRedBrush(NULL), 
                m_pBlueBrush(NULL), 
                m_pBlackBrush(NULL)
    {
    }

    ~dArWin()
    {
        DiscardDeviceResources();
    }
    HRESULT GraphSetUp();
private:
    PCWSTR  ClassName() const { return L"Drawing Area"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    void DiscardDeviceResources()   //replace by using smart pointer
    {
        _CrtMemState s1;
        _CrtMemCheckpoint(&s1);
        _CrtMemState s2, s3;
		SafeRelease(&m_pRenderTarget);
        SafeRelease(&m_pRedBrush);
        SafeRelease(&m_pBlueBrush);
        SafeRelease(&m_pBlackBrush);
        _CrtMemCheckpoint(&s2);
        if (_CrtMemDifference(&s3, &s1, &s2))
            _CrtMemDumpStatistics(&s3);
    }

    // Draw content.
    HRESULT OnRender();
    void DrawText();

    // Resize the render target.
    void OnResize(UINT width, UINT height);
    friend class MainWindow;
    friend class BaseWindow;
};

