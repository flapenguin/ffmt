#include "ffmt.internal.h"

static void nop_flush(ffmt_out_t* self) {
  (void)self;
}

size_t ffmt_write_to_string(
    char* destination,
    size_t destination_size,
    const char* format,
    const ffmt_arg_t* args,
    size_t args_length) {
  /* clang-format off */
  ffmt_out_t out = {
    .buffer = (void*)destination,
    .buffer_size = destination_size,
    .pos = 0,
    .flush = nop_flush
  };
  /* clang-format on */

  return ffmt_write(&out, format, args, args_length);
}
