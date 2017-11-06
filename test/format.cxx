extern "C" {
#include "ffmt.h"
}

#include <cstring>
#include <cstdio>
#include <sstream>

static void asserteq(const char* expected, const char* actual) {
  if (strcmp(expected, actual)) {
    fprintf(stderr, "expected '%s', but got '%s'\n", expected, actual);
  }
}

static void test_flush() {
  static std::stringstream output;

  static char buffer[4096];
  ffmt_t fmt;
  ffmt_out_t out = { (uint8_t*)&buffer[0], sizeof(buffer), 0, [](ffmt_out_t* self) {
    output.write((const char*)self->buffer, self->pos);
    self->pos = 0;
  }};

  asserteq("", output.str().c_str());

  ffmt_putc(&fmt, &out, '1');
  asserteq("", output.str().c_str());

  ffmt_out_flush(&out);
  asserteq("1", output.str().c_str());

  ffmt_putc(&fmt, &out, '2');
  asserteq("1", output.str().c_str());

  ffmt_out_flush(&out);
  asserteq("12", output.str().c_str());
}

int main() {
  test_flush();

  return 0;
}
