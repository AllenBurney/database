#include<stdio.h>
#include<string.h>
#include "database.h"
#define DB_FILE "database.dat"
#define TMP_FILE "database.tmp"

Student table[MAX_STUDENTS];
int32_t student_count=0;

int save_database(){
	if(student_count<0 || student_count>MAX_STUDENTS){
		printf("Invalid student count.\n");
		return 0;
	}
	FILE *fp=fopen(TMP_FILE,"wb");
	if(fp==NULL){
		printf("Error While Opening File.\n");
		return 0;
	}
	char magic[4]=MAGIC;
	int32_t version=VERSION;
	int32_t count=student_count;
	if(fwrite(magic,sizeof(char),4,fp)!=4||
		fwrite(&version,sizeof(int32_t),1,fp)!=1||
		fwrite(&count,sizeof(int32_t),1,fp)!=1||
		fwrite(table,sizeof(Student),count,fp)!=(size_t)count||
		fflush(fp)!=0){
		printf("Error While Writing File.\n");
		fclose(fp);
		remove(TMP_FILE);
		return 0;
	}
	if(fclose(fp)!=0){
		printf("Error while closing file\n");
		remove(TMP_FILE);
		return 0;
	}
	#ifdef _WIN32
		remove(DB_FILE);      //rename() doesnt work if target file exists
	#endif
	if(rename(TMP_FILE,DB_FILE)!=0){
		printf("Error while renaming file.\n");
		return 0;
	}
	printf("Database Saved.\n");
	return 1;
}

int load_database(){
	FILE *fp=fopen(DB_FILE,"rb");
	if(fp==NULL){
		return LOAD_NEW;
	}
	char magic[4];
	int32_t version;
	int32_t count;
	if(fread(magic,sizeof(char),4,fp)!=4){
		printf("Invalid Database File.\n");
		fclose(fp);
		return LOAD_ERR;
	}
	if(magic[0]!='M'||magic[1]!='Y'||magic[2]!='D'||magic[3]!='B'){
		printf("Invalid Database File.\n");
		fclose(fp);
		return LOAD_ERR;
	}
	if(fread(&version,sizeof(int32_t),1,fp)!=1||version!=VERSION){
		printf("Unsupported Database Version.\n");
		fclose(fp);
		return LOAD_ERR;
	}
	if(fread(&count,sizeof(int32_t),1,fp)!=1 || count<0 || count>MAX_STUDENTS){
		printf("Invalid Student Count.\n");
		fclose(fp);
		return LOAD_ERR;
	}
	if(fread(table,sizeof(Student),count,fp)!=(size_t)count){
		printf("Truncated Database File.\n");
		fclose(fp);
		return LOAD_ERR;
	}
	student_count=count;
	fclose(fp);
	printf("Database Loaded.\n");
	return LOAD_OK;
}

void insert_student(){
	return;
}
void select_student(){
	return;
}


int main(){
	char command[20];
	if(load_database()==LOAD_ERR){
		printf("Refusing to start;fix or remove %s.\n",DB_FILE);
		return 1;
	}
	while(1){
		printf("MiniDB> ");
		if(scanf("%19s",command)!=1)
			break;
		if(strcmp(command,"insert")==0){
			insert_student();
		}
		else if(strcmp(command,"select")==0){
			select_student();
		}
		else if(strcmp(command,"exit")==0){
			if(save_database())
				break;
			printf("Save Failed, Not Exiting.\n");
		}
		else{
			printf("Unknown Command.\n");
		}
	}
	return 0;
}