#include "DBMem.h"

#include "ButtonB.h"
#define BRBUTTON 1
#define BGBUTTON 2
#define DEBUTTON 3
#define ABBUTTON 4


void ButtonB::Resize()
{
    RECT rec;
    GetClientRect(GetAncestor(m_hwnd, GA_PARENT), &rec);
    int cRH = rec.bottom - rec.top;
    int i = GetDlgCtrlID(m_hwnd);
    int x = 25, y = (17 * cRH) / 20;
    if (i == BRBUTTON)
        y = cRH / 20;
    else
        if (i == BGBUTTON)
            x = 135;
  
    SetWindowPos(m_hwnd, HWND_TOP, x, y, 100, 50, NULL);
    ShowWindow(m_hwnd, SW_SHOW);
}

LRESULT ButtonB::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
