#include "ffmt.internal.h"

size_t ffmt_putc(const ffmt_t* self, ffmt_out_t* out, char c) {
  (void)self;

  if (out->pos >= out->buffer_size) {
    ffmt_out_flush(out);
  }

  if (out->pos >= out->buffer_size) {
    return FFMT_EFLUSH;
  }

  out->buffer[out->pos++] = c;
  return 1;
}
