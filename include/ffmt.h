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

typedef struct ffmt_out_t ffmt_out_t;
typedef struct ffmt_t ffmt_t;

struct ffmt_out_t {
  uint8_t* buffer;
  size_t buffer_size;
  size_t pos;

  void (*flush)(ffmt_out_t* self);
};

static inline void ffmt_out_flush(ffmt_out_t* out) {
  out->flush(out);
}

struct ffmt_t {};

extern size_t ffmt_putc(
  const ffmt_t* self,
  ffmt_out_t* out,
  char c);

extern int ffmt_u64_to_dec(uint64_t value, char* buffer, size_t buffer_size);
extern int ffmt_u64_to_hex(uint64_t value, char* buffer, size_t buffer_size, bool upper);

#endif /* FFMT_H__ */
