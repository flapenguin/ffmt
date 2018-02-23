#ifndef FFMT_INTERNAL_H__
#define FFMT_INTERNAL_H__

#include "ffmt.h"

typedef unsigned int uint;

uint ffmt__u64_digits_dec(uint64_t value);
void ffmt__u64_to_dec_impl(uint64_t value, char* const buffer, size_t digits);

uint ffmt__u64_digits_hex(uint64_t value);
void ffmt__u64_to_hex_impl(
    uint64_t value,
    char* const buffer,
    size_t digits,
    bool upper);

/* Utils. */

/* clang-format off */
#define FFMT__PAD_CASES '<': case '>': case '^'
/* clang-format on */

#define FFMT__TRY(Expression)                                                  \
  do {                                                                         \
    const size_t __written = Expression;                                       \
    if (ffmt_is_err(__written)) {                                              \
      return __written;                                                        \
    }                                                                          \
  } while (0)

#define FFMT__TRY_EX(Out, Expression)                                          \
  do {                                                                         \
    Out = Expression;                                                          \
    if (ffmt_is_err(Out)) {                                                    \
      return Out;                                                              \
    }                                                                          \
  } while (0)

#define FFMT__TRY_ADVANCE(Counter, Expression)                                 \
  do {                                                                         \
    size_t __written;                                                          \
    FFMT__TRY_EX(__written, Expression);                                       \
    Counter += __written;                                                      \
  } while (0)

static inline bool ffmt__is_digit(char c) {
  return '0' <= c && c <= '9';
}

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

static inline const char*
ffmt__parse_u64(const char* start, const char* end, uint64_t* value) {
  uint64_t result = 0;

  const char* str = start;
  for (; ffmt__is_digit(*str) && str != end; str++) {
    result = 10 * result + (*str - '0');
  }

  *value = result;
  return str;
}

static inline const char* ffmt__parse_pad_spec(
    const char* start,
    const char* end,
    ffmt_pad_t* value,
    size_t args_length,
    const ffmt_arg_t* args) {
  ffmt_pad_t result = *value;

  result.align = *start++;

  if (start != end && *start == '*') {
    result.sticky = true;
    start++;
  }

  if (start != end && *start == '(') {
    start++;
    result.str = start;
    while (*start != ')' && start != end) {
      start++;
    }

    if (start == end) {
      return start;
    }

    result.str_length = start - result.str;
    start++;
  }

  if (result.align == '^') {
    if (start != end && *start == '(') {
      start++;
      result.auxstr = start;
      while (*start != ')' && start != end) {
        start++;
      }

      if (start == end) {
        return start;
      }

      result.auxstr_length = start - result.auxstr;
      start++;
    } else {
      result.auxstr = result.str;
      result.auxstr_length = result.str_length;
    }
  }

  bool ref = false;
  if (*start == '@') {
    ref = true;
    start++;
  }

  start = ffmt__parse_u64(start, end, &result.width);
  if (ref) {
    if (result.width < args_length) {
      result.width = (uint64_t)args[result.width].value;
    } else {
      result.width = 0;
    }
  }

  if (!result.width) {
    result.align = '\0';
  }
  *value = result;

  return start;
}

static inline size_t
ffmt__fix_args_length(const ffmt_arg_t* args, size_t length) {
  if (length != FFMT_AUTO) {
    return length;
  }

  length = 0;
  while (args[length].formatter != FFMT_FORMATTER_LAST) {
    length++;
  }

  return length;
}

static inline size_t ffmt__fix_length(const char* str, size_t length) {
  if (length != FFMT_AUTO) {
    return length;
  }

  const char* x = str;
  while (*x) {
    x++;
  }

  return x - str;
}

static inline size_t
ffmt__puts_base(ffmt_out_t* out, const char* str, size_t length) {
  const int flush_char = out->flags & FFMT_FLUSH_CHAR ? out->flush_char : -1;

  for (size_t left = length; left;) {
    size_t till_flush = out->buffer_size - out->pos;
    while (till_flush && left) {
      const char c = *str++;
      out->buffer[out->pos++] = c;

      left--;
      if (c == flush_char) {
        till_flush = 0;
      } else {
        till_flush--;
      }
    }

    if (!till_flush) {
      ffmt_flush(out);
    }
  }

  return length;
}

static inline size_t ffmt__puts_repeat(
    ffmt_out_t* out,
    const char* str,
    size_t length,
    size_t desired) {
  size_t rest = desired;
  while (rest) {
    size_t written;

    FFMT__TRY_EX(
        written, ffmt__puts_base(out, str, rest < length ? rest : length));

    rest -= written;
  }

  return desired;
}

#endif /* FFMT_INTERNAL_H__ */
