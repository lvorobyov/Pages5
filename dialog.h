/**
 * @Author: Lev Vorobjev
 * @Date:   16.03.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: dialog.h
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 17.03.2018
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <windef.h>

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
