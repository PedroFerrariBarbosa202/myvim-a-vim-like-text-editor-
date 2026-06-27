#ifndef WINDOWS
#define WINDOWS

#include <stdint.h>

#include "editor_types.h"

int find_window_by_id(editor_ctx *edt_ctx, int id);
int change_window_by_offset(editor_ctx *edt_ctx, int offset);
int create_window(editor_ctx *edt_ctx, uint8_t flags);
void refresh_curr_window(editor_ctx *edt_ctx);
int change_window(editor_ctx *edt_ctx, int new_window);
void open_window_by_id(editor_ctx *edt_ctx, int id);
void close_window(editor_ctx *edt_ctx);

#endif
