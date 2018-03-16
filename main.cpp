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

#define IDC_LISTVIEW  40050

#define WND_TITLE TEXT("Сортировщик таблиц")
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

	switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		lpszBuffer = (LPTSTR)malloc(BUFFER_SIZE*sizeof(TCHAR));
		GetClientRect(hWnd, &rc);
        InitCommonControls();
		hListView = CreateWindowEx(0L, WC_LISTVIEW, TEXT(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |
            LVS_EDITLABELS | LVS_SINGLESEL | LVS_AUTOARRANGE,
            0, 0, rc.right - rc.left, rc.bottom - rc.top,
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
		break;
	  case WM_SIZE:
		SetWindowPos(hListView, HWND_TOP, 0, 0,
            LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE);
		break;
	  case WM_COMMAND:
        switch (LOWORD(wParam)) {
		}
		break;
	  case WM_DESTROY:
        free(lpszBuffer);
		PostQuitMessage(0);
		break;
	  default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
