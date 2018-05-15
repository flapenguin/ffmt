#ifndef FFMT_H__
#define FFMT_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* NOTICE: FFMT__* is non-public stuff. */

#define FFMT__STR2(X) #X
#define FFMT__STR(X) FFMT__STR2(X)

#define FFMT_VER_MAJOR 0
#define FFMT_VER_MINOR 1
#define FFMT_VER_PATCH 0

/* clang-format off */
#define FFMT_VER ( \
    FFMT_VER_MAJOR * 0x10000 + \
    FFMT_VER_MINOR * 0x100 + \
    FFMT_VER_PATCH \
  )

#define FFMT_VER_STR ( \
    FFMT__STR(FFMT_VER_MAJOR) "." \
    FFMT__STR(FFMT_VER_MINOR) "." \
    FFMT__STR(FFMT_VER_PATCH) \
  )
/* clang-format on */

#define FFMT__EZ_FORMATTER_LAST ((ffmt_formatter_t)-1)
#define FFMT_EZ_ARG_LAST ((ffmt_arg_t){FFMT__EZ_FORMATTER_LAST})

/* See FFMT_FOREACH_ERROR for descriptions. */
#define FFMT_EFLUSH ((size_t)-1)
#define FFMT_EFORMAT ((size_t)-2)
#define FFMT_EARGLEN ((size_t)-3)
#define FFMT_ENOFORMATTER ((size_t)-4)
#define FFMT_ESMALLBUF ((size_t)-5)
#define FFMT_EALIGN ((size_t)-6)

/* clang-format off */
#define FFMT_FOREACH_ERROR(Fn) \
    Fn(FFMT_EFLUSH, "Cannot flush.") \
    Fn(FFMT_EFORMAT, "Bad format string.") \
    Fn(FFMT_EARGLEN, "Bad argument index.") \
    Fn(FFMT_ENOFORMATTER, "Missing formatter.") \
    Fn(FFMT_ESMALLBUF, "Buffer is too small.") \
    Fn(FFMT_EALIGN, "Bad alignment option.")
/* clang-format on */

/* clang-format off */
#define FFMT_FORMATTER_DECL(Name) \
  size_t Name( \
    ffmt_out_t* out, \
    const ffmt_arg_t arg, \
    size_t args_length, \
    const ffmt_arg_t* args, \
    const char* spec, \
    const char* spec_end)
/* clang-format on */

#define FFMT_FLUSH_CHAR 0x1

/* Forward declarations */
typedef struct ffmt_out_t ffmt_out_t;
typedef struct ffmt_arg_t ffmt_arg_t;
typedef struct ffmt_pad_t ffmt_pad_t;
typedef FFMT_FORMATTER_DECL((*ffmt_formatter_t));

/* ============================================================================
 * API structures
 *
 * Structures can be used from C code.
 * ============================================================================
 */

/** Output abstraction. */
struct ffmt_out_t {
  /** Buffer. */
  uint8_t* buffer;
  /** Buffer size. */
  size_t buffer_size;
  /** Current position in buffer. */
  size_t pos;
  /** Flags for ouput. Only FFMT_FLUSH_CHAR is supported for now. */
  uint64_t flags;

  /** Callback to flush the buffer. */
  void (*flush)(ffmt_out_t* self);

  /** FFMT_FLUSH_CHAR makes buffer flush each time `flush_char` is written. */
  char flush_char;
};

/** Formatting argument. */
struct ffmt_arg_t {
  /** Function to do the actual formatting. */
  const ffmt_formatter_t formatter;
  /** Value casted to void*. */
  const void* value;
};

/** Padding information. */
struct ffmt_pad_t {
  /** Width to pad original string to. */
  size_t width;
  /** String to pad original string with. */
  const char* str;
  size_t str_length;
  /** Auxillary string to pad original string with. */
  const char* auxstr;
  size_t auxstr_length;

  /**
   * Alignment:
   *  '<' - pad the end of the string with `str`
   *  '>' - pad the beginning of the string with `str`
   *  '^' - center the string and pad beginning with `str` and end with `auxstr`
   */
  char align;

  /**
   * Adjust pad string so they start/end on the beggining of the original
   * string.
   * E.g. padding 'FOOBAR' with '0123456' to 10 characters:
   *    sticky=false  0123FOOBAR
   *    sticky=true   3456FOOBAR
   */
  bool sticky;
};

/* clang-format off */
#define FFMT_PAD_DEFAULT (ffmt_pad_t){ 0, " ", 1, "", 0, '\0', false }
/* clang-format on */

/* ============================================================================
 * API + ABI functions
 *
 * Functions below can be used from C and they exist in static/shared library,
 * so they can be referenced via any other language and may be imported
 * dynamically.
 * ============================================================================
 */

/** Flush the output. */
extern void ffmt_flush(ffmt_out_t* out);

/** Write single character to the output. */
extern size_t ffmt_putc(ffmt_out_t* out, char c);

