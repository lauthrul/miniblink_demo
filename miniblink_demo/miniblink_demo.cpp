// miniblink_demo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <windowsx.h>
#include "miniblink_demo.h"
#include "../miniblink/wke.h"
#include "RenderGDI.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
TCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND g_hWndMain = NULL;
wkeWebView g_webView = NULL;
CRenderGDI* g_render = NULL;
int g_cursorInfoType = 0;

#include "miniblink_demo.hpp"

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。
    MSG msg;
    HACCEL hAccelTable;

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MINIBLINK_DEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINIBLINK_DEMO));

    // 主消息循环:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINIBLINK_DEMO));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MINIBLINK_DEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    g_hWndMain = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!g_hWndMain)
    {
        return FALSE;
    }

    InitWebView();

    ShowWindow(g_hWndMain, nCmdShow);
    UpdateWindow(g_hWndMain);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = false;
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDM_NAV_BACKWARD:
                    if (wkeCanGoBack(g_webView))
                        wkeGoBack(g_webView);
                    break;
                case IDM_NAV_FORWARD:
                    if (wkeCanGoForward(g_webView))
                        wkeGoForward(g_webView);
                    break;
                case IDM_NAV_REFRESH:
                    wkeReload(g_webView);
                    break;
                case IDM_NAV_FULLSCREEN:
                    setFullScreen();
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            g_render->renderOnWindowPaint(g_webView, ps.hdc);
            EndPaint(hWnd, &ps);
            break;
        case WM_SIZE:
            if (g_webView && g_render) {
                wkeResize(g_webView, LOWORD(lParam), HIWORD(lParam));
                g_render->resize(LOWORD(lParam), HIWORD(lParam));
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SETCURSOR:
            if (setCursorInfoTypeByCache(hWnd))
                return 0;
            break;
        case WM_KEYDOWN:
            {
                unsigned int virtualKeyCode = wParam;
                unsigned int flags = 0;
                if (HIWORD(lParam) & KF_REPEAT)
                    flags |= WKE_REPEAT;
                if (HIWORD(lParam) & KF_EXTENDED)
                    flags |= WKE_EXTENDED;

                handled = wkeFireKeyDownEvent(g_webView, virtualKeyCode, flags, false);
            }
            break;
        case WM_KEYUP:
            {
                unsigned int virtualKeyCode = wParam;
                unsigned int flags = 0;
                if (HIWORD(lParam) & KF_REPEAT)
                    flags |= WKE_REPEAT;
                if (HIWORD(lParam) & KF_EXTENDED)
                    flags |= WKE_EXTENDED;

                handled = wkeFireKeyUpEvent(g_webView, virtualKeyCode, flags, false);
            }
            break;
        case WM_CHAR:
            {
                unsigned int charCode = wParam;
                unsigned int flags = 0;
                if (HIWORD(lParam) & KF_REPEAT)
                    flags |= WKE_REPEAT;
                if (HIWORD(lParam) & KF_EXTENDED)
                    flags |= WKE_EXTENDED;

                handled = wkeFireKeyPressEvent(g_webView, charCode, flags, false);
            }
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK: //
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
            {
                onCursorChange();

                if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN) {
                    SetFocus(hWnd);
                    SetCapture(hWnd);
                } else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP) {
                    ReleaseCapture();
                }

                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);

                unsigned int flags = 0;

                if (wParam & MK_CONTROL)
                    flags |= WKE_CONTROL;
                if (wParam & MK_SHIFT)
                    flags |= WKE_SHIFT;

                if (wParam & MK_LBUTTON)
                    flags |= WKE_LBUTTON;
                if (wParam & MK_MBUTTON)
                    flags |= WKE_MBUTTON;
                if (wParam & MK_RBUTTON)
                    flags |= WKE_RBUTTON;

                //flags = wParam;

                handled = wkeFireMouseEvent(g_webView, message, x, y, flags);
            }
            break;
        case WM_CONTEXTMENU:
            {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);

                if (pt.x != -1 && pt.y != -1)
                    ScreenToClient(hWnd, &pt);

                unsigned int flags = 0;

                if (wParam & MK_CONTROL)
                    flags |= WKE_CONTROL;
                if (wParam & MK_SHIFT)
                    flags |= WKE_SHIFT;

                if (wParam & MK_LBUTTON)
                    flags |= WKE_LBUTTON;
                if (wParam & MK_MBUTTON)
                    flags |= WKE_MBUTTON;
                if (wParam & MK_RBUTTON)
                    flags |= WKE_RBUTTON;

                handled = wkeFireContextMenuEvent(g_webView, pt.x, pt.y, flags);
            }
            break;
        case WM_MOUSEWHEEL:
            {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &pt);

                int delta = GET_WHEEL_DELTA_WPARAM(wParam);

                unsigned int flags = 0;

                if (wParam & MK_CONTROL)
                    flags |= WKE_CONTROL;
                if (wParam & MK_SHIFT)
                    flags |= WKE_SHIFT;

                if (wParam & MK_LBUTTON)
                    flags |= WKE_LBUTTON;
                if (wParam & MK_MBUTTON)
                    flags |= WKE_MBUTTON;
                if (wParam & MK_RBUTTON)
                    flags |= WKE_RBUTTON;

                //flags = wParam;

                handled = wkeFireMouseWheelEvent(g_webView, pt.x, pt.y, delta, flags);
            }
            break;
        case WM_SETFOCUS:
            wkeSetFocus(g_webView);
            break;
        case WM_KILLFOCUS:
            wkeKillFocus(g_webView);
            break;
        case WM_IME_STARTCOMPOSITION:
            {
                wkeRect caret = wkeGetCaretRect(g_webView);

                CANDIDATEFORM form;
                form.dwIndex = 0;
                form.dwStyle = CFS_EXCLUDE;
                form.ptCurrentPos.x = caret.x;
                form.ptCurrentPos.y = caret.y + caret.h;
                form.rcArea.top = caret.y;
                form.rcArea.bottom = caret.y + caret.h;
                form.rcArea.left = caret.x;
                form.rcArea.right = caret.x + caret.w;

                HIMC hIMC = ImmGetContext(hWnd);
                ImmSetCandidateWindow(hIMC, &form);
                ImmReleaseContext(hWnd, hIMC);
            }
            break;
        default:
            handled = false;
            break;
    }

    if (!handled)
        return DefWindowProc(hWnd, message, wParam, lParam);

    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
