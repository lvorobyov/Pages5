/**
 * @Author: Lev Vorobjev
 * @Date:   16.03.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: dialog.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 17.03.2018
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <windowsx.h>
#include "dialog.h"
#include "resource.h"

#define MSG_TITLE TEXT("Pages5")

#define TRANSLATE_FAIL(idControl)   \
    MessageBox(hDlg, TEXT("Текстовое поле не заполнено"),   \
        MSG_TITLE, MB_OK | MB_ICONWARNING);                 \
    SetFocus(GetDlgItem(hDlg, idControl));                  \
    return TRUE;

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL GetPagesParams(pages_context_t* ctx) {
    return DialogBoxParam(ctx->hInstance,
        MAKEINTRESOURCE(IDD_PAGESDLG),
        ctx->hwndOwner,
        (DLGPROC) DlgProc,
        (LPARAM) ctx
    ) == IDOK;
}

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
    switch (message) {
        case WM_INITDIALOG:
            ctx = (pages_context_t*) lParam;
            if (ctx->lpszTitle != NULL) {
                SetWindowText(hDlg, ctx->lpszTitle);
            }
            SetDlgItemInt(hDlg, IDC_EDITNUM, ctx->numPages, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITPPS, ctx->pagesPerSheet, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITFST, ctx->firstPage, FALSE);
            SetDlgItemInt(hDlg, IDC_EDITLST, ctx->lastPage, FALSE);
            SetFocus(GetDlgItem(hDlg, IDC_EDITNUM));
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
#if 1
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
