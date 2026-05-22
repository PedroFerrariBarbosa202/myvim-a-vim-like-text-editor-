#ifndef BUFFER 
#define BUFFER

#include "editor_types.h"

int readKey();
void create_new_erow(editor_ctx *edt_ctx);
void append_str_erow(editor_ctx *edt_ctx, erow *row, char *str, int at);
int delete_erow_char(editor_ctx *edt_ctx);

void insert_char_commd_erow(editor_ctx *edt_ctx, char chr);
int delete_char_commd_erow(editor_ctx *edt_ctx);
void clear_rows(editor_ctx *edt_ctx);
void roll_down_buffers(editor_ctx *edt_ctx, int at);
void roll_up_buffers(editor_ctx *edt_ctx, int at);
void slice_and_down_buffer(editor_ctx *edt_ctx, int at);
void push_up_buffer(editor_ctx *edt_ctx, int at);

void copy_line(editor_ctx *edt_ctx, int at);
void paste_line(editor_ctx *edt_ctx, int at_y, int at_x);

int delete_erow_buffer(editor_ctx *edt_ctx, int at);
#endif
