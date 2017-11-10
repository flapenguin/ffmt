extern "C" {
#include "ffmt.h"
}

#include <cstring>
#include <cstdio>
#include <sstream>

static bool failed = false;

template <typename T, size_t Size>
static inline size_t array_size(T (&x)[Size]) { return Size; }

static void asserteq(const char* expected, const char* actual) {
  if (strcmp(expected, actual)) {
    fprintf(stderr, "expected '%s', but got '%s'\n", expected, actual);
    failed = true;
  }
}

static void asserteq(const char* expected, const std::stringstream& actual) {
  asserteq(expected, actual.str().c_str());
}

static void asserteq(size_t expected, size_t actual) {
  asserteq(std::to_string(expected).c_str(), std::to_string(actual).c_str());
}

static void test_smoke() {
  static std::stringstream output;

  static char buffer[256];
  ffmt_out_t out = { (uint8_t*)&buffer[0], sizeof(buffer), 0, [](ffmt_out_t* self) {
    output.write((const char*)self->buffer, self->pos);
    self->pos = 0;
  }};

  asserteq("", output);

  ffmt_putc(&out, '1');
  asserteq("", output);

  ffmt_flush(&out);
  asserteq("1", output);

  ffmt_putc(&out, '2');
  asserteq("1", output);

  ffmt_flush(&out);
  asserteq("12", output);

  ffmt_puts(&out, "qwerty", FFMT_AUTO);
  asserteq("12", output);

  ffmt_flush(&out);
  asserteq("12qwerty", output);
}

static void test_flush() {
  static std::stringstream output;
  static char buffer[8];
  ffmt_out_t out = { (uint8_t*)&buffer[0], sizeof(buffer), 0, [](ffmt_out_t* self) {
    output.write((const char*)self->buffer, self->pos);
    self->pos = 0;
  }};

  ffmt_puts(&out, "0123456789", FFMT_AUTO);
  asserteq("01234567", output);

  ffmt_flush(&out);
  asserteq("0123456789", output);
}

static void test_format() {
  static std::stringstream output;
  static char buffer[16];
  ffmt_out_t out = { (uint8_t*)&buffer[0], sizeof(buffer), 0, [](ffmt_out_t* self) {
    output.write((const char*)self->buffer, self->pos);
    self->pos = 0;
  }};

  {
    const ffmt_arg_t args[] = {
      { ffmt_formatter_str, (const void*)"bar" },
      { ffmt_formatter_str, (const void*)"<" },
      { ffmt_formatter_str, (const void*)">" },
      { ffmt_formatter_str, (const void*)"foo" },
    };

    asserteq(10, ffmt_write(&out, "{1}{3}{2}{1}{0}{2}", args, array_size(args)));
    ffmt_flush(&out);
    asserteq("<foo><bar>", output);
  }

  asserteq(FFMT_EFORMAT, ffmt_write(&out, "{...}", 0, 0));
  asserteq(FFMT_EARGLEN, ffmt_write(&out, "{1}", 0, 0));
  {
    const ffmt_arg_t args[] = { {0, (const void*)"nope" } };
    asserteq(FFMT_ENOFORMATTER, ffmt_write(&out, "{0}", args, array_size(args)));
  }
}

int main() {
  test_smoke();
  test_flush();
  test_format();

  return failed ? 1 : 0;
}
