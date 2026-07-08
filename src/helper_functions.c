#include "helper_functions.h"
#include "functions.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

bool file_exists(const char *filename) {
  struct stat st;
  return stat(filename, &st) == 0;
}

int is_dir(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0)
    return 1;
  return S_ISDIR(path_stat.st_mode);
}

int mkdir_p(const char *path, mode_t mode) {
  char temp_path[BUFFER_SIZE];
  char *p;
  size_t len;
  snprintf(temp_path, sizeof(temp_path), "%s", path);
  len = strlen(temp_path);

  if (len > 0 && temp_path[len - 1] == '/')
    temp_path[len - 1] = '\0';

  for (p = temp_path + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      mkdir(temp_path, mode);
      *p = '/';
    }
  }
  return mkdir(temp_path, mode);
}

int rmdir_p(const char *path) {
  struct stat st;
  if (lstat(path, &st) != 0)
    return -1;

  if (!S_ISDIR(st.st_mode))
    return remove(path);

  DIR *dr = opendir(path);
  if (!dr)
    return -1;

  struct dirent *entry;
  char full_path[BUFFER_SIZE];

  while ((entry = readdir(dr)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
    rmdir_p(full_path);
  }

  closedir(dr);
  return remove(path);
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
