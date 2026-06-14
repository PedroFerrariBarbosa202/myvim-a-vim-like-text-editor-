#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "terminal.h"
#include "screen.h"
#include "editor_types.h"
#include "windows.h"

int readKey() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void create_new_erow(editor_ctx *edt_ctx) { 
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    curr_window->rows = realloc(
        curr_window->rows,
        sizeof(erow) * (curr_window->numrows + 1)
    );

    curr_window->rows[curr_window->numrows].size = 0;
    curr_window->rows[curr_window->numrows].buff = NULL;
    curr_window->numrows++;
}

void append_str_erow(editor_ctx *edt_ctx, erow *row, char *str, int at) {

    window_t *curr_window = NULL;

    if(edt_ctx != NULL)
        curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    row->buff = realloc(
        row->buff,
        row->size + strlen(str) + 1
    );

    memmove(
        &row->buff[at + strlen(str)],
        &row->buff[at],
        row->size - at + 1
    );

    memcpy(&row->buff[at], str, strlen(str));

    row->size += strlen(str);
    row->buff[row->size] = '\0';

    if(curr_window != NULL)
        curr_window->file_modified = 1;
}

int delete_erow_char(editor_ctx *edt_ctx) {
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    if ( curr_window->cy < 1 || curr_window->cy > curr_window->numrows)
        return 1;

    erow *row = &curr_window->rows[curr_window->cy + curr_window->y_offset - 1];

    int at = curr_window->cx - 1;

    if (at <= 0 || at > row->size)
        return 1;

    memmove(
        &row->buff[at - 1],
        &row->buff[at],
        row->size - at + 1
    );

    row->size--;

    curr_window->file_modified = 1;
    return 0;
}

void insert_char_commd_erow(editor_ctx *edt_ctx, char chr) {
    int at = edt_ctx->commd_row_cx - 1;

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
    int at = edt_ctx->commd_row_cx - 1;

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
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    for(int i = 0; i < curr_window->numrows; i++){
        free(curr_window->rows[i].buff);
    }

    free(curr_window->rows);

    curr_window->rows = NULL;
    curr_window->numrows = 0;
}

void roll_down_buffers(editor_ctx *edt_ctx, int at){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    create_new_erow(edt_ctx);

    int i;
    for(i = curr_window->numrows - 1; i > at; i--){
        erow *curr = &curr_window->rows[i];
        erow *prev = &curr_window->rows[i - 1];

        curr->buff = realloc(curr->buff, prev->size + 1);

        memcpy(curr->buff, prev->buff, prev->size);
        curr->buff[prev->size] = '\0';

        curr->size = prev->size;
    }

    free(curr_window->rows[at].buff);

    curr_window->rows[at].buff = malloc(1);
    curr_window->rows[at].buff[0] = '\0';
    curr_window->rows[at].size = 0;
}

void roll_up_buffers(editor_ctx *edt_ctx, int at){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    int i;
    for(i = at; i < curr_window->numrows - 1; i++){
        erow *curr = &curr_window->rows[i];
        erow *next = &curr_window->rows[i + 1];

        curr->buff = realloc(curr->buff, next->size + 1);

        memcpy(curr->buff, next->buff, next->size);
        curr->buff[next->size] = '\0';

        curr->size = next->size;
    }

    // free last row
    free(curr_window->rows[curr_window->numrows - 1].buff);

    curr_window->rows[curr_window->numrows - 1].buff = NULL;
    curr_window->rows[curr_window->numrows - 1].size = 0;

    curr_window->numrows--;

    curr_window->rows = realloc(
        curr_window->rows,
        sizeof(erow) * curr_window->numrows
    );
}

void slice_and_down_buffer(editor_ctx *edt_ctx, int at){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    erow *curr_line = &curr_window->rows[at - 1];

    // cursor is 1-based
    int split_at = curr_window->cx - 1;

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
    erow *next_line = &curr_window->rows[at];

    next_line->size = tail_size;
    next_line->buff = malloc(tail_size + 1);

    memcpy(next_line->buff, buffer, tail_size + 1);


    free(buffer);
}


void copy_line(editor_ctx *edt_ctx, int at){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    erow *row = &curr_window->rows[at - 1];

    edt_ctx->clipboard = realloc(edt_ctx->clipboard, row->size + 1);
    memcpy(edt_ctx->clipboard, row->buff, row->size);

    edt_ctx->clipboard[row->size] = '\0';
}

void paste_line(editor_ctx *edt_ctx, int at_y, int at_x){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);

    if(edt_ctx->clipboard == NULL) return;

    erow *row = &curr_window->rows[at_y - 1];

    append_str_erow(edt_ctx, row, edt_ctx->clipboard, at_x - 1);

    free(edt_ctx->clipboard);
    edt_ctx->clipboard = NULL;
}

int delete_erow_buffer(editor_ctx *edt_ctx, int at){
    window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
    erow *row = &curr_window->rows[at - 1];

    row->buff = NULL;
    row->size = 0;

    roll_up_buffers(edt_ctx, at - 1);
}
