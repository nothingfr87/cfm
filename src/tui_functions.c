#include "tui_functions.h"
#include "functions.h"
#include "helper_functions.h"
#include <locale.h>
#include <ncurses.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>

#define STATUS_CORDS LINES - 2
#define TOP_BAR_ROW 0

static const char *exts[] = {"c", "h", "py", "txt", "md", "pdf", "zip", "tar"};
static const wchar_t *icons[] = {L"\ue61e", L"\ue61e", L"\ue73c", L"\uf15c",
                                 L"\uf15c", L"\uf1c1", L"\uf40b", L"\uf40b"};

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

const wchar_t *file_icons(const char *filename) {
  if (is_dir(filename)) {
    return L"\uf07b";
  }

  const char *ext = strchr(filename, '.');
  if (!ext)
    return L"\uf15b";

  ext++;

  for (int i = 0; i < sizeof(exts) / sizeof(exts[i]); i++) {
    if (strcmp(ext, exts[i]) == 0) {
      return icons[i];
    }
  }

  return L"\uf15b";
}

void permissions_to_string(mode_t mode, char *perm) {
  perm[0] = S_ISDIR(mode)    ? 'd'
            : S_ISLNK(mode)  ? 'l'
            : S_ISCHR(mode)  ? 'c'
            : S_ISBLK(mode)  ? 'b'
            : S_ISFIFO(mode) ? 'p'
            : S_ISSOCK(mode) ? 's'
                             : '-';

  perm[1] = (mode & S_IRUSR) ? 'r' : '-';
  perm[2] = (mode & S_IWUSR) ? 'w' : '-';
  perm[3] = (mode & S_IXUSR) ? 'x' : '-';

  perm[4] = (mode & S_IRGRP) ? 'r' : '-';
  perm[5] = (mode & S_IWGRP) ? 'w' : '-';
  perm[6] = (mode & S_IXGRP) ? 'x' : '-';

  perm[7] = (mode & S_IROTH) ? 'r' : '-';
  perm[8] = (mode & S_IWOTH) ? 'w' : '-';
  perm[9] = (mode & S_IXOTH) ? 'x' : '-';

  perm[10] = '\0';
}

void file_details(char *items[], int selected) {
  struct stat file_st;
  if (stat(items[selected], &file_st)) {
    clrtoeol();
    mvprintw(STATUS_CORDS, 0, "Could not get details for %s", items[selected]);
    refresh();
    return;
  }
  char size_str[32];
  char perms[11];
  human_size(file_st.st_size, size_str, sizeof(size_str));
  permissions_to_string(file_st.st_mode, perms);
  clrtoeol();
  mvprintw(STATUS_CORDS, 0, "%s %s %s", perms, size_str, items[selected]);
  refresh();
}

void topbar() {
  char cwd[BUFFER_SIZE];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    mvhline(TOP_BAR_ROW, 0, ' ', COLS);
    mvprintw(TOP_BAR_ROW, 0, "%s", cwd);
  }
}
