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

size_t ffmt_formatter_i64(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    const ffmt_arg_t* args,
    size_t args_length,
    const char* spec,
    const char* spec_end) {
  return ffmt_formatter_u64(out, arg, args, args_length, spec, spec_end);
}

size_t ffmt_formatter_u64(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    const ffmt_arg_t* args,
    size_t args_length,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;

  uint length = 0;

  bool is_hex = false;
  bool is_upper_hex = false;
  bool has_prefix = false;
  for (; spec != spec_end; spec++) {
    if (*spec == '#') has_prefix = true;
    if (*spec == 'x') is_hex = true;
    if (*spec == 'X') {
      is_hex = true;
      is_upper_hex = true;
    }
  }

  uint64_t val;
  if (arg.formatter == ffmt_formatter_i64) {
    int64_t ival = (int64_t)(uint64_t)arg.value;
    if (ival < 0) {
      length += 1;
      ffmt_putc(out, '-');
      val = -ival;
    } else {
      val = ival;
    }
  } else {
    val = (uint64_t)arg.value;
  }

  if (has_prefix) {
    length += 2;
    ffmt_puts(out, "0x", 2);
  }

  const uint digits =
      is_hex ? ffmt__u64_digits_hex(val) : ffmt__u64_digits_dec(val);

  if (out->pos + digits >= out->buffer_size) {
    ffmt_flush(out);
    if (out->pos + digits >= out->buffer_size) {
      return FFMT_EFLUSH;
    }
  }

  char* const start = (char*)(out->buffer + out->pos);
  if (is_hex) {
    ffmt__u64_to_hex_impl(val, start, digits, is_upper_hex);
  } else {
    ffmt__u64_to_dec_impl(val, start, digits);
  }

  out->pos += digits;
  return length + digits;
}
