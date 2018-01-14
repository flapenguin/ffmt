#ifndef FFMT_HXX__
#define FFMT_HXX__

extern "C" {
#include "ffmt.h"
}

#include <cstddef>
#include <exception>
#include <utility>

namespace ffmt {
  inline auto
  puts(::ffmt_out_t& o, const char* s, size_t length = FFMT_AUTO) noexcept {
    return ::ffmt_puts(&o, s, length);
  }

  inline auto putc(::ffmt_out_t& o, char c) noexcept {
    return ::ffmt_putc(&o, c);
  }

  inline auto flush(::ffmt_out_t& o) noexcept {
    return ::ffmt_flush(&o);
  }

  namespace arg_packers {
    // clang-format off
    template <typename T>
    auto get_formatter(const T*) { return ::ffmt_formatter_ptr; }

    auto get_formatter(const char*) { return ::ffmt_formatter_str; }
    auto get_formatter(char) { return ::ffmt_formatter_char; }
    auto get_formatter(bool) { return ::ffmt_formatter_bool; }
    auto get_formatter(int) { return ::ffmt_formatter_i64; }
    auto get_formatter(int64_t) { return ::ffmt_formatter_i64; }
    auto get_formatter(unsigned int) { return ::ffmt_formatter_u64; }
    auto get_formatter(uint64_t) { return ::ffmt_formatter_u64; }
    // clang-format on

    template <typename T>
    auto pack(T value) {
      static_assert(
          sizeof(T) <= sizeof(intptr_t) && alignof(T) <= alignof(intptr_t),
          "Bad size/alignment of value");

      return (::ffmt_arg_t){get_formatter(value), (const void*)(intptr_t)value};
    }
  } // namespace arg_packers

  template <typename... Args>
  inline auto
  write(::ffmt_out_t& o, const char* format, Args... args) noexcept {
    const ffmt_arg_t packed_args[] = {arg_packers::pack(args)...};
    return ::ffmt_write(&o, format, packed_args, sizeof...(Args));
  }

  template <typename... Args>
  size_t write_to_string(
      char* buffer,
      size_t buffer_size,
      const char* format,
      Args&&... args) noexcept {
    const ffmt_arg_t packed_args[] = {arg_packers::pack(args)...};
    return ::ffmt_write_to_string(
        buffer, buffer_size, format, packed_args, sizeof...(Args));
  }

  namespace details {
    template <class Container, class Member>
    size_t offset_of_(const Member Container::*member) {
      return (size_t) & (reinterpret_cast<Container*>(0)->*member);
    }

    template <typename Container, typename Member>
    Container* container_of_(Member* ptr, const Member Container::*member) {
      return (Container*)((char*)ptr - offset_of_(member));
    }

    static const char* map_error_code(size_t error) noexcept {
      if (!ffmt_is_err(error)) {
        return "Thrown by a mistake. (probably a bug in c++ wrapper)";
      }

      switch (error) {
#define FFMT_CXX_WRAPPER_MAP_ERROR(Value, Description)                         \
  case Value:                                                                  \
    return #Value ": " Description;

        FFMT_FOREACH_ERROR(FFMT_CXX_WRAPPER_MAP_ERROR)

#undef FFMT_CXX_WRAPPER_MAP_ERROR
        default:
          return "Unknown error. (probably a bug in c++ wrapper)";
      }
    }
  }

  struct exception final : public std::exception {
  private:
    const char* const msg;

  public:
    const size_t error_code;

    exception(size_t error)
        : msg(details::map_error_code(error)), error_code(error) {
    }

    const char* what() const noexcept override {
      return msg;
    }
  };

  void throw_if_failed(size_t result) {
    if (::ffmt_is_err(result)) {
      throw exception(result);
    }
  }

  struct out {
  public:
    struct backend_with_owner final {
      ::ffmt_out_t data;
      out* owner;
    };

  protected:
    backend_with_owner backend;

  public:
    out() {
      backend.data.pos = 0;
      backend.data.flush = _static_flush;
      backend.owner = this;
    }
    ~out() = default;

    out(out&&) = delete;
    out(out&) = delete;
    out& operator=(out&) = delete;

    virtual void flush() = 0;

    ::ffmt_out_t& get_ffmt_out() {
      return backend.data;
    }

    size_t puts(const char* s, size_t length = FFMT_AUTO) {
      return _check(::ffmt::puts(backend.data, s, length));
    }

    size_t
    puts(std::nothrow_t, const char* s, size_t length = FFMT_AUTO) noexcept {
      return ::ffmt::puts(backend.data, s, length);
    }

    size_t putc(char c) {
      return _check(::ffmt::putc(backend.data, c));
    }

    size_t putc(std::nothrow_t, char c) noexcept {
      return ::ffmt::putc(backend.data, c);
    }

    template <typename... Args>
    size_t write(const char* format, Args&&... args) {
      return _check(
          ::ffmt::write(backend.data, format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    size_t write(std::nothrow_t, const char* format, Args&&... args) noexcept {
      return ::ffmt::write(backend.data, format, std::forward<Args>(args)...);
    }

  private:
    static void _static_flush(::ffmt_out_t* self) {
      backend_with_owner* b =
          details::container_of_(self, &backend_with_owner::data);
      b->owner->flush();
    }

    static size_t _check(size_t result) {
      throw_if_failed(result);

      return result;
    }
  };
} // namespace ffmt

#endif
