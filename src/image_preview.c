#include "image_preview.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int image_preview_supported() {
  const char *term = getenv("TERM");
  const char *term_program = getenv("TERM_PROGRAM");

  if (term_program && strcmp(term_program, "WezTerm") == 0)
    return 1;

  if (term && strstr(term, "kitty"))
    return 1;

  if (term && strstr(term, "konsole"))
    return 1;

  return 0;
}

bool is_image_file(const char *filename) {
  if (!filename)
    return false;

  const char *ext = strrchr(filename, '.');
  if (!ext)
    return false;
  ext++; // Move past the dot

  const char *image_extensions[] = {"png", "jpg",  "jpeg", "gif",
                                    "bmp", "webp", "tiff", "ico"};
  int num_extensions = sizeof(image_extensions) / sizeof(image_extensions[0]);

  for (int i = 0; i < num_extensions; i++) {
    if (strcasecmp(ext, image_extensions[i]) == 0) {
      return true;
    }
  }
  return false;
}

void draw_image_preview(const char *filename, int x, int y, int width,
                        int height) {
  if (!image_preview_supported())
    return;
  // Clear any existing preview first
  clear_image_preview();

  pid_t pid = fork();
  if (pid == 0) {
    char place_arg[64];
    // Format: <width>x<height>@<x>x<y>
    snprintf(place_arg, sizeof(place_arg), "%dx%d@%dx%d", width, height, x, y);

    // Execute kitty +kitten icat
    execlp("kitty", "kitty", "+kitten", "icat", "--transfer-mode=file",
           "--place", place_arg, "--silent", "--stdin=no", filename,
           (char *)NULL);

    // If execution fails, exit silently
    _exit(127);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
  }
}

void clear_image_preview() {
  if (!image_preview_supported())
    return;
  pid_t pid = fork();
  if (pid == 0) {
    // Execute kitty +kitten icat --clear
    execlp("kitty", "kitty", "+kitten", "icat", "--clear", "--silent",
           "--stdin=no", (char *)NULL);

    _exit(127);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
  }
}
