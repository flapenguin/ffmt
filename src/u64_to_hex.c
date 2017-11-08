#include "ffmt.internal.h"

uint ffmt__u64_digits_hex(uint64_t value) {
  if (value < 0x100ull) return 2;
  if (value < 0x10000ull) return 4;
  if (value < 0x1000000ull) return 6;
  if (value < 0x100000000ull) return 8;
  if (value < 0x10000000000ull) return 10;
  if (value < 0x1000000000000ull) return 12;
  if (value < 0x100000000000000ull) return 14;
  return 16;
}

void ffmt__u64_to_hex_impl(
    uint64_t value,
    char* const buffer,
    size_t digits,
    bool upper) {
  const char* const hex_lookup =
      upper ? "0123456789ABCDEF" : "0123456789abcdef";

  uint pos = digits - 1;
  do {
    buffer[pos--] = hex_lookup[value & 0xf];
    buffer[pos--] = hex_lookup[(value >> 4) & 0xf];
    value >>= 8;
  } while (value);
}

int ffmt_u64_to_hex(
    uint64_t value,
    char* const buffer,
    size_t buffer_size,
    bool upper) {
  const uint digits = ffmt__u64_digits_hex(value);
  if (digits >= buffer_size) {
    return -1;
  }

  ffmt__u64_to_hex_impl(value, buffer, digits, upper);

  return digits;
}
