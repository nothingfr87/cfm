#include "functions.h"
#include "helper_functions.h"
#include "tui_functions.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STATUS_CORDS LINES - 2

void create_folder_file(char *items[], int selected, int *count) {
  FILE *temp_file = NULL;
  char filename[BUFFER_SIZE];

  mvprintw(STATUS_CORDS, 0, "Filename (end with '/' for folder): ");
  getnstr(filename, sizeof(filename));

  size_t len = strlen(filename);

  if (len > 0 && filename[len - 1] == '/') {
    filename[len - 1] = '\0';
    if (mkdir_p(filename, 0700) == 0) {
      status_message("Created folder %s", filename);
      freeitems(items, *count);
      *count = loaddirectory(".", items);
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }
    } else {
      status_message("Failed to create folder %s", filename);
    }
  } else {
    if (file_exists(filename)) {
      status_message("File %s already exists", filename);
      if (temp_file)
        fclose(temp_file);
      return;
    }
    temp_file = fopen(filename, "w");

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

void rename_file(char *items[], int selected, int *count) {
  char filename[256];
  mvprintw(LINES - 2, 0, "New Name: ");
  getnstr(filename, sizeof(filename));
  if (rename(items[selected], filename) == 0) {
    status_message("Renamed %s to %s", items[selected], filename);
    freeitems(items, *count);
    *count = loaddirectory(".", items);
    if (selected >= *count) {
      selected = *count - 1;
    }
    if (selected < 0) {
      selected = 0;
    }
  } else {
    status_message("Failed to rename %s", items[selected]);
  }
}

void delfile(char *items[], int selected, int *count, int ch) {
  mvprintw(STATUS_CORDS, 0, "Processed? [y/n]");
  ch = getch();

  if (ch == 'y') {
    if (rmdir_p(items[selected]) == 0) {
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

  if (is_text_file(filename)) {
#if defined(_WIN32) || defined(_WIN64)
    snprintf(command, BUFFER_SIZE, "notepad \"%s\"", filename);

#elif defined(__APPLE__)
    snprintf(command, BUFFER_SIZE, "open -a TextEdit \"%s\"", filename);

#elif defined(__linux__)
    snprintf(command, BUFFER_SIZE, "nano \"%s\"", filename);

#else
#error "Unsupported operating system"
#endif
  } else {
#if defined(_WIN32) || defined(_WIN64)
    snprintf(command, BUFFER_SIZE, "start \"\" \"%s\" > nul 2>&1", filename);

#elif defined(__APPLE__)
    snprintf(command, BUFFER_SIZE, "open \"%s\" > /dev/null 2>&1", filename);

#elif defined(__linux__)
    snprintf(command, BUFFER_SIZE, "xdg-open \"%s\" > /dev/null 2>&1",
             filename);

#else
#error "Unsupported operating system"
#endif
  }

  system(command);
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
  if (!is_dir(items[selected])) {
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
