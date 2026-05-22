#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "terminal.h"
#include "editor_types.h"

void move_cursor(int row, int col){
  dprintf(STDOUT_FILENO, "\033[%d;%dH", row, col);
}

void change_text_color(char *col_cd){ 
  dprintf(STDOUT_FILENO, col_cd);
}

void reset_screen(){
    dprintf(STDOUT_FILENO, "\x1b[48;2;30;41;59m");

    // clear screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void render_dirty_rows(editor_ctx *edt_ctx){
    erow buffer = {0, NULL};

    int screen_h = get_win_height() - 2;

    for(int y = 0; y < screen_h; y++){
        int filerow = y + edt_ctx->y_offset;

        // create rows until filerow exists
        while(filerow >= edt_ctx->numrows){
            create_new_erow(edt_ctx);
        }

        erow *row = &edt_ctx->rows[filerow];

        char tmp[128];

        // move cursor
        snprintf(tmp, sizeof(tmp), "\x1b[%d;1H", y + 1);
        append_str_erow(NULL, &buffer, tmp, buffer.size);

        // clear line
        append_str_erow(NULL, &buffer, "\x1b[2K", buffer.size);

        // line number
        snprintf(tmp, sizeof(tmp), "%4d | ", filerow + 1);
        append_str_erow(NULL, &buffer, tmp, buffer.size);

        // line contents
        if(row->buff)
            append_str_erow(NULL, &buffer, row->buff, buffer.size);
    }

    write(STDOUT_FILENO, buffer.buff, buffer.size);

    free(buffer.buff);
}

void render_status_row(editor_ctx *edt_ctx){
  char *mode_text = (edt_ctx->curr_opm == NORMAL ? "-- NORMAL --" : 
               (edt_ctx->curr_opm == INSERT ? "-- INSERT --" : "-- COMMAND --"));
    
  char *file_edited = (edt_ctx->file_modified == 1 ? "[+]" : " ");
  
  move_cursor(get_win_height() - 1, 2);
  
  char buffer[256];
  sprintf(buffer, "%s - %s  %s   \x1b[33m%s\x1b[39m    %d,%d     %s", 
          edt_ctx->curr_dir,
          edt_ctx->curr_file,
          file_edited,
          mode_text,
          edt_ctx->cx,
          edt_ctx->cy,
          edt_ctx->clipboard == NULL ? " " : "Clipboard Loaded!");

  write(STDOUT_FILENO, buffer, strlen(buffer));
}

void render_commd_row(editor_ctx *edt_ctx){
  char buffer[1000];
  sprintf(buffer, "\033[%d;%dH\x1b[2K:%s", get_win_height(), 1, edt_ctx->commd_row.buff);
  
  // draw row content
  write(
    STDOUT_FILENO,
    buffer,
    strlen(buffer)
  );
}

void roll_screen(editor_ctx *edt_ctx, int y_offset){
  edt_ctx->y_offset += y_offset;
  
  // clamp y_offset
  if(edt_ctx->y_offset < 0) edt_ctx->y_offset = 0;
}

void render_screen(editor_ctx *edt_ctx){
  reset_screen();

  render_dirty_rows(edt_ctx);
  render_status_row(edt_ctx);
  render_commd_row(edt_ctx);

  if(edt_ctx->curr_opm == COMMAND) move_cursor(get_win_height(), edt_ctx->cx + 1);
  else move_cursor(edt_ctx->cy, edt_ctx->cx + edt_ctx->x_offset);
}

