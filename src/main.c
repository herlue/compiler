#include "parser.h"
#include "ast.h"
#include "semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define ARENA_SIZE (1024 << 10)
#define AST_NODE_STACK_CAPACITY 1024

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
  arena_t parser_arena = arena_init(ARENA_SIZE);
  ast_node_stack_t stack = ast_node_stack_init(AST_NODE_STACK_CAPACITY);

  if (parser_arena.capacity == 0 || stack.capacity == 0) {
    fprintf(stderr, "arena or stack capacity = 0\n");
    exit(EXIT_FAILURE);
  }

  arena_t semantic_context_arena = arena_init(ARENA_SIZE);
  if (semantic_context_arena.capacity == 0) {
    fprintf(stderr, "semantic context arena capacity = 0\n");
    exit(EXIT_FAILURE);
  }

  semantic_context_t semantic_context = semantic_context_init(&semantic_context_arena);  

  parser_t parser = parser_init(&scanner, &parser_arena, &stack);
  ast_node_t* node = parser_parse(&parser);
  if (!node) {
    puts("INVALID");
  } else {
    puts("PARSE VALID");

    bool semantic_valid = semantic_analysis(&semantic_context, node);

    puts(semantic_valid ? "SEMANTIC VALID" : "SEMANTIC INVALID");
  }

  // token_t token;
  // do {
  //   token = scanner_next(&scanner);
  //   printf(
  //     "TOKEN line[%2zu-%2zu] offset[%2zu-%2zu] column[%2zu-%2zu]\t<",
  //     token.src_span.start.line,
  //     token.src_span.end.line,
  //     token.src_span.start.offset,
  //     token.src_span.end.offset,
  //     token.src_span.start.column,
  //     token.src_span.end.column
  //   );
  //   if (token.length > 0) {
  //     fwrite(token.lexeme, 1, token.length, stdout);
  //   }
  //   puts(">");
  // } while (token.type != TOK_EOF && token.type != TOK_ERR);

  arena_free(&parser_arena);
  arena_free(&semantic_context_arena);
  free(source);
  close(fd);
  exit(EXIT_SUCCESS);
}
