#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "terminal.h"
#include "editor_types.h"
#include "render_config.h"
#include "macros.h"

#define KEYWORD_COLOR  "\x1b[31m"

#define IRC_RENDER_ALL_LINES  0

void move_cursor(int row, int col){
  dprintf(STDOUT_FILENO, "\033[%d;%dH", row, col);
}

void change_text_color(char *col_cd){ 
  dprintf(STDOUT_FILENO, col_cd);
}

void shift_cursor(editor_ctx *edt_ctx, int offset){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
  move_cursor(curr_window->cy, curr_window->cx + curr_window->x_offset + edt_ctx->abs_x_offset - 1);
}

void reset_screen(editor_ctx *edt_ctx){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    static const int colors[][3] = {
       {30, 41, 59},// slate blue
    };
  
    dprintf(
        STDOUT_FILENO,
        "\x1b[48;2;%d;%d;%dm",
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void check_colored_words(editor_ctx *edt_ctx, erow *curr_row) {
    static const char delimiters[] = " \n\t\0(){}[];,+-*/%=&|!<>\"'";
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
        "#define",
    };

    int e = 0;

    while (e < curr_row->size) {
        // skip spaces
        while (e < curr_row->size && strchr(delimiters, curr_row->buff[e]))
            e++;

        int s = e;

        char word[1024];
        int i = 0;
        while (e < curr_row->size &&
               !strchr(delimiters, curr_row->buff[e]) &&
               i < (int)sizeof(word) - 1){
          word[i++] = curr_row->buff[e++];
        }

        word[i] = '\0';
        if (i == 0) break;

        int n_keywords = sizeof(keywords) / sizeof(keywords[0]);
        for (int k = 0; k < n_keywords; k++) {
            if (strcmp(word, keywords[k]) == 0) {
                append_str_erow(edt_ctx, curr_row, "\x1b[39m", e);
                append_str_erow(edt_ctx, curr_row, KEYWORD_COLOR, s);
                e += strlen(KEYWORD_COLOR) + strlen("\x1b[39m");
                break;
            }
        }
    }
}

void render_dirty_rows(editor_ctx *edt_ctx, int internal_flags){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    erow buffer = {0, NULL};

    int screen_h = get_win_height() - 2;

    for(int y = 1; y <= screen_h; y++) {
      int filerow = y + curr_window->y_offset - 1;

      while(filerow >= curr_window->numrows) {
        create_new_erow(edt_ctx);
      }

      erow *row = &curr_window->rows[filerow];
      char tmp[256];

      // Draw line number
      uint8_t flags = get_render_flags(edt_ctx);
      flags = flags & RC_RENDER_LINE_NUMBER;
      if(flags == RC_RENDER_LINE_NUMBER){
        snprintf(tmp, sizeof(tmp), "\x1b[%d;%dH%4d | ",
             y,
             edt_ctx->abs_x_offset + 1,
             filerow + 1);
        append_str_erow(NULL, &buffer, tmp, buffer.size);
      }

      // Draw row contents
      // get RC_RENDER_ALL mask;
      flags = get_render_flags(edt_ctx);
      flags = flags & RC_RENDER_CONTENT;
      if((curr_window->dirty_rows[y] && flags == RC_RENDER_CONTENT) || internal_flags == IRC_RENDER_ALL_LINES) {
        snprintf(tmp, sizeof(tmp), "\x1b[%d;%dH",
                 y,
                 edt_ctx->abs_x_offset + 8); 

        append_str_erow(NULL, &buffer, tmp, buffer.size);
        append_str_erow(NULL, &buffer, "\x1b[K", buffer.size);

        if(row->buff) {
          // colorize a copy of the row in isolation
          erow render = {0, NULL};
          append_str_erow(NULL, &render, row->buff, 0);
          check_colored_words(edt_ctx, &render);
    
          // then append the colorized result to the main buffer
          append_str_erow(NULL, &buffer, render.buff, buffer.size);
          free(render.buff);
        }
        
        curr_window->dirty_rows[y] = 0;
      }
    }

    write(STDOUT_FILENO, buffer.buff, buffer.size);
    free(buffer.buff);
}

void render_status_row(editor_ctx *edt_ctx){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

  char *mode_text = (edt_ctx->curr_opm == NORMAL ? "-- NORMAL --" : 
               (edt_ctx->curr_opm == INSERT ? "-- INSERT --" : "-- COMMAND --"));
    
  char *file_edited = (curr_window->file_modified == 1 ? "[+]" : " ");
  
  // write upper line
  move_cursor(get_win_height() - 2, 2);
  
  char buffer_u[256];
  sprintf(buffer_u, "\x1b[2K%s - %s  %s   \x1b[33m%s\x1b[39m", 
          curr_window->curr_dir,
          curr_window->curr_file,
          file_edited,
          mode_text);

  write(STDOUT_FILENO, buffer_u, strlen(buffer_u));

  // write lower line
  move_cursor(get_win_height() - 1, 2);

  char buffer_l[256];
  sprintf(buffer_l, "\x1b[2K%d,%d    window: %d/%d     %s", 
          curr_window->cx,
          curr_window->cy,
          edt_ctx->curr_window + 1,
          edt_ctx->num_windows,
          edt_ctx->clipboard == NULL ? " " : "Clipboard Loaded!");

  write(STDOUT_FILENO, buffer_l, strlen(buffer_l));
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

  //reset_screen(edt_ctx);
  
  render_dirty_rows(edt_ctx, IRC_RENDER_ALL_LINES);
  render_status_row(edt_ctx);
  render_commd_row(edt_ctx);

  if(edt_ctx->curr_opm == COMMAND) move_cursor(get_win_height(), edt_ctx->commd_row_cx + 1);
  else move_cursor(curr_window->cy, curr_window->cx + curr_window->x_offset + edt_ctx->abs_x_offset);
}

