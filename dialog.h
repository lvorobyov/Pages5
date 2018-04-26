/**
 * dialog.h
 *
 * Утилита для сортировки страниц брошуры
 * Copyright (c) 2018 Lev Vorobjev
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <windows.h>

typedef struct _pages_context_s {
    LONG lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    LPCTSTR lpszTitle;
    int numPages;
    int pagesPerSheet;
    int firstPage;
    int lastPage;
} pages_context_t;

BOOL GetPagesParams(pages_context_t* ctx);

#endif
