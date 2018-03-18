/**
 * @Author: Lev Vorobjev
 * @Date:   15.03.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: pages.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 17.03.2018
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
        part -> parts = (part_sheet_t*)calloc(2,sizeof(part_sheet_t));
        int half = pps / 2;
        pages_init_recursive(&part -> parts[0], first_page, half);
        pages_init_recursive(&part -> parts[1], first_page + half*2, pps - half);
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

int pages_count(part_sheet_t* part) {
    switch (part -> dwType) {
        case PART_TYPE_SOME:
            return 2;
        case PART_TYPE_LEAF:
            return 4;
        case PART_TYPE_TWO_HALF:
            return pages_count(&part -> parts[0]) +
                pages_count(&part -> parts[1]);
    }
}

static
int pages_height(part_sheet_t* part, bool lscape) {
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        return (lscape?1:2) * pages_height(&part -> parts[0], !lscape);
    } else {
        return 1;
    }
}

static
int pages_width(part_sheet_t* part, bool lscape) {
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        return (lscape?2:1) * pages_width(&part -> parts[0], !lscape);
    } else if (part -> dwType == PART_TYPE_LEAF) {
        return 2;
    } else {
        return 1;
    }
}

typedef struct _part_list_t {
    part_sheet_t* part;
    struct _part_list_t* next;
} part_list_t;

static
part_list_t* part_list_add(part_list_t* list, part_sheet_t* part) {
    if (list == NULL) {
		list = (part_list_t*)malloc(sizeof(part_list_t));
		list -> part = part;
		list -> next = NULL;
	} else {
		while (list->next != NULL)
			list = list -> next;
		list -> next = part_list_add(NULL, part);
	}
	return list;
}

static
part_list_t* part_list_append(part_list_t* list, part_list_t* othList) {
	if (list == NULL) {
		list = othList;
	} else {
		while (list->next != NULL)
			list = list -> next;
		list -> next = othList;
	}
	return list;
}

static
void part_list_free(part_list_t* list) {
	if (list -> next != NULL)
		part_list_free(list -> next);
	free (list);
}

static
part_list_t* pages_tree_to_list(part_sheet_t* part) {
    part_list_t* list = NULL;
    if (part -> dwType == PART_TYPE_TWO_HALF) {
        list = pages_tree_to_list(&part -> parts[0]);
        part_list_append(list,
            pages_tree_to_list(&part -> parts[1]));
    } else {
        list = part_list_add(NULL, part);
    }
    return list;
}

static
part_list_t* part_list_inverse(part_list_t* list, part_list_t* end) {
    if (list == end || list == NULL) {
        return list;
    } else if (list -> next == end || list -> next == end) {
        return list;
    } else {
        part_list_t* tmpList = part_list_inverse(list -> next, end);
        list -> next -> next = list;
        list -> next = end;
        return tmpList;
    }
}

void pages_arrange(part_sheet_t* part, int sheet, DWORD* face, DWORD* back) {
    int offset = pages_count(part) * sheet;
    if (part -> dwType == PART_TYPE_SOME) {
        face[0] = offset + part->page;
        back[0] = offset + part->page + 1;
        return;
    }
    bool lscape = pages_is_lscape(part);
    int height = pages_height(part, lscape);
    int width = pages_width(part, lscape);
    // Составить список страниц
    part_list_t* list = pages_tree_to_list(part);
    part_list_t* item = list;
    part_list_t *first, *last;
    for (int i=0; i<height; i++) {
        first = item;
        for (int j=0; j<width && item!=NULL; j+=2) {
            face[i*width + j] = offset + item -> part -> page + 1;
            face[i*width + j+1] = offset + item -> part -> page + 2;
            item = item -> next;
        }
        last = item;
        item = part_list_inverse(first, last);
        for (int j=0; j<width && item!=NULL; j+=2) {
            back[i*width + j] = offset + item -> part -> page + 3;
            back[i*width + j+1] = offset + item -> part -> page;
            item = item -> next;
        }
    }
    part_list_free(list);
}
