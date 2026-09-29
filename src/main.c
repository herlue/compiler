#include "parser.h"
#include "ast.h"

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
  arena_t arena = arena_init(ARENA_SIZE);
  ast_node_stack_t stack = ast_node_stack_init(AST_NODE_STACK_CAPACITY);

  if (arena.capacity == 0 || stack.capacity == 0) {
    fprintf(stderr, "arena or stack capacity = 0\n");
    exit(EXIT_FAILURE);
  }

  parser_t parser = parser_init(&scanner, &arena, &stack);
  ast_node_t* node = parser_parse(&parser);
  if (!node) {
    puts("INVALID");
  } else {
    size_t i;
    if (node->program.namespace) {
      printf("NAMESPACE: ");
      fflush(stdout);
      ast_node_t* ns = node->program.namespace;
      for (i = 0; i < ns->ns_decl.name->qual_id.parts.count; i++) {
        ast_node_t* id = ns->ns_decl.name->qual_id.parts.items[i];
        write(STDOUT_FILENO, id->id.name, id->id.length);
        putchar('\t');
        fflush(stdout);
      }
      putchar('\n');
    }

    ast_node_list_t uses = node->program.uses;
    for (i = 0; i < uses.count; i++) {
      ast_node_t* use = uses.items[i];
      size_t j;
      for (j = 0; j < use->use_decl.name->qual_id.parts.count; i++) {
        ast_node_t* id = use->use_decl.name->qual_id.parts.items[i];
        write(STDOUT_FILENO, id->id.name, id->id.length);
        write(STDOUT_FILENO, "\t", 1);
      }
      write(STDOUT_FILENO, "\t", 1);
    }
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

  arena_free(&arena);
  free(source);
  close(fd);
  exit(EXIT_SUCCESS);
}
