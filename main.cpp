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
#include <io.h>
#include <fcntl.h>
#include "resource.h"
#include "dialog.h"
#include "pages.h"
#include "tcstok_n.h"

#define IDC_LISTVIEW  40050
#define IDC_STATUSBAR 40051

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
    wcWindowClass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcWindowClass.hbrBackground = (HBRUSH) ( COLOR_WINDOW + 1);
    wcWindowClass.cbClsExtra = 0;
    wcWindowClass.cbWndExtra = 0;
    return RegisterClass(&wcWindowClass);
}

typedef struct _solve_table_item_s {
    int nSheet;
    LPTSTR lpszFace;
    LPTSTR lpszBack;
} solve_table_item_t;

typedef struct _solve_pane_context_s {
    LONG lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    LPTSTR lpszBuffer;
    HWND hListView;
    HWND hStatusBar;
    int nSheets;
    int nPagesPerSide;
    uint32_t* dwPages;
    LPTSTR lpszPages;
    solve_table_item_t* items;
} solve_pane_context_t;

static
int GetWindowHeight(HWND hWnd) {
    RECT rcWnd = {0};
    GetWindowRect(hWnd, &rcWnd);
    return (rcWnd.bottom - rcWnd.top);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    HINSTANCE hInst;
    PAINTSTRUCT ps;

	static LPTSTR lpszBuffer;
	DWORD dwStatus;

	RECT rc = {0};

    static HIMAGELIST hImageListSmall;
    static HIMAGELIST hImageListLarge;
    HICON hIcon;

    static const UINT lvcMask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    static LV_COLUMN lvc = {0};
    static LV_ITEM lvi = {0};

    static solve_pane_context_t ctx = { sizeof(solve_pane_context_t) };

    static HWND hSolvePane;
    static int nPaneHeight;
    static int nStatusBarHeight;

    static OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    const int nMaxFile = 80;

	switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		lpszBuffer = (LPTSTR)calloc(BUFFER_SIZE,sizeof(TCHAR));
		GetClientRect(hWnd, &rc);
        InitCommonControls();
        ctx.hwndOwner = hWnd;
        ctx.hInstance = hInst;
        ctx.lpszBuffer = lpszBuffer;
        hSolvePane = CreateDialogParam(hInst,
            MAKEINTRESOURCE(IDD_DLGSOLVE),
            hWnd,
            (DLGPROC) SolvePaneProc,
            (LPARAM) &ctx);
        GetWindowRect(hSolvePane, &rc);
        nPaneHeight = rc.bottom - rc.top;
        SetWindowPos(hSolvePane, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
        // Создание строки состояния
        ctx.hStatusBar = CreateWindowEx(0L, STATUSCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hWnd, (HMENU) IDC_STATUSBAR, hInst, NULL);
        SetWindowText(ctx.hStatusBar, TEXT("Готово"));
        nStatusBarHeight = GetWindowHeight(ctx.hStatusBar);
        // Создание списка
		ctx.hListView = CreateWindowEx(0L, WC_LISTVIEW, TEXT(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |
			LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_EDITLABELS,
            0, nPaneHeight, rc.right - rc.left,
            rc.bottom - rc.top - nPaneHeight,
            hWnd, (HMENU)IDC_LISTVIEW, hInst, NULL);
        if (ctx.hListView == NULL)
            return FALSE;
		ListView_SetExtendedListViewStyle(ctx.hListView,
			ListView_GetExtendedListViewStyle(ctx.hListView) | LVS_EX_FULLROWSELECT);
        hImageListSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);
        hImageListLarge = ImageList_Create(GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON), ILC_MASK|ILC_COLOR32, 1, 1);
        // TODO: добавить иконки
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        ImageList_AddIcon(hImageListSmall, hIcon);
        ImageList_AddIcon(hImageListLarge, hIcon);
        ListView_SetImageList(ctx.hListView, hImageListSmall, LVSIL_SMALL);
        ListView_SetImageList(ctx.hListView, hImageListLarge, LVSIL_NORMAL);
        // Вставка столбцов
        lvc.mask = lvcMask;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = 150;
        _tcscpy_s(lpszBuffer, BUFFER_SIZE, TEXT("Номер листа"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 0;
        ListView_InsertColumn(ctx.hListView, 0, &lvc);
        _tcscpy_s(lpszBuffer, BUFFER_SIZE, TEXT("Лицевая сторона"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 1;
        ListView_InsertColumn(ctx.hListView, 1, &lvc);
        _tcscpy_s(lpszBuffer, BUFFER_SIZE, TEXT("Обратная сторона"));
        lvc.pszText = lpszBuffer;
        lvc.iSubItem = 2;
        ListView_InsertColumn(ctx.hListView, 2, &lvc);
        // Инициализация диалога сохранения файла
        ofn.hInstance = hInst;
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = (LPTSTR)calloc(nMaxFile,sizeof(TCHAR));
        _tcscpy_s(ofn.lpstrFile, nMaxFile, TEXT("\0"));
        ofn.nMaxFile = nMaxFile;
        ofn.lpstrFilter = TEXT("Все файлы\0*.*\0Текстовые файлы (TXT)\0*.txt\0Значения, разделенные запятыми (CSV)\0*.csv");
        ofn.nFilterIndex = 3;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrDefExt = TEXT("csv");
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
        SendMessage(hSolvePane, WM_COMMAND,
            (WPARAM)IDC_BTNSOLVE, (LPARAM)0L);
		break;
	  case WM_SIZE:
        SendMessage(ctx.hStatusBar, WM_SIZE, 0, 0);
        SetWindowPos(hSolvePane, HWND_TOP, 0, 0,
            LOWORD(lParam), nPaneHeight, SWP_NOMOVE);
		SetWindowPos(ctx.hListView, HWND_TOP, 0, 0, LOWORD(lParam),
            HIWORD(lParam)-nPaneHeight-nStatusBarHeight, SWP_NOMOVE);
		break;
	  case WM_COMMAND:
        switch (LOWORD(wParam)) {
          case IDM_ITEMQUIT:
            PostQuitMessage(0);
            return 0L;
            break;
          case IDM_ITEMABOUT:
            _stprintf(lpszBuffer, TEXT("%s, version %s\n\n%s\n\n%s"),
                TEXT("Pages5"), TEXT("2.0"),
                TEXT("Утилита для сортировки страниц перед печатью"),
                TEXT("Copyright (c) 2018 Lev Vorobjev"));
            MessageBox(hWnd, lpszBuffer, TEXT("О программе")
                MSG_TITLE, MB_OK | MB_ICONINFORMATION);
            break;
          case IDM_ITEMSAVE:
          {
            ofn.lpstrTitle = TEXT("Сохранить таблицу листов, как");
            if (GetSaveFileName(&ofn)) {
                FILE *f = NULL;
                _tfopen_s(&f, ofn.lpstrFile, TEXT("w"));
                _setmode(_fileno(f), _O_U8TEXT);
                _ftprintf(f, TEXT("\"Номер листа\",\"Лицевая сторона\",\"Обратная сторона\"\n"));
                for (int i=0; i < ctx.nSheets; i++) {
                    _ftprintf(f, TEXT("%d,\"%s\",\"%s\"\n"), ctx.items[i].nSheet,
                        ctx.items[i].lpszFace, ctx.items[i].lpszBack);
                }
                fclose(f);
            }
            break;
          }
		}
		break;
      case WM_NOTIFY:
      {
        LV_DISPINFO * lpLvdi = (LV_DISPINFO *)(LPNMHDR)lParam;
        solve_table_item_t* item = (solve_table_item_t*)(lpLvdi->item.lParam);
        if(wParam != IDC_LISTVIEW)
            return 0L;
        switch (lpLvdi->hdr.code) {
          case LVN_GETDISPINFO:
            if (lpLvdi->item.mask & LVIF_TEXT) {
                int i = lpLvdi->item.iSubItem;
                if (i == 0) {
                    _stprintf(lpszBuffer, TEXT("Лист %d"), item->nSheet);
                    lpLvdi->item.pszText = lpszBuffer;
                } else if (i == 1) {
                    lpLvdi->item.pszText = item -> lpszFace;
                } else {
                    lpLvdi->item.pszText = item -> lpszBack;
                }
            }
            break;
        }
        break;
      }
	  case WM_DESTROY:
        free(ctx.dwPages);
        free(ctx.lpszPages);
        free(ctx.items);
        free(ofn.lpstrFile);
        ImageList_Destroy(hImageListLarge);
        ImageList_Destroy(hImageListSmall);
        free(lpszBuffer);
		PostQuitMessage(0);
		break;
	  default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static
LPCTSTR SheetsStr(int count) {
    if (count == 0) {
        return TEXT("листов");
    } else if (count == 1) {
        return TEXT("лист");
    } else if (count >= 2 && count <= 4) {
        return TEXT("листа");
    } else if (count >= 5 && count <= 20) {
        return TEXT("листов");
    } else {
        return SheetsStr(count % 10);
    }
}

static
LPCTSTR EmptyPagesStr(int count) {
    if (count == 0) {
        return TEXT("пустых страниц");
    } else if (count == 1) {
        return TEXT("пустая страница");
    } else if (count >= 2 && count <= 4) {
        return TEXT("пустые страницы");
    } else if (count >= 5 && count <= 20) {
        return TEXT("пустых страниц");
    } else {
        return EmptyPagesStr(count % 10);
    }
}

BOOL CALLBACK SolvePaneProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static solve_pane_context_t *spCtx;
    static pages_context_t ctx = { sizeof(pages_context_t) };
    static HWND hBtnSolve;
    static HWND hEditFace;
    static HWND hEditBack;
    static HWND hLblInfo;
    static int btnLeftToRight;
    static int btnTop;
    static int edtHeight;
    static int edtClientSubWidth;
    static LPTSTR lpszBuffer;
    static LV_ITEM lvi = {0};
    switch (message) {
      case WM_INITDIALOG:
      {
        spCtx = (solve_pane_context_t*) lParam;
        lpszBuffer = spCtx -> lpszBuffer;
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
        RECT rc = {0};
        RECT rcClient = {0};
        POINT pt = {0};
        GetClientRect(hDlg, &rcClient);
        GetWindowRect(hBtnSolve, &rc);
        pt.x = rc.left;
        pt.y = rc.top;
        ScreenToClient(hDlg, &pt);
        btnLeftToRight = rcClient.right - pt.x;
        btnTop = pt.y;
        GetWindowRect(hEditBack, &rc);
        edtHeight = rc.bottom - rc.top;
        edtClientSubWidth = (rcClient.right-rcClient.left) - (rc.right-rc.left);
        break;
      }
      case WM_SIZE:
      {
        RECT rcClient = {0};
        GetClientRect(hDlg, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        SetWindowPos(hBtnSolve, HWND_TOP,
            rcClient.right - btnLeftToRight,
            btnTop, 0, 0, SWP_NOSIZE);
        SetWindowPos(hEditFace, HWND_TOP,
            0, 0, clientWidth - edtClientSubWidth,
            edtHeight, SWP_NOMOVE);
        SetWindowPos(hEditBack, HWND_TOP,
            0, 0, clientWidth - edtClientSubWidth,
            edtHeight, SWP_NOMOVE);
        return 0;
      }
      case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_BTNSOLVE:
            {
                if (GetPagesParams(&ctx)) {
                    div_t n = div(ctx.numPages, ctx.pagesPerSheet*2);
                    int numSheets = n.quot;
                    int emptyPages = 0;
                    if (n.rem != 0) {
                        numSheets ++;
                        emptyPages = numSheets*ctx.pagesPerSheet*2 - ctx.numPages;
                        _stprintf(lpszBuffer, TEXT("Готово %d %s, добавьте "
                            "%d %s после %d-й страницы."),
                             numSheets, SheetsStr(numSheets),
                             emptyPages, EmptyPagesStr(emptyPages), ctx.lastPage);
                        SetWindowText(spCtx->hStatusBar, lpszBuffer);
                    } else {
                        _stprintf(lpszBuffer, TEXT("Готово %d %s"),
                            numSheets, SheetsStr(numSheets));
                        SetWindowText(spCtx->hStatusBar, lpszBuffer);
                    }
                    const int pps = ctx.pagesPerSheet;
                    int numPages = numSheets * pps;
                    spCtx->nSheets = numSheets;
                    spCtx->nPagesPerSide = pps;
                    spCtx->dwPages = (uint32_t*)realloc(spCtx->dwPages, sizeof(uint32_t)*numPages*2);
                    spCtx->lpszPages = (LPTSTR)realloc(spCtx->lpszPages, sizeof(TCHAR)*numPages*8);
                    spCtx->items = (solve_table_item_t*)realloc(spCtx->items, sizeof(solve_table_item_t)*numSheets);
                    uint32_t* face = spCtx->dwPages;
                    uint32_t* back = spCtx->dwPages + numPages;
                    part_sheet_t* part = pages_init(ctx.firstPage, pps);
                    for (int i=0; i<numSheets; i++) {
                        pages_arrange(part, i, face + i*pps, back + i*pps);
                    }
                    _stprintf(lpszBuffer, TEXT("Печать страниц от %d-й "
                        "до %d-й в %s ориентации%s."),
                        ctx.firstPage, ctx.lastPage,
                        (pages_is_lscape(part)) ? TEXT("альбомной") : TEXT("портретной"),
                        (emptyPages != 0)? TEXT(" с пустыми страницами в конце") : TEXT(""));
                    pages_destroy(part);
                    SetWindowText(hLblInfo, lpszBuffer);
                    LPTSTR ptr = spCtx->lpszPages;
                    for (int i=0; i<numPages*2; i++) {
                        ptr += _stprintf(ptr, TEXT("%d,"), spCtx->dwPages[i]);
                    }
                    LPTSTR szFace = _tcstok_n(spCtx->lpszPages, _T(","), numPages);
                    SetWindowText(hEditFace, szFace);
                    LPTSTR szBack = _tcstok_n(NULL, _T(","), numPages);
                    SetWindowText(hEditBack, szBack);

                    ptr = _tcstok_n(szFace, _T(","), pps);
                    for (int i=0; i < numSheets; i++) {
                        spCtx -> items[i].nSheet = i+1;
                        spCtx -> items[i].lpszFace = ptr;
                        ptr = _tcstok_n(NULL, _T(","), pps);
                    }
                    ptr = _tcstok_n(szBack, _T(","), pps);
                    for (int i=0; i < numSheets; i++) {
                        spCtx -> items[i].lpszBack = ptr;
                        ptr = _tcstok_n(NULL, _T(","), pps);
                    }

                    ListView_DeleteAllItems(spCtx -> hListView);
                    ListView_SetItemCount(spCtx -> hListView, numSheets);
                    lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
                    lvi.pszText = LPSTR_TEXTCALLBACK;
                    for (int i=0; i < numSheets; i++) {
                        lvi.iItem = i;
                        lvi.cchTextMax = 80;
                        lvi.lParam = (LPARAM)&spCtx -> items[i];
                        lvi.iImage = 0;
                        for (int j=0; j<3; j++) {
                            lvi.iSubItem = j;
                            ListView_InsertItem(spCtx -> hListView, &lvi);
                        }
                    }
                }
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}
