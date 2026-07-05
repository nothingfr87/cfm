#include "functions.h"
#include <dirent.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define STATUS_CORDS LINES - 2

void status_message(const char *message, ...) {
  va_list args;

  move(STATUS_CORDS, 0);
  clrtoeol();

  va_start(args, message);
  vw_printw(stdscr, message, args);
  va_end(args);

  refresh();
  napms(2000);

  move(STATUS_CORDS, 0);
  clrtoeol();
  refresh();
}

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
  FILE *fp = fopen(filename, "r");
  bool is_exist = false;
  if (fp != NULL) {
    is_exist = true;
    fclose(fp);
  }
  return is_exist;
}

void create_folder_file(char *items[], int selected, int *count) {
  FILE *temp_file = NULL;
  char filename[BUFFER_SIZE];

  mvprintw(STATUS_CORDS, 0, "Filename (end with '/' for folder): ");
  getnstr(filename, sizeof(filename));

  size_t len = strlen(filename);
  if (len > 0 && filename[len - 1] == '/') {
    struct stat st = {0};
    if (stat(filename, &st) == -1) {
      status_message("Created folder %s", filename);
      filename[len - 1] = '\0';
      mkdir(filename, 0700);
      freeitems(items, *count);
      *count = loaddirectory(".", items);
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }

    } else {
      status_message("Folder %s already exists", filename);
    }
  } else {
    if (file_exists(filename)) {
      status_message("File %s already exists", filename);
      if (temp_file)
        fclose(temp_file);
      return;
    }
    temp_file = fopen(filename, "a");

    if (temp_file) {
      status_message("Created %s", filename);
      fclose(temp_file);
      freeitems(items, *count);
      *count = loaddirectory(".", items);
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }
    }
  }
}

void delfile(char *items[], int selected, int *count, int ch) {
  status_message("Processed? [y/n]");
  ch = getch();

  if (ch == 'y') {
    if (remove(items[selected]) == 0) {
      status_message("Deleted %s", items[selected]);
      freeitems(items, *count);
      *count = loaddirectory(".", items);
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }
    } else {
      status_message("Failed to Delete %s", items[selected]);
    }
  } else {
    status_message("Abort");
  }
}

void open_file(const char *filename) {
  char command[BUFFER_SIZE];

#if defined(_WIN32) || defined(_WIN64)
  snprintf(command, BUFFER_SIZE, "start \"\" \"%s\" > nul 2>&1", filename);
  system(command);

#elif defined(__APPLE__)
  snprintf(command, BUFFER_SIZE, "open \"%s\" > /dev/null 2>&1", filename);
  system(command);

#elif defined(__linux__)
  if (is_text_file(filename)) {
    snprintf(command, BUFFER_SIZE, "nano \"%s\"", filename);
  } else {
    snprintf(command, BUFFER_SIZE, "xdg-open \"%s\" > /dev/null 2>&1",
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
