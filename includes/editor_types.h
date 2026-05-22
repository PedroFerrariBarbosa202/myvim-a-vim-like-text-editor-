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
  int saved_cx, saved_cy;
  erow *rows;

  int numrows;
  int x_offset;
  int y_offset;

  erow commd_row;
  op_mode curr_opm;
  
  // file management logic
  char *curr_dir;
  char *curr_file;
  int file_modified;

  // clipboard;
  char *clipboard;
}editor_ctx;

#endif
