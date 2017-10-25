#ifndef FFMT_INTERNAL_H__
#define FFMT_INTERNAL_H__

#include "ffmt.h"

typedef unsigned int uint;

uint ffmt__u64_digits_dec(uint64_t value);
void ffmt__u64_to_dec_impl(uint64_t value, char* const buffer, size_t digits);

uint ffmt__u64_digits_hex(uint64_t value);
void ffmt__u64_to_hex_impl(uint64_t value, char* const buffer, size_t digits, bool upper);

#endif /* FFMT_INTERNAL_H__ */
