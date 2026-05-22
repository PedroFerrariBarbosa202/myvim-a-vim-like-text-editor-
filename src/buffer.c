#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "terminal.h"
#include "screen.h"
#include "editor_types.h"

int readKey() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void create_new_erow(editor_ctx *edt_ctx) { 
    edt_ctx->rows = realloc(
        edt_ctx->rows,
        sizeof(erow) * (edt_ctx->numrows + 1)
    );

    edt_ctx->rows[edt_ctx->numrows].size = 0;
    edt_ctx->rows[edt_ctx->numrows].buff = NULL;
    edt_ctx->numrows++;
}

void append_str_erow(editor_ctx *edt_ctx, erow *row, char *str, int at) {
    row->buff = realloc(
        row->buff,
        row->size + strlen(str) + 1);

    memmove(
        &row->buff[at + strlen(str)],
        &row->buff[at],
        row->size - at + 1
    );

    // copy string
    memcpy(&row->buff[at], str, strlen(str));

    row->size += strlen(str);
    row->buff[row->size] = '\0';
    
    if(edt_ctx != NULL) edt_ctx->file_modified = 1;
}

int delete_erow_char(editor_ctx *edt_ctx) {
    if (edt_ctx->cy < 1 || edt_ctx->cy > edt_ctx->numrows)
        return 1;

    erow *row = &edt_ctx->rows[edt_ctx->cy + edt_ctx->y_offset - 1];

    int at = edt_ctx->cx - 1;

    if (at <= 0 || at > row->size)
        return 1;

    memmove(
        &row->buff[at - 1],
        &row->buff[at],
        row->size - at + 1
    );

    row->size--;

    edt_ctx->file_modified = 1;
    return 0;
}

void insert_char_commd_erow(editor_ctx *edt_ctx, char chr) {
    int at = edt_ctx->cx - 1;

    if (at < 0)
        at = 0;

    if (at > edt_ctx->commd_row.size)
        at = edt_ctx->commd_row.size;

    edt_ctx->commd_row.buff = realloc(
        edt_ctx->commd_row.buff,
        edt_ctx->commd_row.size + 2
    );

    memmove(
        &edt_ctx->commd_row.buff[at + 1],
        &edt_ctx->commd_row.buff[at],
        edt_ctx->commd_row.size - at + 1
    );

    edt_ctx->commd_row.buff[at] = chr;

    edt_ctx->commd_row.size++;
    edt_ctx->commd_row.buff[edt_ctx->commd_row.size] = '\0';
}

int delete_char_commd_erow(editor_ctx *edt_ctx) {
    int at = edt_ctx->cx - 1;

    if (at <= 0 || at > edt_ctx->commd_row.size)
        return 1;

    memmove(
        &edt_ctx->commd_row.buff[at - 1],
        &edt_ctx->commd_row.buff[at],
        edt_ctx->commd_row.size - at + 1
    );

    edt_ctx->commd_row.size--;

    return 0;
}

void clear_rows(editor_ctx *edt_ctx){
    for(int i = 0; i < edt_ctx->numrows; i++){
        free(edt_ctx->rows[i].buff);
    }

    free(edt_ctx->rows);

    edt_ctx->rows = NULL;
    edt_ctx->numrows = 0;
}

void roll_down_buffers(editor_ctx *edt_ctx, int at){
    create_new_erow(edt_ctx);

    int i;
    for(i = edt_ctx->numrows - 1; i > at; i--){
        erow *curr = &edt_ctx->rows[i];
        erow *prev = &edt_ctx->rows[i - 1];

        curr->buff = realloc(curr->buff, prev->size + 1);

        memcpy(curr->buff, prev->buff, prev->size);
        curr->buff[prev->size] = '\0';

        curr->size = prev->size;
    }

    free(edt_ctx->rows[at].buff);

    edt_ctx->rows[at].buff = malloc(1);
    edt_ctx->rows[at].buff[0] = '\0';
    edt_ctx->rows[at].size = 0;
}

void roll_up_buffers(editor_ctx *edt_ctx, int at){
    int i;
    for(i = at; i < edt_ctx->numrows - 1; i++){
        erow *curr = &edt_ctx->rows[i];
        erow *next = &edt_ctx->rows[i + 1];

        curr->buff = realloc(curr->buff, next->size + 1);

        memcpy(curr->buff, next->buff, next->size);
        curr->buff[next->size] = '\0';

        curr->size = next->size;
    }

    // free last row
    free(edt_ctx->rows[edt_ctx->numrows - 1].buff);

    edt_ctx->rows[edt_ctx->numrows - 1].buff = NULL;
    edt_ctx->rows[edt_ctx->numrows - 1].size = 0;

    edt_ctx->numrows--;

    edt_ctx->rows = realloc(
        edt_ctx->rows,
        sizeof(erow) * edt_ctx->numrows
    );
}

void slice_and_down_buffer(editor_ctx *edt_ctx, int at){
    erow *curr_line = &edt_ctx->rows[at - 1];

    // cursor is 1-based
    int split_at = edt_ctx->cx - 1;

    if(split_at < 0)
        split_at = 0;

    if(split_at > curr_line->size)
        split_at = curr_line->size;

    int tail_size = curr_line->size - split_at;

    // save tail before reallocations
    char *buffer = malloc(tail_size + 1);

    memcpy(buffer,
           curr_line->buff + split_at,
           tail_size);

    buffer[tail_size] = '\0';

    // shrink current line
    curr_line->size = split_at;

    curr_line->buff = realloc(
        curr_line->buff,
        curr_line->size + 1
    );

    curr_line->buff[curr_line->size] = '\0';

    // make space for new row
    roll_down_buffers(edt_ctx, at);

    // reacquire pointer after possible realloc
    erow *next_line = &edt_ctx->rows[at];

    next_line->size = tail_size;
    next_line->buff = malloc(tail_size + 1);

    memcpy(next_line->buff, buffer, tail_size + 1);


    free(buffer);
}


void copy_line(editor_ctx *edt_ctx, int at){
    erow *row = &edt_ctx->rows[at - 1];

    edt_ctx->clipboard = realloc(edt_ctx->clipboard, row->size + 1);
    memcpy(edt_ctx->clipboard, row->buff, row->size);

    edt_ctx->clipboard[row->size] = '\0';
}

void paste_line(editor_ctx *edt_ctx, int at_y, int at_x){
    if(edt_ctx->clipboard == NULL) return;

    erow *row = &edt_ctx->rows[at_y - 1];

    append_str_erow(edt_ctx, row, edt_ctx->clipboard, at_x - 1);

    edt_ctx->clipboard = NULL;
}

int delete_erow_buffer(editor_ctx *edt_ctx, int at){
    erow *row = &edt_ctx->rows[at - 1];

    row->buff = NULL;
    row->size = 0;

    roll_up_buffers(edt_ctx, at - 1);
}
