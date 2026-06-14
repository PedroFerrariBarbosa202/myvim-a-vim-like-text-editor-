#ifndef EDITOR_TYPES
#define EDITOR_TYPES

#define MAX_ROWS 1024

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
  int cx, cy;
  erow *rows;

  int numrows;
  int x_offset;
  int y_offset;
  int x_padding;

  // file management logic
  char *curr_dir;
  char *curr_file;
  int file_modified;
}window_t;


typedef struct{
  int commd_row_cx;
  
  erow commd_row;
  op_mode curr_opm;

  window_t *windows;
  int num_windows;
  int curr_window;

  // clipboard
  char *clipboard;
}editor_ctx;
#endif
