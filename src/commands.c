#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor_types.h"
#include "screen.h"
#include "terminal.h"
#include "file_manager.h"

char **tokenize_commd(char *commd){
    char **buffer = malloc(sizeof(char*) * 64);

    int i = 0;

    char *token = strtok(commd, " ");

    while(token != NULL){
        buffer[i++] = token;

        token = strtok(NULL, " ");
    }

    buffer[i] = NULL;

    return buffer;
}

void exec_command(editor_ctx *edt_ctx, char *commd){
  char **buffer;

  // tokenize text
  buffer = tokenize_commd(commd);
  
  // go to absolute line n
  if(strcmp(buffer[0], "l") == 0){
    if(buffer[1] == NULL) return;
    int new_line = strtol(buffer[1], NULL, 10);
     
    if(new_line > (get_win_height() - 2) + edt_ctx->y_offset){
      int overflow = new_line - ((get_win_height() - 2) + edt_ctx->y_offset);
      roll_screen(edt_ctx, overflow);
      edt_ctx->cy = get_win_height() - 2;
    }else if(new_line < edt_ctx->y_offset){
      int underflow = new_line - edt_ctx->y_offset - 1;
      roll_screen(edt_ctx, underflow);
      edt_ctx->cy = 1;
    }else edt_ctx->cy = new_line - edt_ctx->y_offset;

    edt_ctx->cx = 1;
  }

  // load a file into memory
  if(strcmp(buffer[0], "f") == 0){
    if(buffer[1] == NULL) return;
      load_buffers_from_file(edt_ctx, buffer[1]);
  }

  // save and quit
  if(strcmp(buffer[0], "wq") == 0){
    save_file_from_buffers(edt_ctx, NULL);

    // clear screen
    write(STDOUT_FILENO, "\x1b[2J", 4);

    exit(0);
  }

  // save a file
  if(strcmp(buffer[0], "w") == 0){
    char *path = buffer[1] == NULL ? NULL : buffer[1];
    save_file_from_buffers(edt_ctx, path);
  }

  // save a directory
  if(strcmp(buffer[0], "dir") == 0){
    char *path = buffer[1] == NULL ? NULL : buffer[1];
    set_current_directory(edt_ctx, path);
  }

  // quit program
  if(strcmp(buffer[0], "q") == 0){
    // clear screen
    write(STDOUT_FILENO, "\x1b[2J", 4);

    exit(0);
  }
}


