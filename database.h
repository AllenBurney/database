#ifndef DATABASE_H
#define DATABASE_H
#include<stdint.h>
#include<inttypes.h>
#define VERSION 1
#define MAX_STUDENTS 100		//Initially 
#define HEADER_SIZE 12
#define RECORD_SIZE 64
#define PAGE_SIZE 4096
#define MAGIC "MYDB"
#define LOAD_OK 0
#define LOAD_NEW 1
#define LOAD_ERR -1
#define VERSION_OFFSET 4
#define COUNT_OFFSET 8
#define ROWS_PER_PAGE (PAGE_SIZE/RECORD_SIZE)
#define MAX_PAGES (1+(MAX_STUDENTS+ROWS_PER_PAGE-1)/ROWS_PER_PAGE)
typedef struct{
	int32_t id;
	char name[52];
	int32_t age;
	float marks;
}Student;
_Static_assert(sizeof(Student)==RECORD_SIZE,"Student size mismatch");

//DataBase Functions
/*
void insert_student(void);
void select_student(void);
int save_database(void);
int load_database(void);
*/
#endif