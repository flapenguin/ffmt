#ifndef FFMT_H__
#define FFMT_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FFMT__STR2(X) #X
#define FFMT__STR(X) FFMT__STR2(X)

#define FFMT_VER_MAJOR 0
#define FFMT_VER_MINOR 1
#define FFMT_VER_PATCH 0

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

#define FFMT_AUTO ((size_t)-1)

#define FFMT_EFLUSH ((size_t)-1)
#define FFMT_EFORMAT ((size_t)-2)
#define FFMT_EARGLEN ((size_t)-3)
#define FFMT_ENOFORMATTER ((size_t)-4)

#define FFMT_FORMATTER_DECL(Name) \
  size_t Name( \
    ffmt_out_t* out, \
    const ffmt_arg_t arg, \
    const ffmt_arg_t* args, \
    size_t args_length, \
    const char* spec, \
    const char* spec_end)

typedef struct ffmt_out_t ffmt_out_t;
typedef struct ffmt_arg_t ffmt_arg_t;
typedef FFMT_FORMATTER_DECL((*ffmt_formatter_t));

struct ffmt_out_t {
  uint8_t* buffer;
  size_t buffer_size;
  size_t pos;

  void (*flush)(ffmt_out_t* self);
};

struct ffmt_arg_t {
  const ffmt_formatter_t formatter;
  const void* value;
};


// API
static inline bool ffmt_is_err(size_t value) {
  return value >= (size_t)-4096;
}

// API + ABI
extern void ffmt_flush(ffmt_out_t* out);

extern size_t ffmt_putc(ffmt_out_t* out, char c);
extern size_t ffmt_puts(ffmt_out_t* out, const char* str, size_t length);

extern int ffmt_u64_to_dec(uint64_t value, char* buffer, size_t buffer_size);
extern int ffmt_u64_to_hex(uint64_t value, char* buffer, size_t buffer_size, bool upper);

extern size_t ffmt_write(
  ffmt_out_t* out,
  const char* format,
  const ffmt_arg_t* args,
  size_t args_length);

extern FFMT_FORMATTER_DECL(ffmt_formatter_str);
extern FFMT_FORMATTER_DECL(ffmt_formatter_i64);
extern FFMT_FORMATTER_DECL(ffmt_formatter_u64);

#endif /* FFMT_H__ */
