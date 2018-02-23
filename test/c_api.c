#include "ffmt.h"

#include <string.h>

int main() {
  static char buffer[256];
  const size_t err = ffmt_write_to_string(buffer, sizeof(buffer), "{1}{0}", FFMT_AUTO, (ffmt_arg_t[]){
    { ffmt_formatter_u64, (void*)42 },
    { ffmt_formatter_strz, "foo" },
    { FFMT_FORMATTER_LAST }
  });

  return err == 6 && strcmp("foo42", buffer) ? 1 : 0;
}
