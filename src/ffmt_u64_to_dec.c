#include "ffmt.internal.h"

uint ffmt__u64_digits_dec(uint64_t value) {
  uint result = 1;

  for (;;) {
    if (value < 10) return result;
    if (value < 100) return result + 1;
    if (value < 1000) return result + 2;
    value /= 1000u;
    result += 3;
  }

  return result;
}

void ffmt__u64_to_dec_impl(uint64_t value, char* const buffer, size_t digits) {
  uint pos = digits - 1;

  while (value >= 10) {
    buffer[pos--] = '0' + (value % 10);
    value /= 10;
  }

  *buffer = '0' + value;
}

int ffmt_u64_to_dec(uint64_t value, char* buffer, size_t buffer_size) {
  const uint digits = ffmt__u64_digits_dec(value);
  if (digits >= buffer_size) {
    return -1;
  }

  ffmt__u64_to_dec_impl(value, buffer, digits);

  return digits;
}
