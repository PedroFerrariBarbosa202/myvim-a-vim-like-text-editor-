#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "terminal.h"
#include "editor_types.h"

#define KEYWORD_COLOR  "\x1b[31m"   

void move_cursor(int row, int col){
  dprintf(STDOUT_FILENO, "\033[%d;%dH", row, col);
}

void change_text_color(char *col_cd){ 
  dprintf(STDOUT_FILENO, col_cd);
}

void reset_screen(editor_ctx *edt_ctx){
    static const int colors[][3] = {
        {30, 41, 59},    // slate blue
        {35, 45, 70},    // navy
        {28, 52, 63},    // teal
        {45, 35, 65},    // purple
        {25, 55, 45},    // emerald
        {55, 30, 45},    // wine
        {40, 40, 40},    // graphite
    };

    int idx = edt_ctx->curr_window % (sizeof(colors) / sizeof(colors[0]));

    dprintf(
        STDOUT_FILENO,
        "\x1b[48;2;%d;%d;%dm",
        colors[idx][0],
        colors[idx][1],
        colors[idx][2]
    );
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void check_colored_words(editor_ctx *edt_ctx, erow *curr_row){
  static char* keywords[] = {
        "if",
        "else",
        "while",
        "for",
        "void",
        "int",
        "float",
        "static",
        "extern",
        "char",
        "#include",
        "#define"
    };
  
  int s = 0, e = 0;

  while(e < curr_row->size){
    char buffer[1024];
    int i = 0;

    while (curr_row->buff[e] != ' ' &&
       curr_row->buff[e] != '\0' &&
       i < sizeof(buffer) - 1)
    {
       buffer[i++] = curr_row->buff[e++];
    }

    buffer[i] = '\0';

    // check if word is equal to buffer
    int n_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for(int i = 0; i < n_keywords; i++){
      if(strcmp(buffer, keywords[i]) == 0){
        append_str_erow(edt_ctx, curr_row, KEYWORD_COLOR, s);
        append_str_erow(edt_ctx, curr_row, "\x1b[39m", e + strlen(KEYWORD_COLOR));
      }
    }
    if (curr_row->buff[e] == ' ')
      e++;

    s = e;
  }
}

void render_dirty_rows(editor_ctx *edt_ctx){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    erow buffer = {0, NULL};

    int screen_h = get_win_height() - 2;

    for(int y = 0; y < screen_h; y++){
        int filerow = y + curr_window->y_offset;

        // create rows until filerow exists
        while(filerow >= curr_window->numrows){
            create_new_erow(edt_ctx);
        }

        erow *row = &curr_window->rows[filerow];

        char tmp[128];

        // move cursor
        snprintf(tmp, sizeof(tmp), "\x1b[%d;1H", y + 1);
        append_str_erow(NULL, &buffer, tmp, buffer.size);

        // clear line
        append_str_erow(NULL, &buffer, "\x1b[2K", buffer.size);

        // line number
        snprintf(tmp, sizeof(tmp), "\x1b[39m%4d | ", filerow + 1);
        append_str_erow(NULL, &buffer, tmp, buffer.size);

        // line contents
        if(row->buff)
            append_str_erow(NULL, &buffer, row->buff, buffer.size);
            check_colored_words(edt_ctx, &buffer);
    }

    write(STDOUT_FILENO, buffer.buff, buffer.size);

    free(buffer.buff);
}

void render_status_row(editor_ctx *edt_ctx){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  char *mode_text = (edt_ctx->curr_opm == NORMAL ? "-- NORMAL --" : 
               (edt_ctx->curr_opm == INSERT ? "-- INSERT --" : "-- COMMAND --"));
    
  char *file_edited = (curr_window->file_modified == 1 ? "[+]" : " ");
  
  move_cursor(get_win_height() - 1, 2);
  
  char buffer[256];
  sprintf(buffer, "%s - %s  %s   \x1b[33m%s\x1b[39m    %d,%d    window: %d/%d     %s", 
          curr_window->curr_dir,
          curr_window->curr_file,
          file_edited,
          mode_text,
          curr_window->cx,
          curr_window->cy,
          edt_ctx->curr_window + 1,
          edt_ctx->num_windows,
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
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
  curr_window->y_offset += y_offset;
  
  // clamp y_offset
  if(curr_window->y_offset < 0) curr_window->y_offset = 0;
}

void render_screen(editor_ctx *edt_ctx){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  reset_screen(edt_ctx);
  
  render_dirty_rows(edt_ctx);
  render_status_row(edt_ctx);
  render_commd_row(edt_ctx);

  if(edt_ctx->curr_opm == COMMAND) move_cursor(get_win_height(), edt_ctx->commd_row_cx + 1);
  else move_cursor(curr_window->cy, curr_window->cx + curr_window->x_offset);
}

