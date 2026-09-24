#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <source>\n", *argv);
    exit(EXIT_FAILURE);
  }

  struct stat attr;
  if (stat(argv[1], &attr) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

  size_t length = attr.st_size;
  char* source = malloc(length * sizeof(char));
  if (source == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("open");
    free(source);
    exit(EXIT_FAILURE);
  }

  if (read(fd, source, length) != length) {
    perror("read");
    free(source);
    close(fd);
    exit(EXIT_FAILURE);
  }

  scanner_t scanner = scanner_init(source, length);
  token_t token;
  do {
    token = scanner_next(&scanner);
    printf(
      "TOKEN line[%2zu-%2zu] offset[%2zu-%2zu] column[%2zu-%2zu]\t<",
      token.src_span.start.line,
      token.src_span.end.line,
      token.src_span.start.offset,
      token.src_span.end.offset,
      token.src_span.start.column,
      token.src_span.end.column
    );
    if (token.length > 0) {
      fwrite(token.lexeme, 1, token.length, stdout);
    }
    puts(">");
  } while (token.type != TOK_EOF && token.type != TOK_ERR);

  free(source);
  close(fd);
  exit(EXIT_SUCCESS);
}
