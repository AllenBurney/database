@echo off
gcc -std=c11 -Wall -Wextra main.c pager.c -o minidb.exe && minidb.exe