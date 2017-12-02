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

#define FFMT__COUNT_OR_RETURN(Counter, Expression)                             \
  do {                                                                         \
    const size_t __written = Expression;                                       \
    if (ffmt_is_err(__written)) {                                              \
      return __written;                                                        \
    } else {                                                                   \
      Counter += __written;                                                    \
    }                                                                          \
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
ffmt__parse_uint(const char* start, const char* end, uint* value) {
  uint result = 0;

  const char* str = start;
  for (; ffmt__is_digit(*str) && str != end; str++) {
    result = 10 * result + (*str - '0');
  }

  *value = result;
  return str;
}

static inline const char*
ffmt__parse_width_spec(const char* start, const char* end, ffmt_pad_t* value) {
  ffmt_pad_t result = *value;

  result.align = *start++;
  uint width;
  start = ffmt__parse_uint(start, end, &width);
  result.width = width;

  if (!result.width) {
    result.align = '\0';
  }
  *value = result;

  return start;
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
  for (size_t left = length; left;) {
    size_t till_flush = out->buffer_size - out->pos;
    while (till_flush && left) {
      out->buffer[out->pos++] = *str++;

      left--;
      till_flush--;
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
