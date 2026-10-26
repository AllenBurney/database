#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<errno.h>
#include "pager.h"
static int32_t read_page(FILE *fp,uint32_t page_num,void *buffer){
	long offset=(long)page_num*PAGE_SIZE;
	if(fseek(fp,offset,SEEK_SET)!=0)
		return 0;
	if(fread(buffer,1,PAGE_SIZE,fp)!=PAGE_SIZE)
		return 0;
	return 1;
}
static int32_t write_page(FILE *fp,uint32_t page_num,void *buffer){
	long offset=(long)page_num*PAGE_SIZE;
	if(fseek(fp,offset,SEEK_SET)!=0)
		return 0;
	if(fread(buffer,1,PAGE_SIZE,fp)!=PAGE_SIZE)
		return 0;
	if(fflush(fp)!=0)
		return 0;
	return 1;
}
Pager *pager_open(const char *filename){
	FILE *fp=fopen(filename,"rb+");
	if(fp==NULL){
		if(errno!=ENOENT){
			perror("Error loading database");
			return NULL;
		}
		fp=fopen(filename,"wb+");
		if(fp==NULL){
			perror("Error Creating Database");
			return NULL;
		}
	}
	if(fseek(fp,0,SEEK_END)!=0){
		perror("Error checking database file");
		fclose(fp);
		return NULL;
	}
	long file_length=ftell(fp);
	if(file_length<0){
		perror("Error checking database file");
		fclose(fp);
		return NULL;
	}
	if(file_length%PAGE_SIZE!=0 || file_length>(long)MAX_PAGES*PAGE_SIZE){
		printf("Invalid Database file size.\n");
		fclose(fp);
		return NULL;
	}
	Pager *pager=calloc(1,sizeof(Pager));
	if(pager==NULL){
		printf("Memory Allocation Failed.\n");
		fclose(fp);
		return NULL;
	}
	pager->fp=fp;
	pager->file_length=file_length;
	if(file_length==0){
		pager->num_pages=1;
		pager->pages[0]=calloc(1,PAGE_SIZE);
		if(pager->pages[0]==NULL){
			printf("Memory Allocation Failed.\n");
			fclose(fp);
			free(pager);
			return NULL;
		}
		pager->is_dirty[0]=1;
	}
	else{
		pager->num_pages=(uint32_t)(file_length/PAGE_SIZE);
		if(pager->num_pages==0 || pager->num_pages>MAX_PAGES){
			printf("Invalid Number of Pages.\n");
			fclose(fp);
			free(pager);
			return NULL;
		}
	}
	return pager;
}
void *get_page(Pager *pager,uint32_t page_num){
	if(pager==NULL||page_num>=MAX_PAGES)
		return NULL;
	if(page_num>pager->num_pages)
		return NULL;
	if(page_num==pager->num_pages){
		pager->pages[page_num]=calloc(1,PAGE_SIZE);
		if(pager->pages[page_num]==NULL){
			printf("Memory Allocation failed.\n");
			return NULL;
		}
		pager->is_dirty[page_num]=1;
		pager->num_pages++;
		return pager->pages[page_num];
	}
	//Cached
	if(pager->pages[page_num]!=NULL)
		return pager->pages[page_num];
	//Loading From disk
	pager->pages[page_num]=malloc(PAGE_SIZE);
	if(pager->pages[page_num]==NULL){
		printf("Memory Allocation failed.\n");
		return NULL;
	}
	if(!read_page(pager->fp,page_num,pager->pages[page_num])){
		printf("Error reading database page.\n");
		free(pager->pages[page_num]);
		pager->pages[page_num]=NULL;
		return NULL;
	}
	return pager->pages[page_num];
}
void pager_mark_dirty(Pager *pager,uint32_t page_num){
	if(pager!=NULL || page_num<pager->num_pages)
		pager->is_dirty[page_num]=1;
}
int32_t pager_flush(Pager *pager,uint32_t page_num){
	if(pager==NULL || page_num>=pager->num_pages)
		return 0;
	if(pager->pages[page_num]==NULL)
		return 1;
	if(!pager->is_dirty[page_num])
		return 1;
	if(!write_page(pager->fp,page_num,pager->pages[page_num])){
		printf("Error writing database page.\n");
		return 0;
	}
	pager->is_dirty[page_num]=0;
	return 1;
}
int32_t pager_class(Pager *pager){
	if(pager==NULL)
		return 0;
	int32_t success=1;
	for(uint32_t i=1;i<pager->num_pages;i++){
		if(!pager_flush(pager,i))
			success=0;
	}
	if(!pager_flush(pager,0))
		success=0;
	for(uint32_t i=0;i<MAX_PAGES;i++)
		free(pager->pages[i]);
	if(fclose(pager->fp)!=0){
		perror("Error closing database file");
		success=0;
	}
	free(pager);
	return success;
}