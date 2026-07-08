#ifndef TUI_FUNCTIONS_H
#define TUI_FUNCTIONS_H

#include <wchar.h>
void status_message(const char *message, ...);
const wchar_t *file_icons(const char *filename);
void file_details(char *items[], int selected);
void topbar();

#endif
