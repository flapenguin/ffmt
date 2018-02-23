#ifndef FFMT_H__
#define FFMT_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#define FFMT_AUTO ((size_t)-1)
#define FFMT_FORMATTER_LAST ((ffmt_formatter_t)-1)

#define FFMT_FLUSH_CHAR 0x1

#define FFMT_EFLUSH ((size_t)-1)
#define FFMT_EFORMAT ((size_t)-2)
#define FFMT_EARGLEN ((size_t)-3)
#define FFMT_ENOFORMATTER ((size_t)-4)
#define FFMT_ESMALLBUF ((size_t)-5)
#define FFMT_EALIGN ((size_t)-6)

/* clang-format off */
#define FFMT_FOREACH_ERROR(Fn) \
    Fn(FFMT_EFLUSH, "Cannot flush.") \
    Fn(FFMT_EFORMAT, "Bad format.") \
    Fn(FFMT_EARGLEN, "Bad argument index.") \
    Fn(FFMT_ENOFORMATTER, "No formatter.") \
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

/* Forward declarations */
typedef struct ffmt_out_t ffmt_out_t;
typedef struct ffmt_arg_t ffmt_arg_t;
typedef struct ffmt_pad_t ffmt_pad_t;
typedef FFMT_FORMATTER_DECL((*ffmt_formatter_t));

/* Actual declaration of ffmt structs */
struct ffmt_out_t {
  uint8_t* buffer;
  size_t buffer_size;
  size_t pos;
  uint64_t flags;

  void (*flush)(ffmt_out_t* self);
  char flush_char;
};

struct ffmt_arg_t {
  const ffmt_formatter_t formatter;
  const void* value;
};

struct ffmt_pad_t {
  size_t width;
  const char* str;
  size_t str_length;
  const char* auxstr;
  size_t auxstr_length;
  char align;
  bool sticky;
};

/* clang-format off */
#define FFMT_PAD_DEFAULT (ffmt_pad_t){ 0, " ", 1, "", 0, '\0', false }
/* clang-format on */

/* API */
static inline bool ffmt_is_err(size_t value) {
  return value >= (size_t)-4096;
}

/* API + ABI */
extern void ffmt_flush(ffmt_out_t* out);

extern size_t ffmt_putc(ffmt_out_t* out, char c);
extern size_t ffmt_puts(ffmt_out_t* out, const char* str, size_t length);
extern size_t
ffmt_puts_pad(ffmt_out_t* out, const char* str, size_t length, ffmt_pad_t pad);

extern int ffmt_u64_to_dec(uint64_t value, char* buffer, size_t buffer_size);
extern int
ffmt_u64_to_hex(uint64_t value, char* buffer, size_t buffer_size, bool upper);

extern size_t ffmt_write(
    ffmt_out_t* out,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args);

extern size_t ffmt_write_to_string(
    char* destination,
    size_t destination_size,
    const char* format,
    size_t args_length,
    const ffmt_arg_t* args);

extern FFMT_FORMATTER_DECL(ffmt_formatter_strz);
extern FFMT_FORMATTER_DECL(ffmt_formatter_char);
extern FFMT_FORMATTER_DECL(ffmt_formatter_i64);
extern FFMT_FORMATTER_DECL(ffmt_formatter_u64);
extern FFMT_FORMATTER_DECL(ffmt_formatter_bool);
extern FFMT_FORMATTER_DECL(ffmt_formatter_ptr);

#endif /* FFMT_H__ */
