#include <string.h>
#include <stdio.h>

#include "macros.h"

static char* macro_keywords[] = {
  "main",
  "dec",
};

void check_macros(editor_ctx *edt_ctx, erow *curr_row){
  int s = 0, e = 0;

  while(e < curr_row->size){
    char buffer[1024];
    int i = 0;
    while(curr_row->buff[e] != '!'){
      e++;
      continue;
    }
    
    while (curr_row->buff[e] != ' ' &&
       curr_row->buff[e] != '\n' &&
       curr_row->buff[e] != '\0' &&
       i < sizeof(buffer) - 1)
    {
       buffer[i++] = curr_row->buff[e++];
    }

    buffer[i] = '\0';

    // check if word is equal to buffer
    int n_keywords = sizeof(macro_keywords) / sizeof(macro_keywords[0]);
    for(int i = 0; i < n_keywords; i++){
      if(strcmp(buffer, macro_keywords[i]) == 0){
        printf("FOUND KEYWORD\n");
      }
    }
    if (curr_row->buff[e] == ' ')
      e++;

    s = e;
  }
}
