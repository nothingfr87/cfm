#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <dirent.h>

void open_with_default_app(const char *filename);
int isDir(const char *path);
void changecwd(DIR **dr, char *items[], int selected);
void freeitems(char *items[], int count);
int loaddirectory(const char *path, char *items[]);
int goback(char **items, int selected, int *count);
int changedirectory(char *items[], int selected, int *count);

#endif
