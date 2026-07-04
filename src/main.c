#include "ui.h"
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_ITEMS 1024

int main(int argc, char *argv[]) {
  char *items[MAX_ITEMS];
  int count = 0;
  int selected = 0;

  DIR *dr = opendir(".");
  struct dirent *in_file;

  if (dr == NULL) {
    fprintf(stderr, "Error: Failed to open directory - %s\n", strerror(errno));
    endwin();
    return 1;
  }

  while ((in_file = readdir(dr)) && count < MAX_ITEMS) {
    struct stat buffer;
    int status;
    status = stat(in_file->d_name, &buffer);
    if (status == -1) {
      fprintf(stderr, "Error: Failed to stat item - %s\n", strerror(errno));
      endwin();
      return 1;
    }

    if (in_file->d_name[0] == '.') {
      continue;
    }
    items[count++] = strdup(in_file->d_name);
  }

  closedir(dr);

  int ch;

  create_ui(&dr, ch, items, count, selected);

  return 0;
}
