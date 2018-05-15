# `ffmt` - freestanding formatting library

> Formatting library for C99 without any dependencies (even libc) and clear
> distinction between API and ABI.

## Example

```c
#include <ffmt.h>

// You can wrap ffmt_out_t into your own struct and restore pointer to your
// original struct via container_of() macro or simple cast, if ffmt_out_t is
// the first member.
typedef struct {
    ffmt_out_t out;
    char* context;
} my_output;

static void my_flush(ffmt_out_t* out) {
    // Do whatever you want:

    // Syscalls.
    syscall(SYS_write, 1, out->buffer, out->pos);
    // Pass to another library.
    printf("%*.s", out->buffer, out->pos);
    // Copy to memory somewhere else.
    memcpy(all_my_stuff, out->buffer, out->pos);
    all_my_stuff += out->pos;

    // You may flush whole buffer.
    out->pos = 0;
    // Or only part of it.
    out->pos -= 30;
}

int main() {
    static char buffer[256];
    ffmt_out_t output = {
        .buffer = buffer,
        .buffer_size = sizeof(buffer),
        .pos = 0,
        .flush = my_flush,
        // Automatically flush on char.
        .flags = FFMT_FLUSH_CHAR,
        .flush_char = '\n'
    };
    size_t err;

    err = ffmt_ez_write(output, "hello from {3}@{2}.{1}.{0}\n", (ffmt_arg_t[]){
        { ffmt_formatter_u64, (void*)FFMT_VER_PATCH },
        { ffmt_formatter_u64, (void*)FFMT_VER_MINOR },
        { ffmt_formatter_u64, (void*)FFMT_VER_MAJOR },
        { ffmt_formatter_strz, "ffmt" },
        FFMT_EZ_ARG_LAST
    });

    return !ffmt_is_err(err);
}
```

## API

### C

See `src/ffmt.h`.

`ffmt_ez_*` functions is the same as `ffmt_` but allow you to pass null-terminated format strings and `FFMT_EZ_ARG_LAST`-terminated arguments lists.

### C++

See `src/ffmt.hxx`.

## Building

You'll need a fresh version of CMake 3.9 (Jul 2017).

```sh
$ mkdir -p build
$ cd build
$ cmake ..
$ cmake --build .
$ ls bin
libffmt.a  libffmt.so
```

## Development

Run `./tools/githooks/install.sh` to install git hooks.

Useful scripts (also see `config.local.sh.example`):

- `./tools/lint` - lint whole project
- `./tools/run-tests` - run tests in debug and release mode

## Why?

I needed some extensible, composable and dependency-free formatting library for my glibc-free linux and handmade dynamic linker experiments. So here it is.
