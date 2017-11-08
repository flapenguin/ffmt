#include "ffmt.internal.h"

size_t ffmt_putc(ffmt_out_t* out, char c) {
  if (out->pos >= out->buffer_size) {
    ffmt_flush(out);
  }

  if (out->pos >= out->buffer_size) {
    return FFMT_EFLUSH;
  }

  out->buffer[out->pos++] = c;
  return 1;
}

size_t ffmt_puts(ffmt_out_t* out, const char* str, size_t length) {
  if (length == FFMT_AUTO) {
    const char* x = str;
    while (*x) {
      x++;
    }

    length = x - str;
  }

  const char* start = str;
  while (length) {
    size_t till_flush = out->buffer_size - out->pos;
    while (till_flush && length) {
      out->buffer[out->pos++] = *str++;

      length--;
      till_flush--;
    }

    if (!till_flush) {
      ffmt_flush(out);
    }
  }

  return str - start;
}
