#include "ninecc.h"

char *substr(char *str, size_t len) {
  char *s = (char *)calloc(len + 1, sizeof(char));

  if (!s) exit_with_error();
  strncpy(s, str, len);
  return (s);
}
