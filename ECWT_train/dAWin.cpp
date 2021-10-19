#include "DBMem.h"

#include "dAWin.h"

ID2D1Factory* dAWin::m_pDirect2dFactory = NULL;
IDWriteFactory* dAWin::m_pWriteFactory = NULL;
IDWriteTextFormat* dAWin::pTextFormat = NULL;

HRESULT dAWin::OnCreate()
{
    HRESULT hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
        hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
        hr = GraphSetUp();
    if (SUCCEEDED(hr))
        ShowWindow(m_hwnd, SW_SHOW);
    return hr;
}

void dAWin::Resize()
{
    HWND hPar = GetAncestor(m_hwnd, GA_PARENT);
    RECT parRec;
    GetClientRect(hPar, &parRec);
    int num = GetDlgCtrlID(m_hwnd) - 100;
    int cheight = (3 * (parRec.bottom - parRec.top)) / 5;
    int chwidth = (parRec.right - parRec.left - 40) / 3;
    SetWindowPos(m_hwnd, HWND_TOP, num * (chwidth + 10) + 10, (parRec.bottom - parRec.top) / 6, chwidth, cheight,
        SWP_SHOWWINDOW);
    if (FAILED(GraphSetUp()))
        exit(-1);
}

void dAWin::OnPaint()
{
    if (FAILED(GraphSetUp()))
        exit(-1);
    /*    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    HBRUSH brush = CreateSolidBrush(RGB(225, 255, 255));
    FillRect(hdc, &ps.rcPaint, brush);

    EndPaint(m_hwnd, &ps);
    */
}

LRESULT dAWin::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //    int i;
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate();

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_SIZE:
        Resize();
        return 0;

    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

HRESULT dAWin::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pWriteFactory));
    }
    if (SUCCEEDED(hr))
        hr = m_pWriteFactory->CreateTextFormat(L"Gabriola", // Font family name.
            NULL,   // Font collection (NULL sets it to use the system font collection).
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.0f,
            L"en-GB",
            &pTextFormat);
    return hr;
}

// Initialize device-dependent resources.
HRESULT dAWin::CreateDeviceResources()
{
    if (!m_pDirect2dFactory)
        CreateDeviceIndependentResources();

    // Create a Direct2D render target.
    HRESULT hr = S_OK;
    if (!m_pRenderTarget)
    {
        DiscardDeviceResources();
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
        props.pixelFormat = pixelFormat;

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = m_pDirect2dFactory->CreateHwndRenderTarget(props,
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget);
    }
    if (SUCCEEDED(hr) && !m_pBlackBrush)
    {
        // Create a black brush.
        D2D1::ColorF clf(0, 0, 0, 1);
        hr = m_pRenderTarget->CreateSolidColorBrush(clf, &m_pBlackBrush);
    }
    if (SUCCEEDED(hr) && !m_pBlueBrush)
    {
		// Create a blue brush.
		D2D1::ColorF clf(0, 0, 1, 1);
		hr = m_pRenderTarget->CreateSolidColorBrush(clf, &m_pBlueBrush);
	}
    if (SUCCEEDED(hr) && !m_pRedBrush)
    {
		// Create a red brush.
		D2D1::ColorF clf(1, 0, 0, 1);
		hr = m_pRenderTarget->CreateSolidColorBrush(clf, &m_pRedBrush);
	}
    return hr;
}

// Draw content.
HRESULT OnRender()
{
    HRESULT hr = TRUE;
    return hr;
}

HRESULT dAWin::GraphSetUp()
{
    HRESULT hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(0.9f, 1.f, 1.f, 0.f));
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
        float x = rtSize.width / 50.f;
        m_pRenderTarget->DrawLine(D2D1::Point2F(x, 0.0f),
            D2D1::Point2F(x, rtSize.height),
            m_pBlackBrush,
            0.5f);
        float y = rtSize.height / 2.f;
        m_pRenderTarget->DrawLine(D2D1::Point2F(0.0f, y),
            D2D1::Point2F(rtSize.width, y),
            m_pBlackBrush,
            0.5f);
        DrawText();
        hr = m_pRenderTarget->EndDraw();
    }
    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }
    return hr;
}

void dAWin::DrawText()
{
    int num = GetDlgCtrlID(m_hwnd) - 100;
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    UINT w = rc.right - rc.left, h = rc.bottom - rc.top;
    D2D1_RECT_F layoutRect = D2D1::RectF((static_cast<FLOAT>(rc.left + w / 25)),
        0.f,
        static_cast<FLOAT>(w),
        static_cast<FLOAT>(h));
    wchar_t text[10];
    switch (num)
    {
    case 0:
        wcscpy_s(text, L"Channel 1");
        break;
    case 1:
        wcscpy_s(text, L"Channel 2");
        break;
    case 2:
        wcscpy_s(text, L"Channel 3");
    }
    m_pRenderTarget->DrawText(text, wcslen(text), pTextFormat, layoutRect, m_pBlackBrush);
}

