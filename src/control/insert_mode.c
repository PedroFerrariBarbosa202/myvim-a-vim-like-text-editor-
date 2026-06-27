#include <unistd.h>
#include <stdio.h>

#include "screen.h"
#include "buffer.h"
#include "editor_types.h"
#include "terminal.h"
#include "commands.h"
#include "windows.h"

void handle_insert_mode(editor_ctx *edt_ctx, int key){
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
  // handle arrow keys
  else if(key >= 1000 && key <= 1003){
    switch(key){
      case ARROW_UP:
        // roll screen up when reaching upper border
        if(curr_window->cy <= 1 && curr_window->y_offset != 0) roll_screen(edt_ctx, -1);
        else curr_window->cy--;

        if(curr_window->cy < 1) curr_window->cy = 1;
        break;
      case ARROW_DOWN:
        // roll everything down to show lower part of screen
        if(curr_window->cy > get_win_height() - 4) roll_screen(edt_ctx, 1);
        else curr_window->cy++;

        if(curr_window->cy > curr_window->numrows) create_new_erow(edt_ctx);
        break;
      case ARROW_LEFT:  
        curr_window->cx--;
        break;
      case ARROW_RIGHT:
        curr_window->cx++;
        break;
    }
  }

  // special case: autofill the closing variants of some characters
  else if(key == '"' || key == '\''){
    erow *row = &(curr_window->rows[curr_window->cy + curr_window->y_offset - 1]);
    char str[3] = { key, key,'\0' };
    append_str_erow(edt_ctx, row, str, curr_window->cx - 1);
    curr_window->cx++;
  }

  else if(key == '{'){
    erow *row = &(curr_window->rows[curr_window->cy + curr_window->y_offset - 1]);
    char str[3] = { key, '}','\0' };
    append_str_erow(edt_ctx, row, str, curr_window->cx - 1);
    curr_window->cx++;
  }

  else if(key == '('){
    erow *row = &(curr_window->rows[curr_window->cy + curr_window->y_offset - 1]);
    char str[3] = { key, ')','\0' };
    append_str_erow(edt_ctx, row, str, curr_window->cx - 1);
    curr_window->cx++;
  }



  else{
    erow *row = &(curr_window->rows[curr_window->cy + curr_window->y_offset - 1]);
    char str[2] = { key, '\0' };
    append_str_erow(edt_ctx, row, str, curr_window->cx - 1);
    curr_window->cx++;
  }
  
  curr_window->dirty_rows[curr_window->cy + curr_window->y_offset] = 1;
}
