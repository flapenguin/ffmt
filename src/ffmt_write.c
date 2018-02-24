#include "ffmt.internal.h"

size_t ffmt_write(
    ffmt_out_t* out,
    size_t format_length,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args) {
  size_t total = 0;
  bool position_args = false;
  size_t arg_ix = 0;

  const char* curr = format;
  const char* end = format + format_length;
  const char* plain_start;
  while (true) {
    plain_start = curr;
    while (curr < end && *curr != '{') {
      curr++;
    }

    if (curr > plain_start) {
      FFMT__TRY_ADVANCE(
          total, ffmt__puts_base(out, plain_start, curr - plain_start));
    }

    if (curr >= end) {
      break;
    }

    /* Skip '{'. */
    curr++;

    const char* spec_start = 0;
    const char* spec_end = 0;

    if (ffmt__is_digit(*curr)) {
      position_args = true;
      /* 4 digits seems reasonable amount for argument reference */
      curr = ffmt__parse_u64(curr, curr + 4, &arg_ix);
    } else if (position_args) {
      return FFMT_EFORMAT;
    }

    if (*curr == ':') {
      curr++;
      spec_start = curr;
      spec_end = ffmt__strchr(spec_start, ":}");
      if (!spec_end) {
        return FFMT_EFORMAT;
      }

      curr = spec_end;
    }

    if (*curr != '}') {
      return FFMT_EFORMAT;
    }

    if (arg_ix >= args_length) {
      return FFMT_EARGLEN;
    }

    const ffmt_arg_t arg = args[arg_ix];
    ffmt_formatter_t formatter = arg.formatter;

    if (!formatter) {
      return FFMT_ENOFORMATTER;
    }

    FFMT__TRY_ADVANCE(
        total, formatter(out, arg, args_length, args, spec_start, spec_end));

    if (!position_args) {
      arg_ix++;
    }

    curr++;
  }

  return total;
}
