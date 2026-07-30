#include "syntax.h"
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdbool.h>

static const char *c_keywords[] = {
    "if", "else", "while", "for", "do", "return", "switch", "case", "break", 
    "continue", "default", "struct", "class", "public", "private", "protected", 
    "import", "export", "package", "new", "this", "super", "fn", "let", "mut", 
    "var", "const", "function", "func", "type", "interface", "enum", "goto", 
    "sizeof", "typedef", "extern", "static", "register", "volatile", "inline", NULL
};

static const char *c_types[] = {
    "int", "char", "float", "double", "void", "short", "long", "unsigned", 
    "signed", "bool", "boolean", "string", "String", "size_t", "int8_t", 
    "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", NULL
};

static const char *py_keywords[] = {
    "def", "class", "if", "elif", "else", "while", "for", "in", "try", "except", 
    "finally", "raise", "import", "from", "as", "return", "yield", "pass", "break", 
    "continue", "and", "or", "not", "is", "lambda", "assert", "global", "nonlocal", 
    "with", "del", NULL
};

static const char *py_types[] = {
    "True", "False", "None", "self", "str", "int", "float", "dict", "list", "set", "tuple", "object", NULL
};

static const char *sh_keywords[] = {
    "if", "then", "elif", "else", "fi", "for", "while", "in", "do", "done", 
    "case", "esac", "function", "local", "return", "exit", "echo", NULL
};

LangType get_lang_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) {
        if (strcasecmp(filename, "Makefile") == 0) {
            return LANG_SHELL;
        }
        return LANG_NONE;
    }
    
    if (strcasecmp(ext, ".c") == 0 || strcasecmp(ext, ".h") == 0 ||
        strcasecmp(ext, ".cpp") == 0 || strcasecmp(ext, ".hpp") == 0 ||
        strcasecmp(ext, ".cc") == 0 || strcasecmp(ext, ".java") == 0 ||
        strcasecmp(ext, ".rs") == 0 || strcasecmp(ext, ".go") == 0 ||
        strcasecmp(ext, ".js") == 0 || strcasecmp(ext, ".ts") == 0 ||
        strcasecmp(ext, ".dart") == 0 || strcasecmp(ext, ".json") == 0 ||
        strcasecmp(ext, ".cs") == 0 || strcasecmp(ext, ".php") == 0) {
        return LANG_C_LIKE;
    }
    
    if (strcasecmp(ext, ".py") == 0) {
        return LANG_PYTHON;
    }
    
    if (strcasecmp(ext, ".sh") == 0 || strcasecmp(ext, ".bash") == 0) {
        return LANG_SHELL;
    }
    
    return LANG_NONE;
}

void init_syntax_colors(void) {
    if (has_colors()) {
        init_pair(COLOR_KEYWORD, COLOR_CYAN, -1);
        init_pair(COLOR_TYPE, COLOR_BLUE, -1);
        init_pair(COLOR_COMMENT, COLOR_GREEN, -1);
        init_pair(COLOR_STRING, COLOR_YELLOW, -1);
        init_pair(COLOR_NUMBER, COLOR_RED, -1);
        init_pair(COLOR_PREPROC, COLOR_MAGENTA, -1);
    }
}

