#include "ffmt.internal.h"

static inline bool ffmt__is_digit(char c) { return '0' <= c && c <= '9'; }

static inline const char* ffmt__strchr(const char* str, const char* breaks) {
  for (; *str; str++) {
    for (const char* b = breaks; *b; b++) {
      if (*str == *b) {
        return str;
      }
    }
  }

  return 0;
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

  while (format[i]) {
    while (format[i] && format[i] != '{') {
      i++;
    }

    ffmt_puts(out, &format[plain_start], i - plain_start);

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

    size_t r = formatter(out, arg, args, args_length, spec_start, spec_end);
    if (ffmt_is_err(r)) {
      return r;
    }
    total += r;

    if (!position_args) {
      arg_ix++;
    }

    i++;
    plain_start = i;
  }

  ffmt_puts(out, &format[plain_start], i - plain_start);

  return total;
}
