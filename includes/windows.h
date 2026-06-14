#ifndef WINDOWS
#define WINDOWS

#include "editor_types.h"

int create_window(editor_ctx *edt_ctx);
int change_window(editor_ctx *edt_ctx, int new_window);
#endif