void print_highlighted_line(int y, int x, const char *line, int max_width, int *in_multiline_comment, LangType lang) {
    int len = strlen(line);
    int i = 0;
    int col = 0;
    
    move(y, x);
    
    while (i < len && col < max_width) {
        // 1. Handle tabs explicitly to expand to spaces correctly
        if (line[i] == '\t') {
            int tab_spaces = 4 - (col % 4);
            for (int s = 0; s < tab_spaces && col < max_width; s++) {
                addch(' ');
                col++;
            }
            i++;
            continue;
        }

        // 2. Handle Multiline Comment (C-like languages)
        if (lang == LANG_C_LIKE && *in_multiline_comment) {
            attron(COLOR_PAIR(COLOR_COMMENT));
            while (i < len && col < max_width) {
                char c = line[i];
                if (c == '\t') {
                    int tab_spaces = 4 - (col % 4);
                    for (int s = 0; s < tab_spaces && col < max_width; s++) {
                        addch(' ');
                        col++;
                    }
                    i++;
                    continue;
                }
                addch(c);
                col++;
                i++;
                if (c == '*' && i < len && line[i] == '/') {
                    addch('/');
                    col++;
                    i++;
                    *in_multiline_comment = 0;
                    break;
                }
            }
            attroff(COLOR_PAIR(COLOR_COMMENT));
            continue;
        }
        
        // 3. Handle single-line comment
        if (lang == LANG_C_LIKE && line[i] == '/' && i + 1 < len && line[i+1] == '/') {
            attron(COLOR_PAIR(COLOR_COMMENT));
            while (i < len && col < max_width) {
                if (line[i] == '\t') {
                    int tab_spaces = 4 - (col % 4);
                    for (int s = 0; s < tab_spaces && col < max_width; s++) {
                        addch(' ');
                        col++;
                    }
                    i++;
                    continue;
                }
                addch(line[i]);
                col++;
                i++;
            }
            attroff(COLOR_PAIR(COLOR_COMMENT));
            break; // Done with this line
        }
        if ((lang == LANG_PYTHON || lang == LANG_SHELL) && line[i] == '#') {
            attron(COLOR_PAIR(COLOR_COMMENT));
            while (i < len && col < max_width) {
                if (line[i] == '\t') {
                    int tab_spaces = 4 - (col % 4);
                    for (int s = 0; s < tab_spaces && col < max_width; s++) {
                        addch(' ');
                        col++;
                    }
                    i++;
                    continue;
                }
                addch(line[i]);
                col++;
                i++;
            }
            attroff(COLOR_PAIR(COLOR_COMMENT));
            break; // Done with this line
        }
        
        // 4. Handle start of multiline comment
        if (lang == LANG_C_LIKE && line[i] == '/' && i + 1 < len && line[i+1] == '*') {
            *in_multiline_comment = 1;
            attron(COLOR_PAIR(COLOR_COMMENT));
            addch('/');
            addch('*');
            col += 2;
            i += 2;
            attroff(COLOR_PAIR(COLOR_COMMENT));
            continue;
        }
        
        // 5. Handle Strings
        if (line[i] == '"' || line[i] == '\'') {
            char quote_char = line[i];
            attron(COLOR_PAIR(COLOR_STRING));
            addch(quote_char);
            col++;
            i++;
            while (i < len && col < max_width) {
                char c = line[i];
                if (c == '\\' && i + 1 < len) {
                    addch('\\');
                    addch(line[i+1]);
                    col += 2;
                    i += 2;
                    continue;
                }
                if (c == '\t') {
                    int tab_spaces = 4 - (col % 4);
                    for (int s = 0; s < tab_spaces && col < max_width; s++) {
                        addch(' ');
                        col++;
                    }
                    i++;
                    continue;
                }
                addch(c);
                col++;
                i++;
                if (c == quote_char) {
                    break;
                }
            }
            attroff(COLOR_PAIR(COLOR_STRING));
            continue;
        }
        
        // 6. Handle preprocessor directives (C-like only)
        if (lang == LANG_C_LIKE && line[i] == '#' && col == 0) {
            char preproc_word[64] = {0};
            int pw_len = 0;
            addch('#');
            col++;
            i++;
            while (i < len && (isalpha(line[i]) || line[i] == '_') && pw_len < 63) {
                preproc_word[pw_len++] = line[i++];
            }
            attron(COLOR_PAIR(COLOR_PREPROC));
            addstr(preproc_word);
            col += pw_len;
            attroff(COLOR_PAIR(COLOR_PREPROC));
            continue;
        }
        
        // 7. Handle numbers
        if (isdigit(line[i])) {
            attron(COLOR_PAIR(COLOR_NUMBER));
            while (i < len && col < max_width && (isxdigit(line[i]) || line[i] == '.' || line[i] == 'x' || line[i] == 'X')) {
                addch(line[i]);
                col++;
                i++;
            }
            attroff(COLOR_PAIR(COLOR_NUMBER));
            continue;
        }
        
        // 8. Handle identifiers (Keywords, Types, variables)
        if (isalpha(line[i]) || line[i] == '_') {
            char word[128] = {0};
            int w_len = 0;
            while (i < len && (isalnum(line[i]) || line[i] == '_') && w_len < 127) {
                word[w_len++] = line[i++];
            }
            
            bool is_kw = false;
            bool is_tp = false;
            
            const char **kws = NULL;
            const char **tps = NULL;
            if (lang == LANG_C_LIKE) {
                kws = c_keywords;
                tps = c_types;
            } else if (lang == LANG_PYTHON) {
                kws = py_keywords;
                tps = py_types;
            } else if (lang == LANG_SHELL) {
                kws = sh_keywords;
            }
            
            if (kws) {
                for (int k = 0; kws[k] != NULL; k++) {
                    if (strcmp(word, kws[k]) == 0) {
                        is_kw = true;
                        break;
                    }
                }
            }
            if (tps && !is_kw) {
                for (int k = 0; tps[k] != NULL; k++) {
                    if (strcmp(word, tps[k]) == 0) {
                        is_tp = true;
                        break;
                    }
                }
            }
            
            if (is_kw) {
                attron(COLOR_PAIR(COLOR_KEYWORD));
            } else if (is_tp) {
                attron(COLOR_PAIR(COLOR_TYPE));
            }
            
            // Limit drawing word if it exceeds max_width
            for (int w = 0; w < w_len && col < max_width; w++) {
                addch(word[w]);
                col++;
            }
            
            if (is_kw) {
                attroff(COLOR_PAIR(COLOR_KEYWORD));
            } else if (is_tp) {
                attroff(COLOR_PAIR(COLOR_TYPE));
            }
            continue;
        }
        
        // 9. Default fallback
        addch(line[i]);
        col++;
        i++;
    }
}
