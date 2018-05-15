#include "ffmt.h"

#include <string.h>

int main() {
  static char buffer[256];
  size_t err;

  err = ffmt_ez_write_to_string(buffer, sizeof(buffer), "{1}{0}", (ffmt_arg_t[]){
    { ffmt_formatter_u64, (void*)42 },
    { ffmt_formatter_strz, "foo" },
    FFMT_EZ_ARG_LAST
  });

  if (err != 6 || strcmp("foo42", buffer) != 0) {
    return 1;
  }

  err = ffmt_write_to_string(buffer, sizeof(buffer), 6, "foobar{1}{0}", 2, (ffmt_arg_t[]){
    { ffmt_formatter_u64, (void*)42 },
    { ffmt_formatter_strz, "foo" },
  });

  if (err != 7 || strcmp("foobar", buffer) != 0) {
    return 1;
  }

  return 0;
}