/** Write string of characters of specified length to the output. */
extern size_t ffmt_puts(ffmt_out_t* out, const char* str, size_t length);

/** Write string with padding. See ffmt_pad_t. */
extern size_t
ffmt_puts_pad(ffmt_out_t* out, const char* str, size_t length, ffmt_pad_t pad);

/** Convert integer to decimal ASCII / UTF-8 string without \0. */
extern int ffmt_u64_to_dec(uint64_t value, char* buffer, size_t buffer_size);

/** Convert integer to hexadecimal ASII / UTF-* string wihout \0. */
extern int
ffmt_u64_to_hex(uint64_t value, char* buffer, size_t buffer_size, bool upper);

/**
 * Write formatted string with substituted args.
 *
 * Format string can refer to arguments via `{}`/`{N}` syntax. After firs
 * reference with an explicit number, all further references must have explicit
 * number too.
 *
 * Argument references may contain instructions to a formatter in a reference.
 * E.g. `{0:#X}`. How these instructions are used is left to a formatter.
 *
 * All builtin formatters parse padding in instructions (see `ffmt_pad_t`):
 *   - Single char '<', '>' or '^' for alignment
 *   - Optional padding string in parens: '(0)'
 *   - Optional auxillary paddings string in parens: '(0)'
 *   - Desired width or reference to argument containing width: '10' / '@2'
 * Examples:
 *   '{:^([)(])10}' - pad up to 10 chars with [ on the left and ] on the right
 *                    [[[quak]]]
 *   '{:>20}'       - pad up to 20 chars with space on the left
 *   '{:<@2}'       - pad up to X chars with space on the left, where X is the
 *                    value from 2-nd argument
 *
 * Returns number of characters written or an error, see `FFMT_E*`.
 */
extern size_t ffmt_write(
    ffmt_out_t* out,
    size_t format_length,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args);

/** Writes formatted string to a buffer. See `ffmt_write`. */
extern size_t ffmt_write_to_string(
    char* destination,
    size_t destination_size,
    size_t format_length,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args);

/** Formatter for null-terminated string. */
extern FFMT_FORMATTER_DECL(ffmt_formatter_strz);

/** Formatter for single characters. */
extern FFMT_FORMATTER_DECL(ffmt_formatter_char);

/**
 * Formatter for unsigned integers. Default output is decimal.
 * Accepts following formatting instructions:
 *    'x'   - Output hexadecimal, lowercase.
 *    'X'   - Ouput hexadecimal, uppercase.
 *    '#'   - Add prefix: '0x' for hex, '0d' for dec.
 */
extern FFMT_FORMATTER_DECL(ffmt_formatter_u64);

/** Formatter for signed integers. See `ffmt_formatter_u64`. */
extern FFMT_FORMATTER_DECL(ffmt_formatter_i64);

/** Formatter for booleans. Ouputs 'true' or 'false'. */
extern FFMT_FORMATTER_DECL(ffmt_formatter_bool);

/** Formatter for pointers. Ouputs 16-digit hex value. */
extern FFMT_FORMATTER_DECL(ffmt_formatter_ptr);

/* ============================================================================
 * Non-public utils
 * ============================================================================
 */
static inline size_t ffmt__strlen(const char* str) {
  const char* x = str;
  while (*x) {
    x++;
  }

  return x - str;
}

static inline size_t ffmt__argslen(const ffmt_arg_t* args) {
  size_t length = 0;
  while (args[length].formatter != FFMT__EZ_FORMATTER_LAST) {
    length++;
  }

  return length;
}

/** Use existing strlen if not in the freestanting world. */
#ifdef __cplusplus
size_t strlen(const char* str) throw()
    __attribute__((pure, weak, alias("ffmt__strlen")));
#else
size_t strlen(const char* str)
    __attribute__((pure, weak, alias("ffmt__strlen")));
#endif

/* ============================================================================
 * API functions
 *
 * Functions below are available only from C and has no ABI.
 * ============================================================================
 */
static inline bool ffmt_is_err(size_t value) {
  return value >= (size_t)-4096;
}

static inline size_t ffmt_ez_write_to_string(
    char* destination,
    size_t destination_size,
    const char* format,
    const ffmt_arg_t* args) {
  return ffmt_write_to_string(
      destination,
      destination_size,
      strlen(format),
      format,
      ffmt__argslen(args),
      args);
}

static inline size_t
ffmt_ez_write(ffmt_out_t* out, const char* format, const ffmt_arg_t* args) {
  return ffmt_write(out, strlen(format), format, ffmt__argslen(args), args);
}

static inline size_t ffmt_ez_puts(ffmt_out_t* out, const char* str) {
  return ffmt_puts(out, str, strlen(str));
}

static inline size_t
ffmt_ez_puts_pad(ffmt_out_t* out, const char* str, ffmt_pad_t pad) {
  return ffmt_puts_pad(out, str, strlen(str), pad);
}

#endif /* FFMT_H__ */
