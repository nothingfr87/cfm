#ifndef IMAGE_PREVIEW_H
#define IMAGE_PREVIEW_H

#include <stdbool.h>

/**
 * Checks if a file is an image based on its extension.
 */
bool is_image_file(const char *filename);

/**
 * Draws an image preview using the Kitty graphics protocol (icat).
 *
 * @param filename Path to the image file
 * @param x Column offset where the preview area starts
 * @param y Row offset where the preview area starts
 * @param width Width in character columns
 * @param height Height in character rows
 */
void draw_image_preview(const char *filename, int x, int y, int width, int height);

/**
 * Clears any active image preview drawn via the Kitty graphics protocol.
 */
void clear_image_preview();

#endif // IMAGE_PREVIEW_H
