#ifndef EDITOR_TYPES
#define EDITOR_TYPES

#include <stdint.h>

#define MAX_ROWS 9999
#define LINE_MAX_SIZE 524

#define ARROW_UP 1000
#define ARROW_DOWN 1001
#define ARROW_LEFT 1002
#define ARROW_RIGHT 1003

typedef struct {
  int size;
  char *buff;
} erow;

typedef enum { 
  INSERT,
  NORMAL,
  COMMAND,
}op_mode;

typedef struct{
  int window_id;
  int cx, cy;
  erow *rows;

  int numrows;
  int x_offset;
  int y_offset;

  // dirty line tracker
  int dirty_rows[MAX_ROWS + 1];

  // flags to configure rendering 
  uint8_t render_flags;

  // file management logic
  char *curr_dir;
  char *curr_file;
  int file_modified;
}window_t;


typedef struct{
  int commd_row_cx;
  int abs_x_offset;
  
  erow commd_row;
  op_mode curr_opm;

  window_t *windows;
  int num_windows;
  int curr_window;
  int window_id_count;

  // clipboard
  char *clipboard;
}editor_ctx;

#endif
