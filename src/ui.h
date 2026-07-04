#ifndef UI_H
#define UI_H
#include <dirent.h>

void open_file(const char *filename);
void create_ui(DIR **dr, int ch, char *items[], int count, int selected);

#endif
