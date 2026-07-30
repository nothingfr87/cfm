#ifndef SYNTAX_H
#define SYNTAX_H

#include <ncurses.h>

#define COLOR_KEYWORD 3
#define COLOR_TYPE    4
#define COLOR_COMMENT 5
#define COLOR_STRING  6
#define COLOR_NUMBER  7
#define COLOR_PREPROC 8

typedef enum {
    LANG_NONE,
    LANG_C_LIKE, // C, C++, Java, Rust, Go, JS, TS, Dart, C#, PHP, JSON
    LANG_PYTHON,
    LANG_SHELL
} LangType;

// Identifies the programming language from file extension
LangType get_lang_type(const char *filename);

// Initialises ncurses color pairs for syntax highlighting
void init_syntax_colors(void);

// Prints a single line of text with syntax highlighting to the standard window
void print_highlighted_line(int y, int x, const char *line, int max_width, int *in_multiline_comment, LangType lang);

#endif // SYNTAX_H
