#ifndef PAGER_H
#define PAGER_H
#include<stdint.h>
#include<sys/types.h>
#include<stdio.h>
#include "database.h"
typedef struct {
	FILE *fp;
	long file_length;
	uint32_t num_pages;
	void *pages[MAX_PAGES];
	int32_t is_dirty[MAX_PAGES];
}Pager;

Pager *pager_open(const char *filename);
void *get_page(Pager *pager,uint32_t page_name);
void pager_mark_dirty(Pager *pager,uint32_t page_num);
int32_t pager_flush(Pager *pager,uint32_t page_num);
int32_t pager_close(Pager *pager);

#endif