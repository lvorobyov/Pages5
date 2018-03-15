/**
 * @Author: Lev Vorobjev
 * @Date:   15.03.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: pages.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 15.03.2018
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include "pages.h"

static void pages_init_recursive(part_sheet_t* part, int first_page, int pps);
static void pages_destroy_recursive(part_sheet_t* part);

part_sheet_t* pages_init(int first_page, int pps) {
    part_sheet_t* part = (part_sheet_t*)malloc(sizeof(part_sheet_t));
    pages_init_recursive(part, first_page, pps);
    return part;
}

void pages_destroy(part_sheet_t* part) {
    pages_destroy_recursive(part);
    free(part);
}

static void pages_init_recursive(part_sheet_t* part, int first_page, int pps) {
    if (pps > 2) {
        part -> dwType = PART_TYPE_TWO_HALF;
        part -> parts = (part_sheet_t*)malloc(2*sizeof(part_sheet_t));
        int half = pps / 2;
        pages_init_recursive(&part -> parts[0], first_page, half);
        pages_init_recursive(&part -> parts[1], first_page + half, pps - half);
    } else {
        part -> dwType = (pps == 2) ? PART_TYPE_LEAF : PART_TYPE_SOME;
        part -> page = first_page;
    }
}

static void pages_destroy_recursive(part_sheet_t* part) {
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        pages_destroy_recursive(&part -> parts[0]);
        pages_destroy_recursive(&part -> parts[1]);
        free(part -> parts);
    }
}

static
bool pages_is_lscape(part_sheet_t* part) {
    switch (part -> dwType) {
        case PART_TYPE_SOME:
            return false;
        case PART_TYPE_LEAF:
            return true;
        case PART_TYPE_TWO_HALF:
            return ! pages_is_lscape(&part -> parts[1]);
    }
}

static
int pages_height(part_sheet_t* part, bool lscape) {
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        return (lscape?1:2) * pages_height(part -> parts[0], !lscape);
    } else {
        return 1;
    }
}

static
int pages_width(part_sheet_t* part, bool lscape) {
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        return (lscape?2:1) * pages_width(part -> parts[0], !lscape);
    } else {
        return 1;
    }
}

void pages_arrange(part_sheet_t* part, int* face, int* back) {
    // TODO: составить список страниц
}
