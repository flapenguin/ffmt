#include "ffmt.internal.h"

static void nop_flush(ffmt_out_t* self) {
  (void)self;
}

size_t ffmt_write_to_string(
    char* destination,
    size_t destination_size,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args) {
  /* clang-format off */
  ffmt_out_t out = {
    .buffer = (void*)destination,
    .buffer_size = destination_size,
    .pos = 0,
    .flush = nop_flush
  };
  /* clang-format on */

  size_t length = ffmt_write(&out, format, args_length, args);
  if (length + 1 > destination_size) {
    return FFMT_ESMALLBUF;
  }

  destination[length] = '\0';
  return length + 1;
}
