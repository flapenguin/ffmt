#include "ffmt.internal.h"

void ffmt_flush(ffmt_out_t* out) {
  out->flush(out);
}

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
  length = ffmt__fix_length(str, length);

  return ffmt__puts_base(out, str, length);
}

extern size_t
ffmt_puts_pad(ffmt_out_t* out, const char* str, size_t length, ffmt_pad_t pad) {
  if (!pad.align || !pad.width || pad.width == length) {
    return ffmt_puts(out, str, length);
  }

  length = ffmt__fix_length(str, length);

  size_t left_pad = 0;
  size_t right_pad = 0;

  switch (pad.align) {
    case '<':
      if (pad.width < length) {
        length = pad.width;
      } else {
        right_pad = pad.width - length;
      }
      break;
    case '>':
      if (pad.width < length) {
        str += (length - pad.width);
        length = pad.width;
      } else {
        left_pad = pad.width - length;
      }
      break;
    case '^':
      if (pad.width < length) {
        str += (length - pad.width) / 2;
        length = pad.width;
      } else {
        left_pad = (pad.width - length) / 2;
        right_pad = pad.width - length - left_pad;
      }
      break;
    default:
      return FFMT_EALIGN;
  }

  if (left_pad) {
    FFMT__TRY(ffmt__puts_repeat(out, pad.str, pad.str_length, left_pad));
  }

  FFMT__TRY(ffmt__puts_base(out, str, length));

  if (right_pad) {
    FFMT__TRY(ffmt__puts_repeat(out, pad.str, pad.str_length, right_pad));
  }

  return pad.width;
}

size_t ffmt_write(
    ffmt_out_t* out,
    const char* format,
    const ffmt_arg_t* args,
    size_t args_length) {
  size_t total = 0;
  bool position_args = false;
  size_t arg_ix = 0;
  size_t plain_start = 0;
  size_t i = 0;

  while (true) {
    while (format[i] && format[i] != '{') {
      i++;
    }

    if (i > plain_start) {
      FFMT__COUNT_OR_RETURN(
          total, ffmt_puts(out, &format[plain_start], i - plain_start));
    }

    if (!format[i]) {
      break;
    }

    const char* spec_start = 0;
    const char* spec_end = 0;
    const char* formatter_start = 0;
    const char* formatter_end = 0;

    i++;

    if (ffmt__is_digit(format[i])) {
      position_args = true;
      arg_ix = 0;
      do {
        arg_ix = arg_ix * 10 + (format[i] - '0');
        i++;
      } while (ffmt__is_digit(format[i]));
    } else {
      if (position_args) {
        return FFMT_EFORMAT;
      }
    }

    if (format[i] == ':') {
      i++;
      spec_start = &format[i];
      spec_end = ffmt__strchr(spec_start, ":}");
      if (!spec_end) {
        return FFMT_EFORMAT;
      }

      i = spec_end - format;

      if (format[i] == ':') {
        formatter_start = spec_start;
        formatter_end = spec_end;

        i++;
        spec_start = &format[i];
        spec_end = ffmt__strchr(spec_start, ":}");
        if (!spec_end) {
          return FFMT_EFORMAT;
        }

        i = spec_end - format;
      }
    }

    if (format[i] != '}') {
      return FFMT_EFORMAT;
    }

    if (arg_ix >= args_length) {
      return FFMT_EARGLEN;
    }

    const ffmt_arg_t arg = args[arg_ix];
    ffmt_formatter_t formatter = arg.formatter;

    if (formatter_end != formatter_start) {
      // TODO
    }

    if (!formatter) {
      return FFMT_ENOFORMATTER;
    }

    FFMT__COUNT_OR_RETURN(
        total, formatter(out, arg, args, args_length, spec_start, spec_end));

    if (!position_args) {
      arg_ix++;
    }

    i++;
    plain_start = i;
  }

  return total;
}
