/*
 * pages.h
 *
 * Утилита для сортировки страниц брошуры
 * Copyright (c) 2018 Lev Vorobjev
 */

#ifndef PAGES_H_
#define PAGES_H_

#include <windef.h>

#define PART_TYPE_SOME     0
#define PART_TYPE_LEAF     1
#define PART_TYPE_TWO_HALF 2

typedef struct _part_sheet_t {
    DWORD dwType;
    union {
        struct _part_sheet_t* parts;
        int page;
    };
} part_sheet_t;

part_sheet_t* pages_init(int first_page, int pps);
void pages_destroy(part_sheet_t* part);

bool pages_is_lscape(part_sheet_t* part);
int pages_count(part_sheet_t* part);
void pages_arrange(part_sheet_t* part, int* face, int* back);

#endif
