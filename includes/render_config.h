#include <stdint.h>

#include "editor_types.h"

//0
//0 
//0
//0
//0
//0
//0 < line number
//0 < content

#define RC_RENDER_ALL 0b11111111
#define RC_RENDER_CONTENT 0b00000001
#define RC_RENDER_LINE_NUMBER 0b00000010

uint8_t get_render_flags(editor_ctx *edt_ctx);
uint8_t set_render_flags(editor_ctx *edt_ctx, uint8_t flag);
uint8_t set_render_flags_by_id(editor_ctx *edt_ctx, uint8_t flag, int id);
