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
#include "pages.h"

#define IDC_LISTVIEW  40050

#define WND_TITLE TEXT("Сортировщик страниц")
#define WND_MENU_NAME MAKEINTRESOURCE(IDR_APPMENU)
#define MSG_TITLE TEXT("Pages5")
#define BUFFER_SIZE 4096

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
    int nPagesPerSide;
    DWORD* dwPages;
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

#if 0
    static HFONT hFont;
    static LOGFONT lf = {0};
#endif

	switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		lpszBuffer = (LPTSTR)calloc(BUFFER_SIZE,sizeof(TCHAR));
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
#if 0
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
        free(ctx.dwPages);
        //DeleteObject(hFont);
        free(lpszBuffer);
		PostQuitMessage(0);
		break;
	  default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#ifdef _UNICODE
  #define SCAS_TCHAR    "scasw\n\t"
  #define MOVS_TCHAR    "movsw\n\t"
  #define STOS_TCHAR    "stosw\n\t"
  #define LODS_TCHAR    "lodsw\n\t"
  #define CMP_AX_TCHAR  "cmp ax, word ptr [rsi]\n\t"
  #define MOV_TCHAR_AX  "mov word ptr [rsi], ax\n\t"
  #define MOV_TCHAR_Z   "mov word ptr [rsi-2], 0\n\t"
  #define TCHAR_PTR     "word ptr"
  #define INC_ESI_TCHAR "add rsi, 2\n\t"
  #define DEC_ESI_TCHAR "sub rsi, 2\n\t"
#else
  #define SCAS_TCHAR    "scasb\n\t"
  #define MOVS_TCHAR    "movsb\n\t"
  #define STOS_TCHAR    "stosb\n\t"
  #define LODS_TCHAR    "lodsb\n\t"
  #define CMP_AX_TCHAR  "cmp al, byte ptr [rsi]\n\t"
  #define MOV_TCHAR_AX  "mov byte ptr [rsi], al\n\t"
  #define MOV_TCHAR_Z   "mov byte ptr [rsi-1], 0\n\t"
  #define TCHAR_PTR     "byte ptr"
  #define INC_ESI_TCHAR "inc rsi\n\t"
  #define DEC_ESI_TCHAR "dec rsi\n\t"
#endif

#define ASM_BREAK       \
    "xor rcx,rcx\n\t"   \
    "jmp .exit2\n\t"

