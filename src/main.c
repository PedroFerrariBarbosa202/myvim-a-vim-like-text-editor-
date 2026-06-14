#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "screen.h"
#include "terminal.h"
#include "modes.h"
#include "file_manager.h"
#include "editor_types.h"
#include "windows.h"

editor_ctx edt_ctx = {
  .commd_row_cx = 1,

  .curr_opm = NORMAL,
  .windows = NULL,
  .num_windows = 0,
  .curr_window = 0,
  .clipboard = NULL,
};

int main(int argc, char *argv[]){
  enableRawMode();
  atexit(disableRawMode);

  reset_screen(&edt_ctx);
  create_window(&edt_ctx);

  // check if user wants to open a file with argvs
  if(argc > 1) load_buffers_from_file(&edt_ctx, argv[1]);

  while(1){
    render_screen(&edt_ctx);
    char key = readKey();

    // change mode if ESCAPE is pressed
    if (key == '\x1b') {
      // reset mouse to saved cx/cy postions
      if(edt_ctx.curr_opm == COMMAND) {
        edt_ctx.commd_row.buff = NULL;
      }
      edt_ctx.commd_row_cx = 1;

      // change mode
      edt_ctx.curr_opm = NORMAL;
      continue;
    }
  
    if(edt_ctx.curr_opm == NORMAL) handle_normal_mode(&edt_ctx, key);
    else if(edt_ctx.curr_opm == INSERT) handle_insert_mode(&edt_ctx, key);
    else if(edt_ctx.curr_opm == COMMAND) handle_command_mode(&edt_ctx, key);

    // clamp cx and cy values relative to screen    
    window_t *curr_window = &(edt_ctx.windows[edt_ctx.curr_window]);

    int filerow = curr_window->cy + curr_window->y_offset - 1;
    int curr_line_len = 0;

    if(filerow >= 0 && filerow < curr_window->numrows && curr_window->rows != NULL)
      curr_line_len = curr_window->rows[filerow].size;

    if(curr_window->cx < 1) curr_window->cx = 1;
    if(curr_window->cx > curr_line_len + 1 && edt_ctx.curr_opm != COMMAND) curr_window->cx = curr_line_len + 1;
    if(curr_window->cy < 1) curr_window->cy = 1;
    if(curr_window->cy > get_win_height() - 2) curr_window->cy = get_win_height() - 2;
  }

  return 0;
}



