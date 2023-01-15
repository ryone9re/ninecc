#include "ninecc.h"

char *remove_escape_string(char *str)
{
  int count = 0;
  char *s = str;
  char *result;

  while (*s != '\0')
  {
    if (*s == '\\')
    {
      s++;
      if (*s == '\0')
        break;
    }
    s++;
    count++;
  }

  result = (char *)calloc(count + 1, sizeof(char));

  if (!result)
    exit_with_error();

  int i = 0;
  while (i < count)
  {
    if (*str == '\\')
    {
      str++;
      if (*str == '\0')
        break;
      if (*str == 'a')
        result[i] = '\a';
      else if (*str == 'b')
        result[i] = '\b';
      else if (*str == 't')
        result[i] = '\t';
      else if (*str == 'n')
        result[i] = '\n';
      else if (*str == 'v')
        result[i] = '\v';
      else if (*str == 'f')
        result[i] = '\f';
      else if (*str == 'r')
        result[i] = '\r';
      else if (*str == 'e')
        result[i] = '\e';
      else
        result[i] = *str;
    }
    else
      result[i] = *str;
    str++;
    i++;
  }
  result[i] = '\0';
  return (result);
}

char *substr(char *str, size_t len)
{
  char *s = (char *)calloc(len + 1, sizeof(char));

  if (!s)
    exit_with_error();
  strncpy(s, str, len);
  return (s);
}
