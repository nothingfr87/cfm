#include "functions.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 512

int is_text_file(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    return -1;
  }

  unsigned char buffer[BUFFER_SIZE];
  size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
  fclose(file);

  for (size_t i = 0; i < bytes_read; i++) {
    if (buffer[i] == '\0') {
      return 0;
    }
    if (buffer[i] < 32 && buffer[i] != '\t' && buffer[i] != '\n' &&
        buffer[i] != '\r') {
      return 0;
    }
  }
  return 1;
}

void open_file(const char *filename) {
  char command[512];

#if defined(_WIN32) || defined(_WIN64)
  snprintf(command, sizeof(command), "start \"\" \"%s\" > nul 2>&1", filename);
  system(command);

#elif defined(__APPLE__)
  snprintf(command, sizeof(command), "open \"%s\" > /dev/null 2>&1", filename);
  system(command);

#elif defined(__linux__)
  if (is_text_file(filename)) {
    snprintf(command, sizeof(command), "nano \"%s\"", filename);
  } else {
    snprintf(command, sizeof(command), "xdg-open \"%s\" > /dev/null 2>&1",
             filename);
  }
  system(command);

#else
#error "Unsupported operating system"
#endif
}

int isDir(const char *path) {
  struct stat path_stat;

  if (stat(path, &path_stat) != 0) {
    fprintf(stderr, "File Does Not Exist.\n");
    return 1;
  }

  return S_ISDIR(path_stat.st_mode);
}

void freeitems(char **items, int count) {
  for (int i = 0; i < count; i++) {
    free(items[i]);
    items[i] = NULL;
  }
}

int loaddirectory(const char *path, char **items) {
  DIR *dr = opendir(path);
  if (dr == NULL) {
    perror("Error Occured");
    return -1;
  }
  struct dirent *in_file;
  int count = 0;

  while ((in_file = readdir(dr))) {
    struct stat buffer;

    if (in_file->d_name[0] == '.') {
      continue;
    }
    items[count++] = strdup(in_file->d_name);
  }

  closedir(dr);
  return count;
}

int goback(char **items, int selected, int *count) {
  if (chdir("..") == -1) {
    perror("Error Occured");
    return -1;
  }
  freeitems(items, *count);
  *count = loaddirectory(".", items);
  return 1;
}

int changedirectory(char **items, int selected, int *count) {
  if (!isDir(items[selected])) {
    return 0;
  }
  if (chdir(items[selected]) == -1) {
    perror("Error Occured");
    return -1;
  }
  freeitems(items, *count);
  *count = loaddirectory(".", items);
  return 1;
}
