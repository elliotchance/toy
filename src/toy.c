#include "toy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *call_to_string(call_t call) {
  char *s = malloc(100);
  s[0] = 0;

  strncat(s, "[", 1);
  for (int i = 0; i < call.pairs.size; i++) {
    if (i > 0) {
      strncat(s, " ", 1);
    }

    strncat(s, call.pairs.pairs[i].name, strlen(call.pairs.pairs[i].name));
    strncat(s, ":", 1);
    char *value = expr_to_string(call.pairs.pairs[i].expr);
    strncat(s, value, strlen(value));
  }
  strncat(s, "]", 1);

  return s;
}

char *stmt_to_string(struct stmt_t *stmt) {
  char *buf = malloc(1000);
  switch (stmt->kind) {
  case stmt_kind_assign_block:
    // sprintf(buf, "assign: %s = %s\n", stmt->assign_block->var,
    //         expr_to_string(stmt->assign_block->expr));
    break;
  case stmt_kind_expr:
    sprintf(buf, "expr: %s\n", expr_to_string(stmt->expr));
    break;
  case stmt_kind_return:
    sprintf(buf, "return: %s\n", expr_to_string(stmt->rtn));
    break;
  default:
    sprintf(buf, "unknown: %d\n", stmt->kind);
  }

  return buf;
}

char *block_to_string(struct block_t *block) {
  char *buf = malloc(1000);
  buf[0] = 0;
  strcat(buf, "{\n");
  for (int i = 0; i < block->size; i++) {
    strcat(buf, stmt_to_string(block->stmts[i]));
    strcat(buf, "\n");
  }
  strcat(buf, "}\n");
  return buf;
}

void print_block(struct block_t *b, int indent) {
  printf("BLOCK: %s\n", block_to_string(b));
  printf("VARS:\n");
  if (b->var_head != NULL) {
    printf("  %s = %s\n", b->var_head->name,
           expr_to_string(b->var_head->value));
  }
}

char *expr_to_string(struct expr_t *e) {
  switch (e->kind) {
  case expr_kind_string:
    return e->string;

  case expr_kind_var:
    return e->var;

  case expr_kind_block:
    return block_to_string(e->block);

  case expr_kind_call:
    return call_to_string(e->call);

  case expr_kind_number: {
    char *buf = malloc(20);
    sprintf(buf, "%f", e->number);
    return buf;
  }

  case expr_kind_binary: {
    char *buf = malloc(1000);
    sprintf(buf, "(%s %c %s)", expr_to_string(e->binary.left), e->binary.op,
            expr_to_string(e->binary.right));
    return buf;
  }

  default: {
    char *buf = malloc(20);
    sprintf(buf, "error: %d", e->kind);
    return buf;
  }
  }
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    char *s = read_file(argv[i]);
    tokens_t *tokens = tokenize(s, strlen(s));
    // print_tokens(tokens);

    struct block_t *program = parse(tokens);
    // char *buf = malloc(10000000);
    // buf[0] = 0;
    // toy_fmt_program(buf, program);
    // printf("%s", buf);
    // print_block(program, 0);

    run_program(program);
    // print_block(program, 0);
  }

  return 0;
}
