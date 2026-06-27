#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "screen.h"
#include "file_manager.h"
#include "editor_types.h"
#include "render_config.h"

int create_window(editor_ctx *edt_ctx, uint8_t flags){
    window_t window = {
        .window_id = edt_ctx->window_id_count,
        .cx = 1,
        .cy = 1,

        .x_offset = 7,
        .y_offset = 0,

        .curr_dir = NULL,
        .curr_file = NULL,
        .file_modified = 0,

        .dirty_rows = {0},
        .render_flags = flags,

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
    edt_ctx->window_id_count++;
    return 0;
}

int find_window_by_id(editor_ctx *edt_ctx, int id){
  for(int i = 0; i < edt_ctx->num_windows; i++){
    window_t *curr_window = &(edt_ctx->windows[i]);
    if(curr_window->window_id == id) return 1;
  }
  return 0;
}

void refresh_curr_window(editor_ctx *edt_ctx){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
  load_buffers_from_file(edt_ctx, curr_window->curr_file);
}

int change_window(editor_ctx *edt_ctx, int new_window){
    edt_ctx->curr_window = new_window;
    
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    if(curr_window->window_id == 1) refresh_curr_window(edt_ctx);
    
    // redraw background color 
    reset_screen(edt_ctx);
}

int change_window_by_offset(editor_ctx *edt_ctx, int offset){
    edt_ctx->curr_window += offset;
    
    // clamp curr_window value inside num_windows
    if(edt_ctx->curr_window >= edt_ctx->num_windows) edt_ctx->curr_window = 0;
    
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    if(curr_window->window_id == 1) refresh_curr_window(edt_ctx);
    
    // redraw background color 
    reset_screen(edt_ctx);
}


void open_window_by_id(editor_ctx *edt_ctx, int id){
  for(int i = 0; i < edt_ctx->num_windows; i++){
    window_t *curr_window = &(edt_ctx->windows[i]);
    if(curr_window->window_id == 1) change_window(edt_ctx, i);
  }
}

void close_window(editor_ctx *edt_ctx){
    if(edt_ctx->curr_window == edt_ctx->num_windows - 1) edt_ctx->curr_window--;
    edt_ctx->num_windows--;

    edt_ctx->windows = realloc(
        edt_ctx->windows,
        edt_ctx->num_windows * sizeof(window_t)
    );
}

