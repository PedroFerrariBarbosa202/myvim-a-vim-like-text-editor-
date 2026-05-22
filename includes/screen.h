#ifndef SCREEN
#define SCREEN

#include "editor_types.h"

#define LINE_MAX_SIZE 524

void move_cursor(int row, int col);
void change_text_color(char *col_cd);
void reset_screen();

void render_screen(editor_ctx *edt_ctx);
void roll_screen(editor_ctx *edt_ctx, int y_offset);

#endif
