extern "C" {
#include "ffmt.h"
}

#include <cstring>
#include <cstdio>
#include <sstream>

static bool failed = false;

static void asserteq(const char* expected, const char* actual) {
  if (strcmp(expected, actual)) {
    fprintf(stderr, "expected '%s', but got '%s'\n", expected, actual);
    failed = true;
  }
}
static void asserteq(const char* expected, const std::stringstream& actual) {
  asserteq(expected, actual.str().c_str());
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

int main() {
  test_smoke();
  test_flush();

  return failed ? 1 : 0;
}
