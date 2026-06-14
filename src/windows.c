#include <stdlib.h>
#include <stdio.h>

#include "screen.h"
#include "editor_types.h"

int create_window(editor_ctx *edt_ctx){
    window_t window = {
        .cx = 1,
        .cy = 1,

        .x_offset = 7,
        .y_offset = 0,
        .x_padding = 0,

        .curr_dir = NULL,
        .curr_file = NULL,
        .file_modified = 0,

        .rows = NULL,
        .numrows = 0,
    };

    int idx = edt_ctx->num_windows;
    edt_ctx->num_windows++;

    edt_ctx->windows = realloc(
        edt_ctx->windows,
        edt_ctx->num_windows * sizeof(window_t)
    );

    edt_ctx->windows[idx] = window;
    return 0;
}

int change_window(editor_ctx *edt_ctx, int new_window){
    edt_ctx->curr_window = new_window;
}
