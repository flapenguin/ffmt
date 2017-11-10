#include "ffmt.internal.h"

size_t ffmt_formatter_str(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    const ffmt_arg_t* args,
    size_t args_length,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;
  (void)spec;
  (void)spec_end;

  return ffmt_puts(out, (const char*)arg.value, FFMT_AUTO);
}
