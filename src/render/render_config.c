#include <stdint.h>

#include "render_config.h"

uint8_t get_render_flags(editor_ctx *edt_ctx){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
  return curr_window->render_flags;
}

uint8_t set_render_flags(editor_ctx *edt_ctx, uint8_t flag){
  window_t *curr_window = &(edt_ctx->windows[edt_ctx->curr_window]);
  curr_window->render_flags = flag;
  return curr_window->render_flags;
}

uint8_t set_render_flags_by_id(editor_ctx *edt_ctx, uint8_t flag, int id){
  for(int i = 0; i < edt_ctx->num_windows; i++){
    window_t *curr_window = &(edt_ctx->windows[i]);
    if(curr_window->window_id == id){
      curr_window->render_flags = flag;
      return curr_window->render_flags;
    }
  }
  return 0b00000000;
}
