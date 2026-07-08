#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <dirent.h>
#include <stdbool.h>

#define BUFFER_SIZE 512

void create_folder_file(char *items[], int selected, int *count);
void rename_file(char *items[], int selected, int *count);
void delfile(char *items[], int selected, int *count, int ch);
void open_file(const char *filename);
void preview_file(char *items[], int selected);
int goback(char **items, int selected, int *count);
int changedirectory(char *items[], int selected, int *count);

#endif
