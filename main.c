#include<stdio.h>
#include<stdint.h>
#include<inttypes.h>
#include<string.h>
#include "database.h"
#include "pager.h"
#define DB_FILE "database.dat"

static void flush_line(void){
	int c;
	while((c=getchar())!='\n' && c!=EOF);
}
static int32_t initialize_database(Pager *pager,int32_t *student_count){
	unsigned char *header=get_page(pager,0);
	if(header==NULL)
		return LOAD_ERR;
	
	if(pager->file_length==0){
		memset(header,0,PAGE_SIZE);
		int32_t version=VERSION;
		int32_t count=0;
		memcpy(header,MAGIC,4);
		memcpy(header+VERSION_OFFSET,&version,sizeof(version));
		memcpy(header+COUNT_OFFSET,&count,sizeof(count));
		pager_mark_dirty(pager,0);
		*student_count=0;
		printf("New Database Created.\n");
		return LOAD_NEW;
	}
	if(memcmp(header,MAGIC,4)!=0){
		printf("INVALID Database File.\n");
		return LOAD_ERR;
	}
	int32_t version;
	int32_t count;
	memcpy(&version,header+VERSION_OFFSET,sizeof(version));
	memcpy(&count,header+COUNT_OFFSET,sizeof(count));
	if(version!=VERSION){
		printf("Unsupported Database Version.\n");
		return LOAD_ERR;
	}
	if(count<0 || count>MAX_STUDENTS){
		printf("Invalid Student Count.\n");;
		return LOAD_ERR;
	}
	uint32_t required_pages=1+((uint32_t)count+ROWS_PER_PAGE-1)/ROWS_PER_PAGE;
	if(required_pages>pager->num_pages){
		printf("Truncated Database File.\n");
		return LOAD_ERR;
	}
	*student_count=count;
	printf("Database Located.\n");
	return LOAD_OK;
}
static void insert_student(Pager *pager,int32_t *student_count){
	if(*student_count>=MAX_STUDENTS){
		printf("Table is full.\n");
		return;
	}
	Student s={0};
	printf("Enter Id:");
	if(scanf("%" SCNd32,&s.id)!=1){
		printf("Invalid Id.\n");
		flush_line();
		return;
	}
	printf("Enter Name:");
	if(scanf("%51s",s.name)!=1){
		printf("Invalid Name.\n");
		flush_line();
		return;
	}
	printf("Enter Age:");
	if(scanf("%" SCNd32,&s.age)!=1){
		printf("Invalid Age.\n");
		flush_line();
		return;
	}
	printf("Enter Marks:");
	if(scanf("%f",&s.marks)!=1){
		printf("Invalid Marks.\n");
		flush_line();
		return;
	}
	flush_line();
	uint32_t page_num=1+(uint32_t)(*student_count)/ROWS_PER_PAGE;
	uint32_t row_num=(uint32_t)(*student_count)%ROWS_PER_PAGE;
	//Getting both pages before changing anything
	unsigned char *header=get_page(pager,0);
	if(header==NULL){
		printf("Error accessing database header.\n");
		return;
	}
	unsigned char *page=get_page(pager,page_num);
	if(page==NULL){
		printf("Error accessing database page.\n");
		return;
	}
	memcpy(page+(size_t)row_num*RECORD_SIZE,&s,sizeof(Student));
	pager_mark_dirty(pager,page_num);
	(*student_count)++;
	memcpy(header+COUNT_OFFSET,student_count,sizeof(*student_count));
	pager_mark_dirty(pager,0);
	printf("Student Inserted.\n");
}
static void select_student(Pager *pager,int32_t student_count){
	if(student_count==0){
		printf("Table Empty.\n");
		return;
	}
	for(int32_t i=0;i<student_count;i++){
		uint32_t page_num=1+(uint32_t)i/ROWS_PER_PAGE;
		uint32_t row_num=(uint32_t)i%ROWS_PER_PAGE;
		unsigned char *page=get_page(pager,page_num);
		if(page==NULL){
			printf("Error reading student record.\n");
			return;
		}
		Student s;
		memcpy(&s,page+(size_t)row_num*RECORD_SIZE,sizeof(Student));
		printf("ID:%"PRId32" |Name:%s |Age:%" PRId32" |Marks:%.2f\n",s.id,s.name,s.age,s.marks);
	}
}
int main(void){
	char command[20];
	Pager *pager=pager_open(DB_FILE);
	if(pager==NULL){
		printf("Refusing to start; fix or replace %s.\n",DB_FILE);
		return 1;
	}
	int32_t student_count=0;
	if(initialize_database(pager,&student_count)==LOAD_ERR){
		pager_close(pager);
		printf("Refusing to start;fix or replace %s.\n",DB_FILE);
		return 1;
	}
	while(1){
		printf("MiniDB> ");
		if(scanf("%19s",command)!=1)
			break;
		if(strcmp(command,"insert")==0){
			insert_student(pager,&student_count);
		}
		else if(strcmp(command,"select")==0){
			select_student(pager,student_count);
		}
		else if(strcmp(command,"exit")==0){
			break;
		}
		else{
			printf("Unknown Command.\n");
		}
	}
	if(!pager_close(pager)){
		printf("Error Saving or Closing database.\n");
		return 1;
	}
	printf("Database Closed.\n");
	return 0;
}