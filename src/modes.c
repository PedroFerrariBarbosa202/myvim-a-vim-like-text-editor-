#include <unistd.h>
#include <stdio.h>

#include "screen.h"
#include "buffer.h"
#include "editor_types.h"
#include "terminal.h"
#include "commands.h"

void handle_normal_mode(editor_ctx *edt_ctx, char key){
  if(key == 'h') edt_ctx->cx--;
  else if(key == 'l') edt_ctx->cx++;

  else if(key == 'k') {
    // roll screen up when reaching upper border
    if(edt_ctx->cy <= 1 && edt_ctx->y_offset != 0) roll_screen(edt_ctx, -1);
    else edt_ctx->cy--;

    if(edt_ctx->cy < 1) edt_ctx->cy = 1;
  }

  // if loking into a new line, create it on memory
  else if(key == 'j') { 
    // roll everything down to show lower part of screen
    if(edt_ctx->cy > get_win_height() - 3) roll_screen(edt_ctx, 1);
    else edt_ctx->cy++;

    if(edt_ctx->cy > edt_ctx->numrows)
      create_new_erow(edt_ctx);
  }

  // if user press ':', switch into command mode
  else if(key == ':') {
    edt_ctx->saved_cx = edt_ctx->cx;
    edt_ctx->saved_cy = edt_ctx->cy;

    edt_ctx->curr_opm = COMMAND;

    edt_ctx->cx = 1;

    edt_ctx->commd_row.buff = NULL;
    edt_ctx->commd_row.size = 0;

    return;
  }
  
  // CTRL+C
  if(key == 'y')copy_line(edt_ctx, edt_ctx->cy + edt_ctx->y_offset);

  // CTRL+V
  if(key == 'p')paste_line(edt_ctx, edt_ctx->cy + edt_ctx->y_offset, edt_ctx->cx);
  
  // delete line logic
  if(key == 'd') delete_erow_buffer(edt_ctx, edt_ctx->cy + edt_ctx->y_offset);

  // 'o' and 'i' key logic
  else if(key == 'i') edt_ctx->cx;
  else if(key == 'o' ) edt_ctx->cx = edt_ctx->rows[edt_ctx->cy + edt_ctx->y_offset - 1].size + 1;
}

void handle_insert_mode(editor_ctx *edt_ctx, char key){
  // DELETE key logic
  if(key == 127){
    // normal delete inside line
    if(delete_erow_char(edt_ctx) == 0){
        edt_ctx->cx--;
    }
    // backspace at beginning of line
    else if(edt_ctx->cy > 1 || edt_ctx->y_offset > 0){
        int curr_filerow = edt_ctx->cy + edt_ctx->y_offset - 1;
        int prev_filerow = curr_filerow - 1;

        erow *curr = &edt_ctx->rows[curr_filerow];
        erow *prev = &edt_ctx->rows[prev_filerow];

        int prev_size = prev->size;

        // append current line into previous line
        if(curr->buff != NULL && curr->size > 0)
          append_str_erow(edt_ctx, prev, curr->buff, prev->size);

        // remove current line
        roll_up_buffers(edt_ctx, curr_filerow);

        // move cursor
        if(edt_ctx->cy == 1 && edt_ctx->y_offset > 0){
            roll_screen(edt_ctx, -1);
        } else {
            edt_ctx->cy--;
        }

        edt_ctx->cx = prev_size + 1;
    }
  }
  
  // ENTER key logic
  else if(key == '\r' || key == '\n'){
    slice_and_down_buffer(edt_ctx, edt_ctx->cy);

    edt_ctx->cy++;
    edt_ctx->cx = 1;

    // roll window when reaching lower border
    if(edt_ctx->cy > get_win_height() - 2) {
      roll_screen(edt_ctx, 1);
      edt_ctx->cy--;
    }
  }

  else{
    erow *row = &(edt_ctx->rows[edt_ctx->cy + edt_ctx->y_offset - 1]);
    char str[2] = { key, '\0' };
    append_str_erow(edt_ctx, row, str, edt_ctx->cx - 1);
    edt_ctx->cx++;
  }
}

void handle_command_mode(editor_ctx *edt_ctx, char key){
  // DELETE key logic
  if(key == 127) {
    if(delete_char_commd_erow(edt_ctx) == 0)
      edt_ctx->cx--;
  }
  // ENTER key logic
  else if(key == '\r' || key == '\n'){
    edt_ctx->cx = edt_ctx->saved_cx;
    edt_ctx->cy = edt_ctx->saved_cy;

    // execute command
    exec_command(edt_ctx, edt_ctx->commd_row.buff);
  
    // clean command row
    edt_ctx->commd_row.buff = NULL;
 
    edt_ctx->curr_opm = NORMAL;
    return;
  }
  else{
    insert_char_commd_erow(edt_ctx, key);
    edt_ctx->cx++;
  }
}
