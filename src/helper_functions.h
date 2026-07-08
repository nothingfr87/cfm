#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H
#include <stdbool.h>
#include <sys/types.h>

int is_text_file(const char *filename);
bool file_exists(const char *filename);
int is_dir(const char *path);
int mkdir_p(const char *path, mode_t mode);
int rmdir_p(const char *path);
void freeitems(char *items[], int count);
int loaddirectory(const char *path, char *items[]);

#endif
