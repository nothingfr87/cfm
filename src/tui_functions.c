#include "tui_functions.h"
#include "functions.h"
#include <ncurses.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Define Constants
#define BUFFER_SIZE 512
#define STATUS_CORDS LINES - 2
#define TOP_BAR_ROW 0

// Helper function to format raw size from stat to human readable format
void human_size(off_t bytes, char *buf, size_t size) {
  const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"}; // Predefines units
  double value = (double)bytes;
  int unit = 0;

  while (value >= 1024.0 && unit < 5) {
    value /= 1024.0;
    unit++;
  }

  snprintf(buf, size, "%.1f%s", value,
           units[unit]); // Format the size and store it in buf
}

// Helper function to make showing notifications easier
void status_message(const char *message, ...) {
  // Defining a variable for arguments
  va_list args;

  // Clearing screen before printing out anything
  move(STATUS_CORDS, 0);
  clrtoeol();

  va_start(args, message);
  vw_printw(stdscr, message, args);
  va_end(args);

  refresh();
  napms(2000); // Short delay before clearing the screen again

  move(STATUS_CORDS, 0);
  clrtoeol();
  refresh();
}

// Helper function to format the output of file_st.st_mode into "lrwxrwxrwx"
// format
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

// Show file details like file permissions, size, and name in the status bar
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

// Show current direcotry the user in on the topbar
void topbar() {
  char cwd[BUFFER_SIZE];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    mvhline(TOP_BAR_ROW, 0, ' ', COLS);
    mvprintw(TOP_BAR_ROW, 0, "%s", cwd);
  }
}

// Creating new folder/file
void create_folder_file(char *items[], int selected, int *count) {
  FILE *temp_file = NULL;
  char filename[BUFFER_SIZE];

  mvprintw(STATUS_CORDS, 0, "Filename (end with '/' for folder): ");
  getnstr(
      filename,
      sizeof(filename)); // Ask the user for the name then store it in filename

  size_t len = strlen(filename); // get the length of filename
  if (len > 0 &&
      filename[len - 1] ==
          '/') { // checking if the user ended it with / if yes create a folder
    struct stat st = {0};
    if (stat(filename, &st) == -1) {
      status_message("Created folder %s", filename); // Show a success message
      filename[len - 1] = '\0'; // end the string with the null terminator
      mkdir(filename, 0700);    // make the folder
      freeitems(items, *count); // free old items
      *count = loaddirectory(
          ".", items); // load the new directory and allocate new items
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }

    } else {
      status_message("Folder %s already exists", filename); // show fail message
    }
  } else {
    if (file_exists(filename)) { // if the folder/file exists
      status_message("File %s already exists", filename); // show a message
      if (temp_file)
        fclose(temp_file); // close and free the opened file
      return;
    }
    temp_file = fopen(filename, "w"); // open the file with the write mode

    if (temp_file) {
      status_message("Created %s", filename); // show sucess message
      fclose(temp_file);                      // close and free the opened file
      freeitems(items, *count);               // free old items
      *count = loaddirectory(
          ".", items); // load new direcotry and allocate new items
      if (selected >= *count) {
        selected = *count - 1;
      }
      if (selected < 0) {
        selected = 0;
      }
    }
  }
}
