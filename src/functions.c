#include "functions.h"
#include "tui_functions.h"
#include <dirent.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Define Constants
#define BUFFER_SIZE 512
#define STATUS_CORDS LINES - 2

// Helper function to check whether a file is a text, a binary, or an image file
int is_text_file(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    return -1;
  }

  unsigned char buffer[BUFFER_SIZE]; // Defining buffer for the *file to read to
  size_t bytes_read =
      fread(buffer, 1, sizeof(buffer), file); // Reads the file content
  fclose(file);

  for (size_t i = 0; i < bytes_read; i++) {
    if (buffer[i] == '\0') { // check whether there is a NUL (0x00), if yes file
                             // is likely a binary
      return 0;
    }
    if (buffer[i] < 32 && buffer[i] != '\t' && buffer[i] != '\n' &&
        buffer[i] != '\r') { // check for non-printable ascii control characters
      return 0;
    }
  }
  return 1;
}

// Helper function to check whether a file already exists or no
bool file_exists(const char *filename) {
  FILE *fp = fopen(filename, "r");
  bool is_exist = false;
  if (fp != NULL) {
    is_exist = true;
    fclose(fp);
  }
  return is_exist;
}

// Rename file/folder
void rename_file(char *items[], int selected, int *count) {
  char filename[256];
  mvprintw(LINES - 2, 0, "New Name: ");
  getnstr(filename, sizeof(filename)); // ask the user for the new name for the
                                       // selected file/folder
  if (rename(items[selected],
             filename)) { // check if the rename function was done properly
    status_message("Renamed %s to %s", items[selected],
                   filename); // show success message
    freeitems(items, *count); // free old items
    *count =
        loaddirectory(".", items); // load new direcotry and allocate new items
    if (selected >= *count) {
      selected = *count - 1;
    }
    if (selected < 0) {
      selected = 0;
    }
  } else {
    status_message("Failed to rename %s", items[selected]); // show fail message
  }
}

static int rmrf(const char *path) {
  struct stat st;
  if (lstat(path, &st) != 0) return -1;
  if (!S_ISDIR(st.st_mode)) return remove(path);
  DIR *d = opendir(path);
  if (!d) return -1;
  struct dirent *entry;
  char fullpath[BUFFER_SIZE];
  while ((entry = readdir(d)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)continue;
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
    rmrf(fullpath);
  }
  closedir(d);
  return remove(path);
}

// Deleting file/folder
void delfile(char *items[], int selected, int *count, int ch) {
  mvprintw(STATUS_CORDS, 0, "Processed? [y/n]");
  ch = getch();
  if (ch == 'y') {
    if (rmrf(items[selected]) == 0) {
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

// Open a file
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

// Helper function to check whether a file is a folder
int isDir(const char *path) {
  struct stat path_stat;

  if (stat(path, &path_stat) != 0) {
    fprintf(stderr, "File Does Not Exist.\n");
    return 1;
  }

  return S_ISDIR(path_stat.st_mode);
}

// Helper function to free old items
void freeitems(char **items, int count) {
  for (int i = 0; i < count; i++) {
    free(items[i]);
    items[i] = NULL;
  }
}

// Helper function to load the new direcotry and allocate new items
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

// Go back after entering a folder
int goback(char **items, int selected, int *count) {
  if (chdir("..") == -1) {
    perror("Error Occured");
    return -1;
  }
  freeitems(items, *count); // free old items
  *count =
      loaddirectory(".", items); // load new direcotry and allocate new items
  return 1;
}

// Helper function to change direcotries
int changedirectory(char **items, int selected, int *count) {
  if (!isDir(items[selected])) { // if the selected item isn't a folder return 0
                                 // (false)
    return 0;
  }
  if (chdir(items[selected]) == -1) { // if changing direcotry failed
    perror("Error Occured");          // show fail message
    return -1;
  }
  freeitems(items, *count); // free old items
  *count =
      loaddirectory(".", items); // load new direcotry and allocate new items
  return 1;
}
