#include "ffmt.hxx"

#include <cstring>
#include <cstdio>
#include <sstream>

static bool failed = false;

template <typename T, size_t Size>
static inline size_t array_size(T (&x)[Size]) { return Size; }

template <size_t Size>
static inline size_t str_size(const char (&x)[Size]) { return Size - 1; }

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

static void assert_write(size_t expected, size_t actual) {
  if (expected != actual) {
    fprintf(stderr, "expected %zd, but got %zd\n", (ssize_t)expected, (ssize_t)actual);
    failed = true;
  }
}

static void test_smoke() {
  static std::stringstream output;

  static char buffer[256];
  ffmt_out_t out = { (uint8_t*)&buffer[0], sizeof(buffer), 0, [](ffmt_out_t* self) {
    output.write((const char*)self->buffer, self->pos);
    self->pos = 0;
  }};

  asserteq("", output);

  ffmt::putc(out, '1');
  asserteq("", output);

  ffmt::flush(out);
  asserteq("1", output);

  ffmt::putc(out, '2');
  asserteq("1", output);

  ffmt::flush(out);
  asserteq("12", output);

  ffmt::puts(out, "qwerty");
  asserteq("12", output);

  ffmt::flush(out);
  asserteq("12qwerty", output);
}

template <size_t BufferSize>
struct ss_output : public ffmt::out {
private:
  std::stringstream _ss;
  char _buffer[BufferSize];
public:
  ss_output() {
    this->backend.data.buffer = (uint8_t*)&this->_buffer;
    this->backend.data.buffer_size = array_size(this->_buffer);
  }

  void flush() override {
    this->_ss.write((const char*)this->backend.data.buffer, this->backend.data.pos);
    this->backend.data.pos = 0;
  }

  void clear() {
    _ss.str("");
    _ss.clear();
  }

  auto& get_sringstream() {
    return _ss;
  }
};

static void test_flush() {
  ss_output<8> out;

  out.puts("0123456789");
  asserteq("01234567", out.get_sringstream());

  out.flush();
  asserteq("0123456789", out.get_sringstream());
}

static void test_format() {
  ss_output<64> out;

  {
    const char result[] = "<foo><bar>";

    assert_write(str_size(result), out.write("{1}{3}{2}{1}{0}{2}", "bar", "<", ">", "foo"));

    out.flush();
    asserteq(result, out.get_sringstream());
    out.clear();
  }

  {
    const char result[] = "12302652060662169617,123456789123456789,-123456789123456789,"
      "aabbccddeeff0011,0xaabbccddeeff0011,AABBCCDDEEFF0011,0xAABBCCDDEEFF0011";

    assert_write(str_size(result),
        out.write("{0},{1},{2},{0:x},{0:#x},{0:X},{0:#X}",
          0xaabbccddeeff0011, 123456789123456789UL, -123456789123456789L));

    out.flush();
    asserteq(result, out.get_sringstream());
    out.clear();
  }

  asserteq(FFMT_EFORMAT, out.write("{...}"));
  asserteq(FFMT_EARGLEN, out.write("{1}"));

  {
    const ffmt_arg_t args[] = { {0, (const void*)"nope" } };
    asserteq(FFMT_ENOFORMATTER, ::ffmt_write(&out.get_ffmt_out(), "{0}", args, array_size(args)));
    out.clear();
  }
}

int main() {
  test_smoke();
  test_flush();
  test_format();

  return failed ? 1 : 0;
}
