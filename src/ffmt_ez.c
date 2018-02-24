#include "ffmt.internal.h"

size_t ffmt_ez_write_to_string(
    char* destination,
    size_t destination_size,
    const char* format,
    const ffmt_arg_t* args) {
  return ffmt_write_to_string(
      destination,
      destination_size,
      ffmt__calculate_string_length(format),
      format,
      ffmt__calculate_args_length(args),
      args);
}

size_t
ffmt_ez_write(ffmt_out_t* out, const char* format, const ffmt_arg_t* args) {
  return ffmt_write(
      out,
      ffmt__calculate_string_length(format),
      format,
      ffmt__calculate_args_length(args),
      args);
}

size_t ffmt_ez_puts(ffmt_out_t* out, const char* str) {
  return ffmt_puts(out, str, ffmt__calculate_string_length(str));
}

size_t ffmt_ez_puts_pad(ffmt_out_t* out, const char* str, ffmt_pad_t pad) {
  return ffmt_puts_pad(out, str, ffmt__calculate_string_length(str), pad);
}
