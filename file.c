#include "ninecc.h"

char *read_file(char *path)
{
  FILE *fp = fopen(path, "r");
  if (!fp)
    exit_with_error();

  if (fseek(fp, 0, SEEK_END) == -1)
    exit_with_error();
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    exit_with_error();

  char *buf = (char *)calloc(1, sizeof(char) * size + 2);
  fread(buf, size, 1, fp);

  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return (buf);
}
