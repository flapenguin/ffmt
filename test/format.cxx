#include "ffmt.hxx"

#include <cstring>
#include <cstdio>
#include <sstream>

static bool failed = false;

template <typename T, size_t Size>
static inline size_t array_size(T (&x)[Size]) { return Size; }

template <size_t Size>
static inline size_t str_size(const char (&x)[Size]) { return Size - 1; }

template <size_t BufferSize>
struct ss_output final : public ffmt::out {
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

  std::stringstream& get_stringstream() {
    return _ss;
  }
};

static void asserteq(const char* expected, const char* actual) {
  if (strcmp(expected, actual)) {
    fprintf(stderr, "expected '%s', but got '%s'\n", expected, actual);
    failed = true;
  }
}

template <template<size_t BufferSize> class ss_output, size_t BufferSize>
static void asserteq(const char* expected, ss_output<BufferSize>& out) {
  asserteq(expected, out.get_stringstream().str().c_str());
}

static void asserteq(size_t expected, size_t actual) {
  if (expected != actual) {
    fprintf(stderr, "expected %zd, but got %zd\n", (ssize_t)expected, (ssize_t)actual);
    failed = true;
  }
}

static void test_smoke() {
  ss_output<256> out;

  asserteq("", out);

  out.putc('1');
  asserteq("", out);

  out.flush();
  asserteq("1", out);

  out.putc('2');
  asserteq("1", out);

  out.flush();
  asserteq("12", out);

  out.puts("qwerty");
  asserteq("12", out);

  out.flush();
  asserteq("12qwerty", out);
}

static void test_flush() {
  ss_output<8> out;

  out.puts("0123456789");
  asserteq("01234567", out);

  out.flush();
  asserteq("0123456789", out);
}

static void test_format() {
  ss_output<64> out;

  {
    const char result[] = "<foo><bar>";

    asserteq(str_size(result), out.write("{1}{3}{2}{1}{0}{2}", "bar", "<", ">", "foo"));

    out.flush();
    asserteq(result, out);
    out.clear();
  }

  {
    const char result[] = "12302652060662169617,123456789123456789,-123456789123456789,"
      "aabbccddeeff0011,0xaabbccddeeff0011,AABBCCDDEEFF0011,0xAABBCCDDEEFF0011,"
      "foobar,true,false";

    asserteq(str_size(result),
        out.write(
          "{0},{1},{2},{0:x},{0:#x},{0:X},{0:#X},{3},{4},{5}",
          0xaabbccddeeff0011,
          123456789123456789UL,
          -123456789123456789L,
          "foobar",
          true,
          false));

    out.flush();
    asserteq(result, out);
    out.clear();
  }

  asserteq(FFMT_EFORMAT, out.write(std::nothrow, "{...}"));
  asserteq(FFMT_EARGLEN, out.write(std::nothrow, "{1}"));

  {
    const ffmt_arg_t args[] = { {0, (const void*)"nope" } };
    asserteq(FFMT_ENOFORMATTER, ::ffmt_write(&out.get_ffmt_out(), "{0}", args, array_size(args)));
    out.clear();
  }
}

static void test_throw() {
  try {
    ss_output<18> out;
    out.write("{}", 0xaabbccddeeff0011);
  }
  catch (ffmt::exception& e) {
    asserteq(FFMT_ESMALLBUF, e.error_code);
  }
  catch (...) {
    fprintf(stderr, "Something else failed.\n");
    failed = 1;
  }
}

int main() {
  test_smoke();
  test_flush();
  test_format();
  test_throw();

  return failed ? 1 : 0;
}
