#include <stdio.h>
#include <string.h>
#include <errno.h>

void str_error(int errnum, const char *file, int line, const char *func)
{
  int size = 256;
  char *err;
  char buf[size * 2];

  err = strerror(errnum);
  snprintf(buf, size * 2,
           "[ERROR] %s:%i at %s(): %s", file, line, func, err);

  printf("%s\n", buf);
  fflush(stdout);
}
