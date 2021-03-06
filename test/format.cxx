#include "ffmt.hxx"

#include <cstring>
#include <cstdio>
#include <sstream>

static bool failed = false;

template <typename T, size_t Size>
static inline size_t array_size(T (&x)[Size]) { return Size; }

template <size_t BufferSize>
struct ss_output final : public ffmt::out {
private:
  std::stringstream _ss;
  char _buffer[BufferSize];
public:
  ss_output() {
    this->backend.data.buffer = (uint8_t*)&this->_buffer;
    this->backend.data.buffer_size = array_size(this->_buffer);
    this->backend.data.flags = 0;
  }

  ss_output(ss_output&&) = default;

  ss_output(char flush_char) : ss_output() {
    this->backend.data.flags = FFMT_FLUSH_CHAR;
    this->backend.data.flush_char = flush_char;
  }

  void flush() override {
    this->_ss.write((const char*)this->backend.data.buffer, this->backend.data.pos);
    this->backend.data.pos = 0;
  }

  void clear() {
    _ss.str("");
    _ss.clear();
  }

  std::string to_string() {
    return _ss.str();
  }
};

static void asserteq(std::string expected, std::string actual) {
  if (expected != actual) {
    fprintf(stderr, "expected %ld:'\e[32m%.*s\e[0m', but got %ld:'\e[31m%.*s\e[0m'\n",
      expected.length(), (int)expected.length(), expected.c_str(),
      actual.length(), (int)actual.length(), actual.c_str());

    failed = true;
  }
}

template <template<size_t BufferSize> class ss_output, size_t BufferSize>
static void asserteq(const char* expected, ss_output<BufferSize>& out) {
  asserteq(expected, out.to_string());
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

    out.write("{1}{3}{2}{1}{0}{2}", "bar", "<", ">", "foo");

    out.flush();
    asserteq(result, out);
    out.clear();
  }

  {
    const char result[] =
      "\n12302652060662169617,123456789123456789,-123456789123456789"
      "\naabbccddeeff0011,0xaabbccddeeff0011,AABBCCDDEEFF0011,0xAABBCCDDEEFF0011"
      "\nfoobar,true,false,x,y"
      "\n00000000000000aa,0x00000000000000AA"
      "\n";

      out.write(
        "\n{0},{1},{2}"
        "\n{0:x},{0:#x},{0:X},{0:#X}"
        "\n{3},{4},{5},{6},{7}"
        "\n{8},{8:#X}"
        "\n",
        0xaabbccddeeff0011,
        123456789123456789UL,
        -123456789123456789L,
        "foobar",
        true,
        false,
        'x',
        'y',
        (void*)0xaa);

    out.flush();
    asserteq(result, out);
    out.clear();
  }

  asserteq(FFMT_EFORMAT, out.write(std::nothrow, "{...}"));
  asserteq(FFMT_EARGLEN, out.write(std::nothrow, "{1}"));

  {
    const ffmt_arg_t args[] = { {0, (const void*)"nope" }, FFMT_EZ_ARG_LAST };
    asserteq(FFMT_ENOFORMATTER, ::ffmt_ez_write(&out.get_ffmt_out(), "{0}", args));
    out.clear();
  }
}

static void test_pad() {
  ss_output<64> out;

  const char result[] =
    ".\n123456,foobar"
    ".\n123,foo"
    ".\n123456    ,foobar    "
    ".\n123456_-=_,foobar_-=_"
    ".\n456,bar"
    ".\n    123456,    foobar"
    ".\n-=-=123456,-=-=foobar"
    ".\n  123456  ,  foobar  "
    ".\n1234567,foobar!"
    ".\n123,foo"
    ".\n1234567   ,foobar!   "
    ".\n567,ar!"
    ".\n   1234567,   foobar!"
    ".\n 1234567  , foobar!  "
    ".\n.1234567..,.foobar!.."
    ".\n......custom,custom...... (width = 12)"
    ".\n-<-foobar->-,-<foobar!->-"
    ".\n";

  const char format[] =
    ".\n{0},{1}"
    ".\n{0:<3},{1:<3}"
    ".\n{0:<10},{1:<10}"
    ".\n{0:<(_-=)10},{1:<(_-=)10}"
    ".\n{0:>3},{1:>3}"
    ".\n{0:>10},{1:>10}"
    ".\n{0:>(-=)10},{1:>(-=)10}"
    ".\n{0:^10},{1:^10}"
    ".\n{2},{3}"
    ".\n{2:<3},{3:<3}"
    ".\n{2:<10},{3:<10}"
    ".\n{2:>3},{3:>3}"
    ".\n{2:>10},{3:>10}"
    ".\n{2:^10},{3:^10}"
    ".\n{2:^(.)10},{3:^(.)10}"
    ".\n{4:>(.)@5},{4:<(.)@5} (width = {5})"
    ".\n{1:^*(-<)(>-)12},{3:^*(-<)(>-)12}"
    ".\n";

  out.write(
    format,
    123456,
    "foobar",
    1234567u,
    "foobar!",
    "custom",
    12);

  out.flush();
  asserteq(result, out);
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

static void test_write_to_string() {
  {
    char buffer[1024];
    const size_t r = ffmt::write_to_string(buffer,
      sizeof(buffer),
      "{3},{2},{1},{0}",
      -42,
      1024,
      14u,
      "foobar");

    ffmt::throw_if_failed(r);

    const char expect[] = "foobar,14,1024,-42";
    asserteq(expect, buffer);
  }

  {
    char buffer[8];
    const size_t r = ffmt::write_to_string(buffer, sizeof(buffer), "{0:>7}", "xyz");
    ffmt::throw_if_failed(r);

    const char expect[] = "    xyz";
    asserteq(expect, buffer);
  }
}

static void test_flush_char() {
  auto test_nl = [](ss_output<256> out, bool flush_on_lf) {
    out.write("foo\n");
    asserteq(flush_on_lf ? "foo\n" : "", out);

    out.write("baz\n");
    asserteq(flush_on_lf ? "foo\nbaz\n" : "", out);

    out.write("qux");
    asserteq(flush_on_lf ? "foo\nbaz\n" : "", out);

    out.flush();
    asserteq("foo\nbaz\nqux", out);
  };

  auto test_slash = [](ss_output<256> out, bool flush_on_slash) {
    out.write("/usr");
    asserteq(flush_on_slash ? "/" : "", out);

    out.write("/include/asm");
    asserteq(flush_on_slash ? "/usr/include/" : "", out);

    out.write("-generic");
    asserteq(flush_on_slash ? "/usr/include/" : "", out);

    out.flush();
    asserteq("/usr/include/asm-generic", out);
  };

  test_nl(ss_output<256> {'\n'}, true);
  test_nl(ss_output<256> {}, false);
  test_slash(ss_output<256> {'/'}, true);
  test_slash(ss_output<256> {}, false);
}

int main() try {
  test_smoke();
  test_flush();
  test_format();
  test_write_to_string();
  test_throw();
  test_pad();
  test_flush_char();

  return failed ? 1 : 0;
} catch (std::exception& e) {
  printf("exception: %s\n", e.what());
  return 1;
} catch (...) {
  printf("unknown exception: something really bad happened\n");
  return 1;
}
