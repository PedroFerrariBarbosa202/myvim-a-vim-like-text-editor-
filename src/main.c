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

editor_ctx edt_ctx = {
  .cx = 1,
  .cy = 1,

  .saved_cx = 1,
  .saved_cy = 1,

  .rows = NULL,
  .numrows = 0,

  .x_offset = 7,
  .y_offset = 0,

  .curr_opm = NORMAL,

  .curr_dir = NULL,
  .curr_file = NULL,
  .file_modified = 0,

  .clipboard = NULL,
};

int main(int argc, char *argv[]){
  atexit(disableRawMode);

  reset_screen();

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

        edt_ctx.cx = edt_ctx.saved_cx;
        edt_ctx.cy = edt_ctx.saved_cy;
      }
      
      // change mode
      edt_ctx.curr_opm = (edt_ctx.curr_opm == NORMAL ? INSERT : NORMAL);
      continue;
    }
  
    if(edt_ctx.curr_opm == NORMAL) handle_normal_mode(&edt_ctx, key);
    else if(edt_ctx.curr_opm == INSERT) handle_insert_mode(&edt_ctx, key);
    else if(edt_ctx.curr_opm == COMMAND) handle_command_mode(&edt_ctx, key);

    // clamp mouse value
    int curr_line_len = edt_ctx.rows[edt_ctx.cy + edt_ctx.y_offset - 1].size;

    if(edt_ctx.cx < 1) edt_ctx.cx = 1;
    if(edt_ctx.cx > curr_line_len + 1 && edt_ctx.curr_opm != COMMAND) edt_ctx.cx = curr_line_len + 1;
    if(edt_ctx.cy < 1) edt_ctx.cy = 1;
    if(edt_ctx.cy > get_win_height() - 2) edt_ctx.cy = get_win_height() - 2;
  }

  return 0;
}



