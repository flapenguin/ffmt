#include "ffmt.internal.h"

size_t ffmt_formatter_strz(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;
  (void)spec;
  (void)spec_end;

  ffmt_pad_t pad = FFMT_PAD_DEFAULT;
  for (; spec != spec_end; spec++) {
    switch (*spec) {
      case FFMT__PAD_CASES:
        spec =
            ffmt__parse_pad_spec(spec, spec_end, &pad, args_length, args) - 1;
        break;
    }
  }

  const char* str = (const char*)arg.value;
  return ffmt_puts_pad(out, str, strlen(str), pad);
}

size_t ffmt_formatter_bool(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;
  (void)spec;
  (void)spec_end;

  return arg.value ? ffmt_puts(out, "true", 4) : ffmt_puts(out, "false", 5);
}

size_t ffmt_formatter_char(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;
  (void)spec;
  (void)spec_end;

  return ffmt_putc(out, (char)(intptr_t)arg.value);
}

size_t ffmt_formatter_ptr(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  return ffmt_formatter_u64(out, arg, args_length, args, spec, spec_end);
}

size_t ffmt_formatter_i64(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  return ffmt_formatter_u64(out, arg, args_length, args, spec, spec_end);
}

size_t ffmt_formatter_u64(
    ffmt_out_t* out,
    const ffmt_arg_t arg,
    size_t args_length,
    const ffmt_arg_t* args,
    const char* spec,
    const char* spec_end) {
  (void)args;
  (void)args_length;

  char buffer[64];
  char* current = buffer;

  ffmt_pad_t pad = FFMT_PAD_DEFAULT;

  bool is_ptr = false;
  bool is_hex = false;
  bool is_upper_hex = false;
  bool has_prefix = false;
  for (; spec != spec_end; spec++) {
    switch (*spec) {
      case 'X':
        is_upper_hex = true;
      /* fallthrough */
      case 'x':
        is_hex = true;
        break;
      case '#':
        has_prefix = true;
        break;
      case FFMT__PAD_CASES:
        spec =
            ffmt__parse_pad_spec(spec, spec_end, &pad, args_length, args) - 1;
        break;
    }
  }

  uint64_t val;
  if (arg.formatter == ffmt_formatter_i64) {
    int64_t ival = (int64_t)(uint64_t)arg.value;
    if (ival < 0) {
      *current++ = '-';
      val = -ival;
    } else {
      val = ival;
    }
  } else {
    if (arg.formatter == ffmt_formatter_ptr) {
      is_ptr = true;
      is_hex = true;
    }
    val = (uint64_t)arg.value;
  }

  if (has_prefix) {
    *current++ = '0';
    *current++ = is_hex ? 'x' : 'd';
  }

  /* clang-format off */
  const uint digits = is_hex
    ? (is_ptr ? 16 : ffmt__u64_digits_hex(val))
    : ffmt__u64_digits_dec(val);
  /* clang-format on */

  if (is_hex) {
    ffmt__u64_to_hex_impl(val, current, digits, is_upper_hex);
  } else {
    ffmt__u64_to_dec_impl(val, current, digits);
  }

  current += digits;

  return ffmt_puts_pad(out, buffer, current - buffer, pad);
}
