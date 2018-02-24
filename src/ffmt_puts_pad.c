#include "ffmt.internal.h"

size_t
ffmt_puts_pad(ffmt_out_t* out, const char* str, size_t length, ffmt_pad_t pad) {
  if (!pad.align || !pad.width || pad.width == length) {
    return ffmt_puts(out, str, length);
  }

  size_t left_pad = 0;
  size_t right_pad = 0;

  switch (pad.align) {
    case '<':
      if (pad.width < length) {
        length = pad.width;
      } else {
        right_pad = pad.width - length;
      }
      break;
    case '>':
      if (pad.width < length) {
        str += (length - pad.width);
        length = pad.width;
      } else {
        left_pad = pad.width - length;
      }
      break;
    case '^':
      if (pad.width < length) {
        str += (length - pad.width) / 2;
        length = pad.width;
      } else {
        left_pad = (pad.width - length) / 2;
        right_pad = pad.width - length - left_pad;
      }
      break;
    default:
      return FFMT_EALIGN;
  }

  if (left_pad) {
    FFMT__TRY(ffmt__puts_repeat(out, pad.str, pad.str_length, left_pad));
  }

  FFMT__TRY(ffmt__puts_base(out, str, length));

  if (right_pad) {
    const char* padstr = pad.align == '^' ? pad.auxstr : pad.str;
    const size_t padstr_length =
        pad.align == '^' ? pad.auxstr_length : pad.str_length;

    if (pad.sticky) {
      const size_t part_length = right_pad % padstr_length;
      if (part_length) {
        FFMT__TRY(
            ffmt_puts(out, padstr + part_length, padstr_length - part_length));
        right_pad -= part_length;
      }
    }

    FFMT__TRY(ffmt__puts_repeat(out, padstr, padstr_length, right_pad));
  }

  return pad.width;
}
