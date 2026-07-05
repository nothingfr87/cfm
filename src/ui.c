#include "ui.h"
#include "functions.h"
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIR_COLOR 1

void create_ui(DIR **dr, int ch, char *items[], int count, int selected) {
  initscr();
  echo();
  cbreak();
  curs_set(0);
  timeout(-1);
  keypad(stdscr, TRUE);

  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  start_color();
  use_default_colors();
  init_pair(DIR_COLOR, COLOR_BLUE, -1);

  char status_bar[256];

  while (1) {
    clear();
    for (int i = 0; i < count; i++) {
      if (i == selected) {
        attron(A_REVERSE);
      }
      if (isDir(items[i])) {
        attron(COLOR_PAIR(DIR_COLOR));
        mvprintw(i, 0, "%s/", items[i]);
        attroff(COLOR_PAIR(1));
      } else {
        mvprintw(i, 0, "%s", items[i]);
      }
      if (i == selected) {
        attroff(A_REVERSE);
      }
    }
    refresh();

    ch = getch();
    switch (ch) {
    case KEY_UP:
      if (selected > 0)
        selected--;
      break;

    case KEY_DOWN:
      if (selected < count - 1)
        selected++;
      break;

    case '\n':
    case KEY_RIGHT:
      if (isDir(items[selected])) {
        if (changedirectory(items, selected, &count))
          selected = 0;
      } else {
        open_file(items[selected]);
      }
      break;

    case KEY_LEFT:
      if (goback(items, selected, &count))
        selected = 0;
      break;

    case 'a':
      create_folder_file(items, selected, &count);
      break;

    case 'd':
      delfile(items, selected, &count, ch);
      break;

    case 'q':
    case 27:
      goto end;
    }
  }

end:
  endwin();
}
