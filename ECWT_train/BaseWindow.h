#pragma once

#include "DBMem.h"
#include "util.h"

template <typename DERIVED_TYPE>
class BaseWindow
{
    HRESULT CreateDeviceIndependentResources()
    {
        HRESULT hr = S_OK;
        return hr;
        //do nothing else unless overridden
    }
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE* pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow() : m_hwnd(NULL) { }

    BOOL Create(PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0);

    HWND Window() const { return m_hwnd; }

protected:

    virtual PCWSTR  ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;
};

template<typename DERIVED_TYPE>
BOOL BaseWindow<DERIVED_TYPE>::Create(PCWSTR lpWindowName,
    DWORD dwStyle,
    DWORD dwExStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu)
{
    if (DERIVED_TYPE::registered != DONE)
    {
        if (DERIVED_TYPE::registered == NEEDPLAIN)
        {
            WNDCLASS wc = { 0 };

            wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = ClassName();

            RegisterClass(&wc);
        }
        else
        {
            HRESULT hr;

            // Initialize device-indpendent resources, such
            // as the Direct2D factory.
            hr = ((DERIVED_TYPE*)this)->CreateDeviceIndependentResources();
            if (SUCCEEDED(hr))
            {
                WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

                // Register the window class.
                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc = DERIVED_TYPE::WindowProc;
                wcex.cbClsExtra = 0;
                wcex.cbWndExtra = sizeof(LONG_PTR);
                wcex.hInstance = GetModuleHandle(NULL);
                wcex.hbrBackground = NULL;
                wcex.lpszMenuName = NULL;
                wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
                wcex.lpszClassName = ClassName();

                RegisterClassEx(&wcex);

            }
        }
        DERIVED_TYPE::registered = DONE;
    }

    m_hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
        nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this);

    return (m_hwnd ? TRUE : FALSE);
}
