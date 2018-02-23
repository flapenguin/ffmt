#include "ffmt.internal.h"

void ffmt_flush(ffmt_out_t* out) {
  out->flush(out);
}

size_t ffmt_putc(ffmt_out_t* out, char c) {
  if (out->pos >= out->buffer_size) {
    ffmt_flush(out);
    if (out->pos >= out->buffer_size) {
      return FFMT_EFLUSH;
    }
  }

  out->buffer[out->pos++] = c;

  if (out->flags & FFMT_FLUSH_CHAR && out->flush_char == c) {
    ffmt_flush(out);
    if (out->pos >= out->buffer_size) {
      return FFMT_EFLUSH;
    }
  }

  return 1;
}

size_t ffmt_puts(ffmt_out_t* out, const char* str, size_t length) {
  length = ffmt__fix_length(str, length);

  return ffmt__puts_base(out, str, length);
}
