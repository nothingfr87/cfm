#include "ui.h"
#include "functions.h"
#include "helper_functions.h"
#include "tui_functions.h"
#include "image_preview.h"
#include <dirent.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR_COLOR 1
#define HEADER_HEIGHT 2
#define FOOTER_HEIGHT 1
#define CONTENT_START HEADER_HEIGHT
#define CONTENT_END (LINES - FOOTER_HEIGHT)
#define SIDEBAR_WIDTH COLS / 3

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void create_ui(int ch, char *items[], int count, int selected, int scroll) {
  setlocale(LC_ALL, "");
  initscr();
  echo();
  cbreak();
  curs_set(0);
  timeout(-1);
  keypad(stdscr, TRUE);

  int visible_rows = CONTENT_END - CONTENT_START;
  int max_name_width = SIDEBAR_WIDTH - 4;

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
    for (int i = 0; i < visible_rows && scroll + i < count; i++) {
      int index = scroll + i;
      int row = CONTENT_START + i;

      if (index == selected)
        attron(A_REVERSE);

      if (is_dir(items[index])) {
        attron(COLOR_PAIR(DIR_COLOR));
        mvprintw(row, 0, "%ls ", file_icons(items[index]));
        mvaddnstr(row, 2, items[index], max_name_width);
        mvaddch(row, 2 + MIN((int)strlen(items[index]), max_name_width), '/');
        attroff(COLOR_PAIR(DIR_COLOR));
      } else {
        mvprintw(row, 0, "%ls ", file_icons(items[index]));
        mvaddnstr(row, 2, items[index], max_name_width);
      }

      if (index == selected)
        attroff(A_REVERSE);
    }

    topbar();

    if (count > 0) {
      file_details(items, selected);
      preview_file(items, selected);
    } else {
      mvprintw(CONTENT_START, 0, "(empty directory)");
    }

    file_details(items, selected);
    preview_file(items, selected);

    refresh();

    ch = getch();
    switch (ch) {
    case KEY_UP:
    case 'k':
      if (selected > 0)
        selected--;
      if (selected < scroll)
        scroll--;
      break;

    case KEY_DOWN:
    case 'j':
      if (selected < count - 1)
        selected++;
      if (selected >= scroll + visible_rows)
        scroll++;
      break;

    case '\n':
    case KEY_RIGHT:
    case 'l':
      if (is_dir(items[selected])) {
        if (changedirectory(items, selected, &count)) {
          selected = 0;
          scroll = 0;
        }
      } else {
        open_file(items[selected]);
      }
      break;

    case KEY_LEFT:
    case 'h':
      if (goback(items, selected, &count))
        selected = 0;
      break;

    case 'g': {
      int next = getch();
      if (next == 'g') {
        selected = 0;
        scroll = 0;
      }
      break;
    }

    case 'G':
      selected = count - 1;
      scroll = count - visible_rows;
      if (scroll < 0)
        scroll = 0;
      break;

    case 'c':
    case 'y':
      clipboard_cut = 0;
      copy_to_clipboard(items, selected);
      break;

    case 'x':
      clipboard_cut = 1;
      copy_to_clipboard(items, selected);
      break;

    case 'p':
      paste_file(items, &count);
      break;

    case '/':
    case 's':
      search(items, &count, &selected, &scroll);
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
  clear_image_preview();
  endwin();
}
