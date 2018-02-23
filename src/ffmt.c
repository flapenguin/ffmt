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
    const char* padstr = pad.align == '^' ? pad.auxstr : pad.str;
    const size_t padstr_length =
        pad.align == '^' ? pad.auxstr_length : pad.str_length;

    if (pad.sticky) {
      const size_t part_length = right_pad % padstr_length;
      if (part_length) {
        FFMT__TRY(
            ffmt_puts(out, padstr + part_length, padstr_length - part_length));
        right_pad -= part_length;
      }
    }

    FFMT__TRY(ffmt__puts_repeat(out, padstr, padstr_length, right_pad));
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

  const char* curr = format;
  const char* plain_start;
  while (true) {
    plain_start = curr;
    while (*curr && *curr != '{') {
      curr++;
    }

    if (curr > plain_start) {
      FFMT__TRY_ADVANCE(
          total, ffmt__puts_base(out, plain_start, curr - plain_start));
    }

    if (!*curr) {
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
        total, formatter(out, arg, args, args_length, spec_start, spec_end));

    if (!position_args) {
      arg_ix++;
    }

    curr++;
  }

  return total;
}
