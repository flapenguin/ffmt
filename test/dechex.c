#include "ffmt.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

static bool failed = false;

static void check_dec(uint64_t value, const char* expected) {
  static char actual[128];

  int r = ffmt_u64_to_dec(value, actual, sizeof(actual));
  actual[r] = '\0';

  if (strlen(expected) != r || strcmp(actual, expected) != 0) {
    fprintf(stderr, "expected '%s', got '%s'\n", expected, actual);
    failed = true;
  }
}

static void check_hex(uint64_t value, const char* expected) {
  static char actual[128];

  int r = ffmt_u64_to_hex(value, actual, sizeof(actual), true);
  actual[r] = '\0';

  if (strlen(expected) != r || strcmp(actual, expected) != 0) {
    fprintf(stderr, "expected '%s', got '%s'\n", expected, actual);
    failed = true;
  }

  r = ffmt_u64_to_hex(value, actual, sizeof(actual), false);
  actual[r] = '\0';

  static char lowercase_expected[128];
  strcpy(lowercase_expected, expected);
  for (char* s = lowercase_expected; *s; s++) {
    *s = tolower(*s);
  }

  if (strlen(lowercase_expected) != r || strcmp(actual, lowercase_expected) != 0) {
    fprintf(stderr, "expected '%s', got '%s'\n", lowercase_expected, actual);
    failed = true;
  }
}

int main() {
  check_dec(0, "0");
  check_dec(42, "42");
  check_dec(18446744073709551615ULL, "18446744073709551615");

  check_hex(0x00, "00");
  check_hex(0x42, "42");
  check_hex(0xabc, "0ABC");
  check_hex(0xFFFFFFFFFFFFFFFFULL, "FFFFFFFFFFFFFFFF");

  return failed;
}
