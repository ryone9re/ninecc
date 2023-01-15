#include "ninecc.h"

// printfと同じ引数を取る
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  size_t line_num = 1;
  for (char *p = user_input; p < line; p++)
  {
    if (*p == '\n')
      line_num++;
  }

  int indent = fprintf(stderr, "%s:%zu: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラーメッセージを出力して終了
void exit_with_error(void)
{
  fprintf(stderr, "%s\n", strerror(errno));
  exit(errno);
}
