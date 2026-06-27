#include <unistd.h>
#include <stdio.h>

#include "screen.h"
#include "buffer.h"
#include "editor_types.h"
#include "terminal.h"
#include "commands.h"
#include "windows.h"

void handle_normal_mode(editor_ctx *edt_ctx, int key){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  if(key == 'h') curr_window->cx--;
  else if(key == 'l') curr_window->cx++;

  else if(key == 'k') {
    // roll screen up when reaching upper border
    if(curr_window->cy <= 1 && curr_window->y_offset != 0) roll_screen(edt_ctx, -1);
    else curr_window->cy--;

    if(curr_window->cy < 1) curr_window->cy = 1;
  }

  // if loking into a new line, create it on memory
  else if(key == 'j') { 
    // roll everything down to show lower part of screen
    if(curr_window->cy > get_win_height() - 4) roll_screen(edt_ctx, 1);
    else curr_window->cy++;

    if(curr_window->cy > curr_window->numrows)
      create_new_erow(edt_ctx);
  }

  else if(key == 'n'){
    edt_ctx->curr_window++;
    if(edt_ctx->curr_window >= edt_ctx->num_windows) edt_ctx->curr_window = 0;
    change_window(edt_ctx, edt_ctx->curr_window);
  
    // refresh curr_window
    curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    if(curr_window->window_id == 1 || curr_window->window_id == 0) refresh_curr_window(edt_ctx);
  }

  // if user press ':', switch into command mode
  else if(key == ':') {
    edt_ctx->curr_opm = COMMAND;

    curr_window->cx = 1;

    edt_ctx->commd_row.buff = NULL;
    edt_ctx->commd_row.size = 0;

    return;
  }
  
  // CTRL+C
  if(key == 'y')copy_line(edt_ctx, curr_window->cy + curr_window->y_offset);

  // CTRL+V
  if(key == 'p')paste_line(edt_ctx, curr_window->cy + curr_window->y_offset, curr_window->cx);
  
  // delete line logic
  if(key == 'd') delete_erow_buffer(edt_ctx, curr_window->cy + curr_window->y_offset);

  // 'o' and 'i' key logic
  else if(key == 'i') edt_ctx->curr_opm = INSERT;
  else if(key == 'o' ) curr_window->cx = curr_window->rows[curr_window->cy + curr_window->y_offset - 1].size + 1;
}

