#include "functions.h"
#include "helper_functions.h"
#include "tui_functions.h"
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define STATUS_CORDS LINES - 2
#define SIDEBAR_WIDTH COLS / 3
#define PREVIEW_MARGIN 4
#define FOLDER_FILES_COLOR 2

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
  char *editor = getenv("EDITOR");
  if (!editor || !*editor)
    editor = "nano";

  if (is_text_file(filename)) {
    def_prog_mode();
    endwin();

    pid_t pid = fork();

    if (pid == 0) {
      execlp(editor, editor, filename, (char *)NULL);
      perror("execlp");
      _exit(EXIT_FAILURE);
    } else if (pid > 0) {
      waitpid(pid, NULL, 0);

      reset_prog_mode();
      refresh();
      clear();
    } else {
      perror("fork");
    }

  } else {
    pid_t pid = fork();
    if (pid == 0) {
      execlp("xdg-open", "xdg-open", filename, (char *)NULL);
      perror("xdg-open");
      _exit(EXIT_FAILURE);
    }
  }
}

void preview_file(char *items[], int selected) {
  char buffer[BUFFER_SIZE];
  char command[BUFFER_SIZE];
  int y_level = 3;
  int x_level = SIDEBAR_WIDTH + PREVIEW_MARGIN;

  start_color();
  use_default_colors();
  init_pair(FOLDER_FILES_COLOR, COLOR_GREEN, -1);

  mvvline(0, SIDEBAR_WIDTH, ACS_VLINE, LINES);

  int preview_width = COLS - x_level - 10;

  mvprintw(1, x_level, "Previewing File: %s", items[selected]);
  mvhline(2, SIDEBAR_WIDTH, ACS_HLINE, COLS);

  DIR *dir = opendir(items[selected]);

  if (is_dir(items[selected])) {
    DIR *dir = opendir(items[selected]);
    if (!dir) {
      mvprintw(y_level++, x_level, "Cannot open directory");
      return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) {
      if (entry->d_name[0] == '.')
        continue;
      attron(COLOR_PAIR(FOLDER_FILES_COLOR));
      mvprintw(y_level++, x_level, "%s", entry->d_name);
      if (y_level >= LINES - 2)
        break;
    }
    closedir(dir);
    attroff(COLOR_PAIR(FOLDER_FILES_COLOR));
    return;
  }

  switch (is_text_file(items[selected])) {
  case 2:
    mvprintw(y_level++, x_level, "No Preview: Binary File");
    return;
    break;
  case 0:
    mvprintw(y_level++, x_level, "No Preview: Media File (Audio/Video/Image)");
    return;
    break;
  }

  snprintf(command, sizeof(command), "cat \"%s\" 2>/dev/null", items[selected]);
  FILE *tmp = popen(command, "r");

  while (fgets(buffer, sizeof(buffer), tmp)) {
    mvaddnstr(y_level++, x_level, buffer, preview_width);
  }

  pclose(tmp);
  refresh();
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
