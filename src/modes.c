#include <unistd.h>
#include <stdio.h>

#include "screen.h"
#include "buffer.h"
#include "editor_types.h"
#include "terminal.h"
#include "commands.h"
#include "windows.h"

void handle_normal_mode(editor_ctx *edt_ctx, char key){
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
    if(curr_window->cy > get_win_height() - 3) roll_screen(edt_ctx, 1);
    else curr_window->cy++;

    if(curr_window->cy > curr_window->numrows)
      create_new_erow(edt_ctx);
  }

  else if(key == 'n'){
    edt_ctx->curr_window++;
    if(edt_ctx->curr_window >= edt_ctx->num_windows) edt_ctx->curr_window = 0;
    change_window(edt_ctx, edt_ctx->curr_window);
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

void handle_insert_mode(editor_ctx *edt_ctx, char key){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  // DELETE key logic
  if(key == 127){
    // normal delete inside line
    if(delete_erow_char(edt_ctx) == 0){
        curr_window->cx--;
    }
    // backspace at beginning of line
    else if(curr_window->cy > 1 || curr_window->y_offset > 0){
        int curr_filerow = curr_window->cy + curr_window->y_offset - 1;
        int prev_filerow = curr_filerow - 1;

        erow *curr = &curr_window->rows[curr_filerow];
        erow *prev = &curr_window->rows[prev_filerow];

        int prev_size = prev->size;

        // append current line into previous line
        if(curr->buff != NULL && curr->size > 0)
          append_str_erow(edt_ctx, prev, curr->buff, prev->size);

        // remove current line
        roll_up_buffers(edt_ctx, curr_filerow);

        // move cursor
        if(curr_window->cy == 1 && curr_window->y_offset > 0){
            roll_screen(edt_ctx, -1);
        } else {
            curr_window->cy--;
        }

        curr_window->cx = prev_size + 1;
    }
  }
  
  // ENTER key logic
  else if(key == '\r' || key == '\n'){
    slice_and_down_buffer(edt_ctx, curr_window->cy);

    curr_window->cy++;
    curr_window->cx = 1;

    // roll window when reaching lower border
    if(curr_window->cy > get_win_height() - 2) {
      roll_screen(edt_ctx, 1);
      curr_window->cy--;
    }
  }

  else{
    erow *row = &(curr_window->rows[curr_window->cy + curr_window->y_offset - 1]);
    char str[2] = { key, '\0' };
    append_str_erow(edt_ctx, row, str, curr_window->cx - 1);
    curr_window->cx++;
  }
}

void handle_command_mode(editor_ctx *edt_ctx, char key){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  // DELETE key logic
  if(key == 127) {
    if(delete_char_commd_erow(edt_ctx) == 0)
      edt_ctx->commd_row_cx--;
  }
  // ENTER key logic
  else if(key == '\r' || key == '\n'){
    // execute command
    exec_command(edt_ctx, edt_ctx->commd_row.buff);
  
    // clean command row
    edt_ctx->commd_row.buff = NULL;
    edt_ctx->commd_row_cx = 1;
    edt_ctx->curr_opm = NORMAL;
    return;
  }
  else{
    insert_char_commd_erow(edt_ctx, key);
    edt_ctx->commd_row_cx++;
  }
}