static
LPTSTR _tcstok_n(LPTSTR tcs, LPCTSTR delim, __int64 count) {
    static LPTSTR ptr = NULL;
    LPTSTR result;
    if (tcs == NULL) {
        if (ptr == NULL) {
            return NULL;
        } else {
            tcs = ptr;
        }
    }
    __asm__ (
        ".intel_syntax noprefix\n\t"
        "cld\n\t"
        "mov rdi, %[delim]\n\t"
        "mov rcx, -1\n\t"
        "xor ax, ax\n\t"
        "repne " SCAS_TCHAR
        "sub rcx, -1\n\t"
        "neg rcx\n\t"
        "mov r8, rcx\n\t"
        "xor rdx, rdx\n\t"
        // Пропуск ведущих разделителей
        ".for1:\n\t"
        LODS_TCHAR
        "mov rdi, %[delim]\n\t"
        "repne " SCAS_TCHAR
        "jne .end1\n\t"
        "jrcxz .exit\n\t"
        "mov rcx, r8\n\t"
        "jmp .for1\n\t"
        ".end1:\n\t"
        // Найден токен
        "test rdx, rdx\n\t"
        "jnz .end2\n\t"
        // Запомнить начало токена
        DEC_ESI_TCHAR
        "mov %0, rsi\n\t"
		INC_ESI_TCHAR
        "inc rdx\n\t"
        ".end2:\n\t"
        // Найти конец токена
        ".for2:\n\t"
        LODS_TCHAR
        "mov rcx, r8\n\t"
        "mov rdi, %[delim]\n\t"
        "repne " SCAS_TCHAR
        "jne .for2\n\t"
        "jrcxz .exit\n\t"
        // Достигнут конец токена
        "dec rbx\n\t"
        "test rbx, rbx\n\t"
        "jz .end3\n\t"
        "mov rcx, r8\n\t"
        "jmp .for1\n\t"
        ".end3:\n\t"
        // Найдено N токенов
        MOV_TCHAR_Z
        "mov %[ptr], rsi\n\t"
        "jmp .exit2\n\t"
        ".exit:\n\t"
        // Достигнут конец строки
        "mov %[ptr], 0\n\t"
        ".exit2:\n\t"
#if 0
        "sal rcx, 1\n\t"
        "add rsi, rcx\n\t"
        "mov %0,rsi\n\t"
#endif
        : "=m" (result), [ptr]"=m"(ptr)
        : "S" (tcs), [delim]"r" (delim), "b" (count)
        : "rax", "rcx", "rdx", "rdi", "r8", "cc", "memory"
    );
    return result;
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
                    if (n.rem != 0) {
                        numSheets ++;
                        _stprintf(lpszBuffer, TEXT("Перед печатью документа "
                            "необходимо после %d-й страницы вставить %d пустых страниц."),
                            ctx.lastPage, numSheets*ctx.pagesPerSheet*2 - ctx.numPages);
                        MessageBox(spCtx->hwndOwner, lpszBuffer,
                            MSG_TITLE, MB_OK | MB_ICONINFORMATION);
                    }
                    const int pps = ctx.pagesPerSheet;
                    int numPages = numSheets * pps;
                    spCtx->nSheets = numSheets;
                    spCtx->nPagesPerSide = pps;
                    spCtx->dwPages = (DWORD*)realloc(spCtx->dwPages, sizeof(DWORD)*numPages*2);
                    DWORD* face = spCtx->dwPages;
                    DWORD* back = spCtx->dwPages + numPages;
                    part_sheet_t* part = pages_init(ctx.firstPage, pps);
                    for (int i=0; i<numSheets; i++) {
                        pages_arrange(part, i, face + i*pps, back + i*pps);
                    }
                    _stprintf(lpszBuffer, TEXT("Печать страниц от %d "
                        "до %d в %s ориентации."), ctx.firstPage, ctx.lastPage,
                        (pages_is_lscape(part)) ? TEXT("альбомной") : TEXT("портретной"));
                    pages_destroy(part);
                    SetWindowText(hLblInfo, lpszBuffer);
                    LPTSTR ptr = lpszBuffer;
                    for (int i=0; i<numPages*2; i++) {
                        ptr += _stprintf(ptr, TEXT("%d,"), spCtx->dwPages[i]);
                    }
                    LPTSTR szFace = _tcstok_n(lpszBuffer, _T(","), numPages);
                    SetWindowText(hEditFace, szFace);
                    LPTSTR szBack = _tcstok_n(NULL, _T(","), numPages);
                    SetWindowText(hEditBack, szBack);
                    ListView_DeleteAllItems(spCtx -> hListView);
                    LV_ITEM lvi = {0};
                    lvi.mask = LVIF_TEXT;
                    lvi.pszText = _tcstok_n(szFace, _T(","), pps);
                    for (int i=0; i < numSheets; i++) {
                        lvi.iItem = i;
                        lvi.iSubItem = 1;
                        lvi.cchTextMax = 80;
                        ListView_InsertItem(spCtx -> hListView, &lvi);
                        lvi.pszText = _tcstok_n(NULL, _T(","), pps);
                    }
                    lvi.pszText = _tcstok_n(szBack, _T(","), pps);
                    for (int i=0; i < numSheets; i++) {
                        lvi.iItem = i;
                        lvi.iSubItem = 2;
                        lvi.cchTextMax = 80;
                        ListView_InsertItem(spCtx -> hListView, &lvi);
                        lvi.pszText = _tcstok_n(NULL, _T(","), pps);
                    }
                    lvi.pszText = lpszBuffer;
                    for (int i=0; i < numSheets; i++) {
                        lvi.iItem = i;
                        lvi.iSubItem = 0;
                        lvi.cchTextMax = 80;
                        _stprintf(lvi.pszText, TEXT("Лист %d"), i+1);
                        ListView_InsertItem(spCtx -> hListView, &lvi);
                    }
                }
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}
