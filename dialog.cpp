/**
 * dialog.cpp
 *
 * Утилита для сортировки страниц брошуры
 * Copyright (c) 2018 Lev Vorobjev
 */

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include "dialog.h"
#include "resource.h"

#define MSG_TITLE TEXT("Pages5")
#define EDT_AUTOFILL 1

#define TRANSLATE_FAIL(idControl)   \
    MessageBox(hDlg, TEXT("Текстовое поле не заполнено"),   \
        MSG_TITLE, MB_OK | MB_ICONWARNING);                 \
    SetFocus(GetDlgItem(hDlg, idControl));                  \
    return TRUE;

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditNumberWndProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

BOOL GetPagesParams(pages_context_t* ctx) {
    return DialogBoxParam(ctx->hInstance,
        MAKEINTRESOURCE(IDD_PAGESDLG),
        ctx->hwndOwner,
        (DLGPROC) DlgProc,
        (LPARAM) ctx
    ) == IDOK;
}

#define CH_SEP  '.'
#define CH_NEG  '-'
#define FL_NORMAL 0
#define FL_NOSEP  (DWORD)0x01 // Отключение ввода точки
#define FL_NONEG  (DWORD)0x02 // Отключение ввода минута

static
BOOL CheckPPS(int pps) {
    div_t n;
    while (pps > 1) {
        n = div(pps,2);
        pps = n.quot;
        if (n.rem != 0)
            return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static pages_context_t* ctx;
    int value;
    BOOL bTranslated;
    static DWORD dwEditFlags;
    switch (message) {
        case WM_INITDIALOG:
            ctx = (pages_context_t*) lParam;
            if (ctx->lpszTitle != NULL) {
                SetWindowText(hDlg, ctx->lpszTitle);
            }
            dwEditFlags = FL_NONEG | FL_NOSEP;
            SetWindowSubclass( GetDlgItem(hDlg, IDC_EDITNUM),
                EditNumberWndProc, 0, (DWORD_PTR) &dwEditFlags);
            SetWindowSubclass( GetDlgItem(hDlg, IDC_EDITPPS),
                EditNumberWndProc, 0, (DWORD_PTR) &dwEditFlags);
            SetWindowSubclass( GetDlgItem(hDlg, IDC_EDITFST),
                EditNumberWndProc, 0, (DWORD_PTR) &dwEditFlags);
            SetWindowSubclass( GetDlgItem(hDlg, IDC_EDITLST),
                EditNumberWndProc, 0, (DWORD_PTR) &dwEditFlags);
            SetDlgItemInt(hDlg, IDC_EDITNUM, ctx->numPages, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITPPS, ctx->pagesPerSheet, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITFST, ctx->firstPage, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITLST, ctx->lastPage, FALSE);
            SetFocus(GetDlgItem(hDlg, IDC_EDITNUM));
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
#if EDT_AUTOFILL
                case IDC_EDITNUM:
                    if (HIWORD(wParam) == EN_CHANGE) {
                        value = GetDlgItemInt(hDlg, IDC_EDITNUM,
                            &bTranslated, FALSE);
                        if (bTranslated) {
                            ctx->numPages = value;
                            value = ctx->firstPage + value - 1;
                            if (ctx->lastPage != value) {
                                ctx->lastPage = value;
                                SetDlgItemInt(hDlg, IDC_EDITLST, value, FALSE);
                            }
                        }
                    }
                    break;
                case IDC_EDITFST:
                    if (HIWORD(wParam) == EN_CHANGE) {
                        value = GetDlgItemInt(hDlg, IDC_EDITFST,
                            &bTranslated, FALSE);
                        if (bTranslated) {
                            ctx->firstPage = value;
                            value = value + ctx->numPages - 1;
                            if (ctx->lastPage != value) {
                                ctx->lastPage = value;
                                SetDlgItemInt(hDlg, IDC_EDITLST, value, FALSE);
                            }
                        }
                    }
                    break;
                case IDC_EDITLST:
                    if (HIWORD(wParam) == EN_CHANGE) {
                        value = GetDlgItemInt(hDlg, IDC_EDITLST,
                            &bTranslated, FALSE);
                        if (bTranslated) {
                            ctx->lastPage = value;
                            value = value - ctx->firstPage + 1;
                            if (ctx->numPages != value) {
                                ctx->numPages = value;
                                SetDlgItemInt(hDlg, IDC_EDITNUM, value, FALSE);
                            }
                        }
                    }
                    break;
#endif
                case IDOK:
                    value = GetDlgItemInt(hDlg, IDC_EDITNUM,
                        &bTranslated, FALSE);
                    if (!bTranslated) {
                        TRANSLATE_FAIL(IDC_EDITNUM);
                    }
                    ctx -> numPages = value;

                    value = GetDlgItemInt(hDlg, IDC_EDITPPS,
                        &bTranslated, FALSE);
                    if (!bTranslated) {
                        TRANSLATE_FAIL(IDC_EDITPPS);
                    }
                    if (! CheckPPS(value)) {
                        MessageBox(hDlg, TEXT("Число страниц на листе "
                            "должно быть степенью числа 2.\n"
                            "Допустимые значения: 1, 2, 4, 8, 16 и т.д."),
                            MSG_TITLE, MB_OK | MB_ICONWARNING);
                        SetFocus(GetDlgItem(hDlg, IDC_EDITPPS));
                        return TRUE;
                    }
                    ctx -> pagesPerSheet = value;

                    value = GetDlgItemInt(hDlg, IDC_EDITFST,
                        &bTranslated, FALSE);
                    if (!bTranslated) {
                        TRANSLATE_FAIL(IDC_EDITFST);
                    }
                    ctx -> firstPage = value;

                    value = GetDlgItemInt(hDlg, IDC_EDITLST,
                        &bTranslated, FALSE);
                    if (!bTranslated) {
                        TRANSLATE_FAIL(IDC_EDITLST);
                    }
                    ctx -> lastPage = value;

                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
            }
    }
    return FALSE;
}

LRESULT CALLBACK EditNumberWndProc(HWND hEdit, UINT message,
    WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData) {
    DWORD flags = *(DWORD*) dwRefData;
    switch (message) {
        case WM_CHAR:
            if(!((wParam >= '0' && wParam <= '9')
                || (wParam == CH_SEP && (flags & FL_NOSEP) == 0)
                || (wParam == CH_NEG && (flags & FL_NONEG) == 0)
                || wParam == VK_RETURN
                || wParam == VK_BACK))
            {
                return 0;
            }
            break;
    }
    return DefSubclassProc(hEdit, message, wParam, lParam);
}
