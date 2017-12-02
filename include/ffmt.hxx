#ifndef FFMT_HXX__
#define FFMT_HXX__

extern "C" {
#include "ffmt.h"
}

#include <cstddef>
#include <utility>

namespace ffmt {
  inline auto puts(::ffmt_out_t& o, const char* s, size_t length = FFMT_AUTO) {
    return ::ffmt_puts(&o, s, length);
  }

  inline auto putc(::ffmt_out_t& o, char c) {
    return ::ffmt_putc(&o, c);
  }

  inline auto flush(::ffmt_out_t& o) {
    return ::ffmt_flush(&o);
  }

  namespace arg_packers {
    auto pack(const char* value) {
      return (::ffmt_arg_t){::ffmt_formatter_str, (const void*)value};
    }

    auto pack(uint64_t value) {
      return (::ffmt_arg_t){::ffmt_formatter_u64, (const void*)value};
    }

    auto pack(int64_t value) {
      return (::ffmt_arg_t){::ffmt_formatter_i64, (const void*)value};
    }
  } // namespace arg_packers

  template <typename... Args>
  inline auto write(::ffmt_out_t& o, const char* format, Args... args) {
    const ffmt_arg_t packed_args[] = {arg_packers::pack(args)...};
    return ::ffmt_write(&o, format, packed_args, sizeof...(Args));
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
      return ::ffmt::puts(backend.data, s, length);
    }

    size_t putc(char c) {
      return ::ffmt::putc(backend.data, c);
    }

    template <typename... Args>
    size_t write(const char* format, Args&&... args) {
      return ::ffmt::write(backend.data, format, std::forward<Args>(args)...);
    }

  private:
    static void _static_flush(::ffmt_out_t* self) {
      backend_with_owner* b =
          details::container_of_(self, &backend_with_owner::data);
      b->owner->flush();
    }
  };
} // namespace ffmt

#endif
