#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "editor_types.h"
#include "buffer.h"

void set_current_directory(editor_ctx *edt_ctx, char *new_dir){
    edt_ctx->curr_dir = realloc(
        edt_ctx->curr_dir,
        strlen(new_dir) + 1
    );

    strcpy(edt_ctx->curr_dir, new_dir);
}

void set_current_file(editor_ctx *edt_ctx, char *new_file){
    edt_ctx->curr_file = realloc(
        edt_ctx->curr_file,
        strlen(new_file) + 1
    );

    strcpy(edt_ctx->curr_file, new_file);
}

int load_buffers_from_file(editor_ctx *edt_ctx, char *file_name){
    FILE *fp;
    int c;
    int curr_row = 0;

    char curr_dir[256];
    if (edt_ctx->curr_dir != NULL) snprintf(curr_dir, sizeof(curr_dir), "%s/%s",
                                            edt_ctx->curr_dir, file_name);
    else snprintf(curr_dir, sizeof(curr_dir), "%s", file_name);
    
    fp = fopen(curr_dir, "r");

    if(fp == NULL) return 1;

    // set current file
    set_current_file(edt_ctx, file_name);

    clear_rows(edt_ctx);
    create_new_erow(edt_ctx);

    while((c = fgetc(fp)) != EOF){
        if(c == '\n'){
            curr_row++;
            create_new_erow(edt_ctx);
            continue;
        }

        char tmp[2] = {c, '\0'};

        append_str_erow(
            NULL,
            &edt_ctx->rows[curr_row],
            tmp,
            edt_ctx->rows[curr_row].size
        );
    }

    fclose(fp);

    edt_ctx->cy = 1;
    edt_ctx->cy = 1;
    edt_ctx->y_offset = 0;
    return 0;
}

int save_file_from_buffers(editor_ctx *edt_ctx, char *new_name){
    FILE *fp;
    int curr_row = 0;

    char curr_dir[256];
    sprintf(curr_dir, "%s/%s", 
            edt_ctx->curr_dir == NULL ? "" : edt_ctx->curr_dir,
            new_name == NULL ? edt_ctx->curr_file : new_name);

    fp = fopen(curr_dir, "w");

    if(fp == NULL) return 1;
   
    // change current file if a new name is requested
    if(new_name != NULL) set_current_file(edt_ctx, new_name);

    while(curr_row < edt_ctx->numrows){
        char *buffer = edt_ctx->rows[curr_row].buff;

        if(buffer != NULL)
            fputs(buffer, fp);
            fputs("\n", fp);
        curr_row++;
    }

    fclose(fp);
    
    edt_ctx->file_modified = 0;
    return 0;
}

