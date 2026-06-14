#ifndef FILE_MANAGER
#define FILE_MANAGER

#include "editor_types.h"
#include "buffer.h"

int load_buffers_from_file(editor_ctx *edt_ctx, char *file_path);
int load_buffers_from_static_dir(editor_ctx *edt_ctx, char *file_name);
int save_file_from_buffers(editor_ctx *edt_ctx, char *new_name);
void set_current_directory(editor_ctx *edt_ctx, char *new_dir);

#endif
