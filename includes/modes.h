#ifndef MODES
#define MODES

#include "editor_types.h"

void handle_normal_mode(editor_ctx *edt_ctx, char key);
void handle_insert_mode(editor_ctx *edt_ctx, char key);
void handle_command_mode(editor_ctx *edt_ctx, char key);

#endif
