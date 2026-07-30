#include "functions.h"
#include "helper_functions.h"
#include "tui_functions.h"
#include "image_preview.h"
#include "syntax.h"
#include <dirent.h>
#include <linux/limits.h>
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

char clipboard[PATH_MAX];
int clipboard_cut = 0;

void create_folder_file(char *items[], int selected, int *count) {
  FILE *temp_file = NULL;
  char filename[BUFFER_SIZE];

  mvhline(STATUS_CORDS, 0, ' ', COLS);
  mvprintw(STATUS_CORDS, 0, "Filename (end with '/' for folder): ");
  move(STATUS_CORDS, strlen("Filename (end with '/' for folder): "));
  refresh();

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

  mvhline(STATUS_CORDS, 0, ' ', COLS);
  mvprintw(STATUS_CORDS, 0, "New Name: ");
  move(STATUS_CORDS, strlen("New Name: "));
  refresh();

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
  mvhline(STATUS_CORDS, 0, ' ', COLS);
  mvprintw(STATUS_CORDS, 0, "Delete '%s'? [y/N] ", items[selected]);
  refresh();
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
  if (!items || !items[selected]) {
    clear_image_preview();
    return;
  }
  if (!items || selected < 0 || !items[selected])
    return;

  char buffer[BUFFER_SIZE];
  char command[BUFFER_SIZE];
  int y_level = 3;
  int x_level = SIDEBAR_WIDTH + PREVIEW_MARGIN;

  start_color();
  use_default_colors();
  init_pair(FOLDER_FILES_COLOR, COLOR_GREEN, -1);

  mvvline(0, SIDEBAR_WIDTH, ACS_VLINE, LINES);

  int preview_width = COLS - x_level - 10;
  int preview_height = LINES - y_level - 2;

  mvprintw(1, x_level, "Previewing File: %s", items[selected]);
  mvhline(2, SIDEBAR_WIDTH, ACS_HLINE, COLS);

  if (is_image_file(items[selected])) {
    for (int i = 0; i < preview_height; i++) {
      mvhline(y_level + i, x_level, ' ', COLS - x_level);
    }
    refresh();
    draw_image_preview(items[selected], x_level, y_level, preview_width, preview_height);
    return;
  } else {
    clear_image_preview();
  }

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

  FILE *fp = fopen(items[selected], "r");
  if (!fp) {
    mvprintw(y_level++, x_level, "Error: Cannot open file");
    return;
  }

  LangType lang = get_lang_type(items[selected]);
  init_syntax_colors();

  int in_multiline_comment = 0;
  while (fgets(buffer, sizeof(buffer), fp) && y_level < LINES - 2) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    
    if (lang != LANG_NONE) {
      print_highlighted_line(y_level++, x_level, buffer, preview_width, &in_multiline_comment, lang);
    } else {
      mvaddnstr(y_level++, x_level, buffer, preview_width);
    }
  }

  fclose(fp);
  refresh();
}

void copy_to_clipboard(char **items, int selected) {
  char cwd[PATH_MAX];

  getcwd(cwd, sizeof(cwd));
  snprintf(clipboard, sizeof(clipboard), "%s/%s", cwd, items[selected]);

  status_message("Copied %s", items[selected]);
}

void paste_file(char **items, int *count) {
  if (clipboard[0] == '\0') {
    status_message("Clipboard is empty");
    return;
  }

  char cwd[PATH_MAX];
  char dest[PATH_MAX];

  getcwd(cwd, sizeof(cwd));

  char *filename = strrchr(clipboard, '/');
  filename = filename ? filename + 1 : clipboard;

  snprintf(dest, sizeof(dest), "%s/%s", cwd, filename);
  if (strcmp(clipboard, dest) == 0) {
    status_message("File Already Exists");
    return;
  }

  struct stat st;
  if (stat(clipboard, &st) == -1) {
    status_message("Failed to access clipboard");
    return;
  }

  int ret;
  if (S_ISDIR(st.st_mode)) {
    if (clipboard_cut)
      ret = move_dir(clipboard, dest);
    else
      ret = copy_dir(clipboard, dest);
  } else {
    if (clipboard_cut)
      ret = move_file(clipboard, dest);
    else
      ret = copy_file(clipboard, dest);
  }

  if (ret != 0) {
    status_message("Paste failed");
    return;
  }
  freeitems(items, *count);
  *count = loaddirectory(".", items);
  if (clipboard_cut) {
    clipboard[0] = '\0';
    clipboard_cut = 0;
  }
  status_message("Paste Done");
}

void search(char *items[], int *count, int *selected, int *scroll) {
  def_prog_mode();
  endwin();

  const char *cmd = "if command -v fd >/dev/null 2>&1; "
                    "then fd; "
                    "else fdfind; "
                    "fi | fzf";

  FILE *fp = popen(cmd, "r");
  if (!fp) {
    reset_prog_mode();
    refresh();
    return;
  }

  char choice[PATH_MAX];
  char *res = fgets(choice, sizeof(choice), fp);
  pclose(fp);

  reset_prog_mode();
  refresh();
  clear();

  if (!res) {
    return;
  }

  choice[strcspn(choice, "\n")] = '\0';

  char dir[PATH_MAX];
  strcpy(dir, choice);

  char *filename = strrchr(dir, '/');

  if (filename) {
    *filename = '\0';
    filename++;

    if (chdir(dir) == -1)
      return;
  } else {
    filename = dir;
  }

  freeitems(items, *count);
  *count = loaddirectory(".", items);
  for (int i = 0; i < *count; i++) {
    if (strcmp(items[i], filename) == 0) {
      *selected = i;
      *scroll = *selected;
      if (*scroll < 0)
        *scroll = 0;
      break;
    }
  }
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
