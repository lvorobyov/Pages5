/**
 * main.cpp
 * Утилита для сортировки страниц брошуры
 * Copyright (c) 2018 Lev Vorobjev
 */

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "dialog.h"

#define IDC_LISTVIEW  40050
//#define IDC_BTNSOLVE  40051

#define WND_TITLE TEXT("Сортировщик страниц")
#define WND_MENU_NAME MAKEINTRESOURCE(IDR_APPMENU)
#define MSG_TITLE TEXT("Pages5")
#define BUFFER_SIZE 512

#define HANDLE_ERROR(lpszFunctionName, dwStatus) \
    MultiByteToWideChar(CP_ACP, 0, \
        lpszFunctionName, -1, lpszBuffer, BUFFER_SIZE); \
    _stprintf(lpszBuffer, TEXT("%s error.\nStatus code: %d"), \
        lpszBuffer, dwStatus); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM RegMyWindowClass(HINSTANCE, LPCTSTR);
BOOL CALLBACK SolvePaneProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
             HINSTANCE         hPrevInstance,
             LPSTR             lpCmdLine,
             int               nCmdShow) {
    LPCTSTR lpszClass = TEXT("Pages5_Window");
    LPCTSTR lpszTitle = WND_TITLE;
    HWND hWnd;
    MSG msg = {0};
    BOOL status;

    if (!RegMyWindowClass(hInstance, lpszClass))
        return 1;

    hWnd = CreateWindow(lpszClass, lpszTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);
    if(!hWnd) return 2;

    ShowWindow(hWnd, nCmdShow);

    while ((status = GetMessage(&msg, NULL, 0, 0 )) != 0) {
        if (status == -1) return 3;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

ATOM RegMyWindowClass(HINSTANCE hInst, LPCTSTR lpszClassName) {
    WNDCLASS wcWindowClass = {0};
    wcWindowClass.lpfnWndProc = (WNDPROC)WndProc;
    wcWindowClass.style = CS_HREDRAW|CS_VREDRAW;
    wcWindowClass.hInstance = hInst;
    wcWindowClass.lpszClassName = lpszClassName;
    wcWindowClass.lpszMenuName = WND_MENU_NAME;
    wcWindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcWindowClass.hbrBackground = (HBRUSH) ( COLOR_WINDOW + 1);
    wcWindowClass.cbClsExtra = 0;
    wcWindowClass.cbWndExtra = 0;
    return RegisterClass(&wcWindowClass);
}

typedef struct _solve_pane_context_s {
    LONG lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    LPTSTR lpszBuffer;
    HWND hListView;
    int nSheets;
    int* face;
    int* back;
} solve_pane_context_t;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    HINSTANCE hInst;
    PAINTSTRUCT ps;

	static LPTSTR lpszBuffer;
	DWORD dwStatus;

	static HWND hListView;
	RECT rc = {0};

    static HIMAGELIST hImageListSmall;
    static HIMAGELIST hImageListLarge;

    static const UINT lvcMask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    static LV_COLUMN lvc = {0};
    static LV_ITEM lvi = {0};

    static solve_pane_context_t ctx = { sizeof(solve_pane_context_t) };

    static HWND hSolvePane;
    static int nPaneHeight;

#if 1
    static HFONT hFont;
    static LOGFONT lf = {0};
#endif

	switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		lpszBuffer = (LPTSTR)malloc(BUFFER_SIZE*sizeof(TCHAR));
		GetClientRect(hWnd, &rc);
        InitCommonControls();
        ctx.hwndOwner = hWnd;
        ctx.hInstance = hInst;
        ctx.lpszBuffer = lpszBuffer;
        ctx.hListView = hListView;
        hSolvePane = CreateDialogParam(hInst,
            MAKEINTRESOURCE(IDD_DLGSOLVE),
            hWnd,
            (DLGPROC) SolvePaneProc,
            (LPARAM) &ctx);
        GetWindowRect(hSolvePane, &rc);
        nPaneHeight = rc.bottom - rc.top;
        SetWindowPos(hSolvePane, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
		hListView = CreateWindowEx(0L, WC_LISTVIEW, TEXT(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |
            LVS_EDITLABELS | LVS_SINGLESEL | LVS_AUTOARRANGE,
            0, nPaneHeight, rc.right - rc.left,
            rc.bottom - rc.top - nPaneHeight,
            hWnd, (HMENU)IDC_LISTVIEW, hInst, NULL);
        if (hListView == NULL)
            return FALSE;
        hImageListSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), ILC_MASK, 1, 1);
        hImageListLarge = ImageList_Create(GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 1);
        // TODO: добавить иконки
        ListView_SetImageList(hListView, hImageListSmall, LVSIL_SMALL);
        ListView_SetImageList(hListView, hImageListLarge, LVSIL_NORMAL);
        // Вставка столбцов
        lvc.mask = lvcMask;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = 150;
        _tcscpy(lpszBuffer, TEXT("Номер листа"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 0;
        ListView_InsertColumn(hListView, 0, &lvc);
        _tcscpy(lpszBuffer, TEXT("Лицевая сторона"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 1;
        ListView_InsertColumn(hListView, 1, &lvc);
        _tcscpy(lpszBuffer, TEXT("Обратная сторона"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 2;
        ListView_InsertColumn(hListView, 2, &lvc);
#if 1
        lf.lfHeight = 12;
	    lf.lfWeight = FW_NORMAL;
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_TT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        _tcscpy(lf.lfFaceName, TEXT("Ms Shell Dlg"));
        hFont = CreateFontIndirect(&lf);
        SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
#endif
        SendMessage(hSolvePane, WM_COMMAND,
            (WPARAM)IDC_BTNSOLVE, (LPARAM)0L);
		break;
	  case WM_SIZE:
        SetWindowPos(hSolvePane, HWND_TOP, 0, 0,
            LOWORD(lParam), nPaneHeight, SWP_NOMOVE);
		SetWindowPos(hListView, HWND_TOP, 0, 0,
            LOWORD(lParam), HIWORD(lParam)-nPaneHeight, SWP_NOMOVE);
		break;
	  case WM_COMMAND:
        switch (LOWORD(wParam)) {
		}
		break;
	  case WM_DESTROY:
        DeleteObject(hFont);
        free(lpszBuffer);
		PostQuitMessage(0);
		break;
	  default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK SolvePaneProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static solve_pane_context_t *spCtx;
    static pages_context_t ctx = { sizeof(pages_context_t) };
    static HWND hBtnSolve;
    static HWND hEditFace;
    static HWND hEditBack;
    static HWND hLblInfo;
    switch (message) {
      case WM_INITDIALOG:
        spCtx = (solve_pane_context_t*) lParam;
        ctx.hwndOwner = spCtx -> hwndOwner;
        ctx.hInstance = spCtx -> hInstance;
        ctx.numPages = 8;
        ctx.pagesPerSheet = 2;
        ctx.firstPage = 1;
        ctx.lastPage = 8;
        hBtnSolve = GetDlgItem(hDlg, IDC_BTNSOLVE);
        hEditFace = GetDlgItem(hDlg, IDC_EDITFACE);
        hEditBack = GetDlgItem(hDlg, IDC_EDITBACK);
        hLblInfo = GetDlgItem(hDlg, IDC_LBLINFO);
        break;
#if 0
      case WM_SIZE:
        SetWindowPos(hBtnSolve, HWND_TOP,
            LOWORD(lParam)-61, 6, 0, 0, SWP_NOSIZE);
        SetWindowPos(hEditFace, HWND_TOP,
            0, 0, LOWORD(lParam)-81, 14, SWP_NOMOVE);
        SetWindowPos(hEditBack, HWND_TOP,
            0, 0, LOWORD(lParam)-81, 14, SWP_NOMOVE);
        return 0;
#endif
      case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_BTNSOLVE:
            {
                if (GetPagesParams(&ctx)) {
                    div_t n = div(ctx.numPages, ctx.pagesPerSheet);
                    int numSheets = n.quot;
                    if (n.rem != 0) {
                        numSheets ++;
                        _stprintf(spCtx->lpszBuffer, TEXT("Перед печатью документа "
                            "необходимо после %d-й страницы вставить %d пустых страниц."),
                            ctx.lastPage, numSheets*ctx.pagesPerSheet - ctx.numPages);
                        MessageBox(spCtx->hwndOwner, spCtx->lpszBuffer,
                            MSG_TITLE, MB_OK | MB_ICONINFORMATION);
                    }
                }
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}
