#pragma once

//////////////////////////////////////////////////////////////////////////
// 回调：点击了关闭、返回 true 将销毁窗口，返回 false 什么都不做。
bool handleWindowClosing(wkeWebView webWindow, void* param)
{
    return IDYES == MessageBoxW(wkeGetWindowHandle(webWindow), L"确定要退出程序吗？", L"提示", MB_YESNO | MB_ICONQUESTION);
}

// 回调：窗口已销毁
void handleWindowDestroy(wkeWebView webWindow, void* param)
{
    PostQuitMessage(0);
}

// 回调：页面标题改变
void handleTitleChanged(wkeWebView webWindow, void* param, const wkeString title)
{
    wkeSetWindowTitleW(webWindow, wkeGetStringW(title));
}

// 回调：创建新的页面，比如说调用了 window.open 或者点击了 <a target="_blank" .../>
wkeWebView onCreateView(wkeWebView webWindow, void* param, wkeNavigationType navType, const wkeString url, const wkeWindowFeatures* features)
{
    wkeWebView newWindow = wkeCreateWebWindow(WKE_WINDOW_TYPE_POPUP, NULL, features->x, features->y, features->width, features->height);
    wkeOnTitleChanged(newWindow, handleTitleChanged, NULL);
    wkeShowWindow(newWindow, true);
    return newWindow;
}

bool onDownload(wkeWebView webView, void* param, const char* url)
{
    MessageBoxA(NULL, url, "info", MB_OK);
    return false;
}

void onDataRecv(void* ptr, wkeNetJob job, const char* data, int length)
{

}

void onDataFinish(void* ptr, wkeNetJob job, wkeLoadingResult result)
{

}

wkeDownloadOpt onDownload2(
    wkeWebView webView, 
    void* param,
    size_t expectedContentLength,
    const char* url, 
    const char* mime, 
    const char* disposition, 
    wkeNetJob job, 
    wkeNetJobDataBind* dataBind)
{
    static wkeNetJobDataBind s_dataBind;
    s_dataBind.recvCallback = onDataRecv;
    s_dataBind.finishCallback = onDataFinish;
    dataBind = &s_dataBind;
    return wkeDownloadOpt::kWkeDownloadOptCacheData;
}

void onPaintUpdatedCallback(wkeWebView webView, void* param, const HDC hdc, int x, int y, int cx, int cy)
{
    g_render->renderOnBlinkPaint(g_webView, hdc, x, y, cx, cy);
}

void onCursorChange()
{
    g_cursorInfoType = wkeGetCursorInfoType(g_webView);
}

bool setCursorInfoTypeByCache(HWND hWnd)
{
    RECT rc;
    ::GetClientRect(hWnd, &rc);

    POINT pt;
    ::GetCursorPos(&pt);
    ::ScreenToClient(hWnd, &pt);
    if (!::PtInRect(&rc, pt))
        return false;

    HCURSOR hCur = NULL;
    switch (g_cursorInfoType)
    {
        case WkeCursorInfoPointer:
            hCur = ::LoadCursor(NULL, IDC_ARROW);
            break;
        case WkeCursorInfoIBeam:
            hCur = ::LoadCursor(NULL, IDC_IBEAM);
            break;
        case WkeCursorInfoHand:
            hCur = ::LoadCursor(NULL, IDC_HAND);
            break;
        case WkeCursorInfoWait:
            hCur = ::LoadCursor(NULL, IDC_WAIT);
            break;
        case WkeCursorInfoHelp:
            hCur = ::LoadCursor(NULL, IDC_HELP);
            break;
        case WkeCursorInfoEastResize:
            hCur = ::LoadCursor(NULL, IDC_SIZEWE);
            break;
        case WkeCursorInfoNorthResize:
            hCur = ::LoadCursor(NULL, IDC_SIZENS);
            break;
        case WkeCursorInfoSouthWestResize:
        case WkeCursorInfoNorthEastResize:
            hCur = ::LoadCursor(NULL, IDC_SIZENESW);
            break;
        case WkeCursorInfoSouthResize:
        case WkeCursorInfoNorthSouthResize:
            hCur = ::LoadCursor(NULL, IDC_SIZENS);
            break;
        case WkeCursorInfoNorthWestResize:
        case WkeCursorInfoSouthEastResize:
            hCur = ::LoadCursor(NULL, IDC_SIZENWSE);
            break;
        case WkeCursorInfoWestResize:
        case WkeCursorInfoEastWestResize:
            hCur = ::LoadCursor(NULL, IDC_SIZEWE);
            break;
        case WkeCursorInfoNorthEastSouthWestResize:
        case WkeCursorInfoNorthWestSouthEastResize:
            hCur = ::LoadCursor(NULL, IDC_SIZEALL);
            break;
        default:
            hCur = ::LoadCursor(NULL, IDC_ARROW);
            break;
    }

    if (hCur)
    {
        ::SetCursor(hCur);
        return true;
    }

    return false;
}

void setFullScreen()
{
    static bool fullScreen = false;
    static LONG oldStyle = 0;
    static RECT oldRect = {0};

    if (!fullScreen)
    {
        GetWindowRect(g_hWndMain, &oldRect);
        oldStyle = GetWindowLong(g_hWndMain, GWL_STYLE);

        int frameWidth =  GetSystemMetrics(SM_CXFRAME);
        int frameHeight = GetSystemMetrics(SM_CYFRAME);
        int captionHeight = GetSystemMetrics(SM_CYCAPTION);
        int menuHeight = GetSystemMetrics(SM_CYMENU);
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        SetWindowLong(g_hWndMain, GWL_STYLE, WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowPos(g_hWndMain, HWND_TOPMOST, 0, -menuHeight, screenWidth, screenHeight + menuHeight, SWP_SHOWWINDOW);
    }
    else
    {
        SetWindowLong(g_hWndMain, GWL_STYLE, oldStyle);
        SetWindowPos(g_hWndMain, HWND_NOTOPMOST, oldRect.left, oldRect.top, oldRect.right - oldRect.left, oldRect.bottom - oldRect.top, SWP_SHOWWINDOW);
    }
    fullScreen = !fullScreen;
}

void InitWebView()
{
    char buf[MAX_PATH] = {0};
    GetPrivateProfileStringA("config", "url", "www.baidu.com", buf, MAX_PATH, "./config.ini");

    wkeSetWkeDllPath(_T("node.dll"));
    wkeInitialize();

    g_webView = wkeCreateWebView();

    wkeSetHandle(g_webView, g_hWndMain);

    wkeOnPaintUpdated(g_webView, onPaintUpdatedCallback, g_hWndMain);

    RECT rc = { 0 };
    ::GetClientRect(g_hWndMain, &rc);
    wkeResize(g_webView, rc.right, rc.bottom);

    wkeSetWindowTitleW(g_webView, _T("miniblink demo"));
    wkeOnWindowClosing(g_webView, handleWindowClosing, NULL);
    wkeOnWindowDestroy(g_webView, handleWindowDestroy, NULL);
    wkeOnTitleChanged(g_webView, handleTitleChanged, NULL);
    wkeOnCreateView(g_webView, onCreateView, NULL);
//     wkeOnDownload(g_webView, onDownload, NULL);
//     wkeOnDownload2(g_webView, onDownload2, NULL);
    wkeLoadURL(g_webView, buf);
    wkeMoveToCenter(g_webView);
    wkeShowWindow(g_webView, TRUE);

    g_render = new CRenderGDI();
    g_render->init(g_hWndMain);
}