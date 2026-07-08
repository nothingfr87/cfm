#include "ui.h"
#include "functions.h"
#include "helper_functions.h"
#include "tui_functions.h"
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#define DIR_COLOR 1
#define HEADER_HEIGHT 2
#define FOOTER_HEIGHT 1
#define CONTENT_START HEADER_HEIGHT
#define CONTENT_END (LINES - FOOTER_HEIGHT)

void create_ui(int ch, char *items[], int count, int selected) {
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

  while (1) {
    clear();
    for (int i = 0; i < count && CONTENT_START + i < CONTENT_END; i++) {
      int row = CONTENT_START + i;
      if (i == selected)
        attron(A_REVERSE);
      if (is_dir(items[i])) {
        attron(COLOR_PAIR(DIR_COLOR));
        mvprintw(row, 0, "%s/", items[i]);
        attroff(COLOR_PAIR(DIR_COLOR));
      } else {
        mvprintw(row, 0, "%s", items[i]);
      }
      if (i == selected)
        attroff(A_REVERSE);
    }

    topbar();
    file_details(items, selected);
    preview_file(items, selected);

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
      if (is_dir(items[selected])) {
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

    case 'r':
      rename_file(items, selected, &count);
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
