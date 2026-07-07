#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <dirent.h>
#include <stdbool.h>

int is_text_file(const char *filename);
bool file_exists(const char *filename);
void create_folder_file(char *items[], int selected, int *count);
void rename_file(char *items[], int selected, int *count);
void delfile(char *items[], int selected, int *count, int ch);
void open_file(const char *filename);
int isDir(const char *path);
void freeitems(char *items[], int count);
int loaddirectory(const char *path, char *items[]);
int goback(char **items, int selected, int *count);
int changedirectory(char *items[], int selected, int *count);

#endif
