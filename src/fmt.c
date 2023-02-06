#include "toy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void toy_fmt_block(char *dest, struct block_t *block, int indent);
void toy_fmt_expr(char *dest, struct expr_t *e, int indent);

void toy_fmt_indent(char *dest, int n) {
  for (int i = 0; i < n; i++) {
    strcat(dest, "  ");
  }
}

void toy_fmt_call(char *dest, call_t e, int indent) {
  strcat(dest, "[");
  for (int i = 0; i < e.pairs.size; i++) {
    if (i > 0) {
      strcat(dest, " ");
    }
    strcat(dest, e.pairs.pairs[i].name);
    strcat(dest, ":");
    toy_fmt_expr(dest, e.pairs.pairs[i].expr, indent);
  }
  strcat(dest, "]");
}

void toy_fmt_ref(char *dest, struct ref_t e, int indent) {
  strcat(dest, "@[");
  for (int i = 0; i < e.pairs.size; i++) {
    if (i > 0) {
      strcat(dest, " ");
    }
    strcat(dest, e.pairs.pairs[i].name);
    strcat(dest, ":");
    if (strcmp(e.pairs.pairs[i].name, e.pairs.pairs[i].alias) != 0) {
      strcat(dest, "(");
      strcat(dest, e.pairs.pairs[i].alias);
      strcat(dest, ")");
    }
    strcat(dest, e.pairs.pairs[i].type);
  }
  strcat(dest, "]");

  if (strcmp(e.returnType, "") != 0) {
    strcat(dest, ": ");
    strcat(dest, e.returnType);
  }
}

void toy_fmt_binary(char *dest, struct binary_t e, int indent) {
  strcat(dest, "(");
  toy_fmt_expr(dest, e.left, indent);
  strcat(dest, " ");
  strncat(dest, &e.op, 1);
  strcat(dest, " ");
  toy_fmt_expr(dest, e.right, indent);
  strcat(dest, ")");
}

void toy_fmt_expr(char *dest, struct expr_t *e, int indent) {
  switch (e->kind) {
  case expr_kind_call:
    toy_fmt_call(dest, e->call, indent);
    return;

  case expr_kind_block:
    toy_fmt_block(dest, e->block, indent);
    return;

  case expr_kind_var:
    strcat(dest, e->var);
    return;

  case expr_kind_binary:
    toy_fmt_binary(dest, e->binary, indent);
    return;

  case expr_kind_string:
    strcat(dest, "\"");
    strcat(dest, e->string);
    strcat(dest, "\"");
    return;

  case expr_kind_number: {
    char *buf = malloc(20);
    sprintf(buf, "%f", e->number);
    strcat(dest, buf);
    return;
  }

  default:
    strcat(dest, "EXPR");
  }
}

void toy_fmt_var(char *dest, struct var_t *head, int indent) {
  toy_fmt_indent(dest, indent);
  strcat(dest, "# ");
  strcat(dest, head->name);
  strcat(dest, " -> ");
  strcat(dest, head->value->type);
  strcat(dest, "\n");
}

void toy_fmt_vars(char *dest, struct var_t *head, int indent) {
  struct var_t *cursor = head;
  while (cursor != NULL) {
    toy_fmt_var(dest, cursor, indent);
    cursor = cursor->next;
  }
}

void toy_fmt_assign_block(char *dest, struct assign_block_t *assign_block,
                          int indent) {
  toy_fmt_ref(dest, assign_block->ref, indent);
  strcat(dest, " = ");
  toy_fmt_block(dest, assign_block->block, indent);
  strcat(dest, "\n");
}

void toy_fmt_assign_expr(char *dest, struct assign_expr_t *assign_expr,
                         int indent) {
  strcat(dest, assign_expr->var);
  strcat(dest, " = ");
  toy_fmt_expr(dest, assign_expr->expr, indent);
  strcat(dest, "\n");
}

void toy_fmt_return(char *dest, struct expr_t *e, int indent) {
  strcat(dest, "< ");
  toy_fmt_expr(dest, e, indent);
  strcat(dest, "\n");
}

void toy_fmt_stmt(char *dest, struct stmt_t *stmt, int indent) {
  toy_fmt_indent(dest, indent);
  switch (stmt->kind) {
  case stmt_kind_assign_block:
    toy_fmt_assign_block(dest, stmt->assign_block, indent);
    return;

  case stmt_kind_assign_expr:
    toy_fmt_assign_expr(dest, stmt->assign_expr, indent);
    return;

  case stmt_kind_expr:
    toy_fmt_expr(dest, stmt->expr, indent);
    strcat(dest, "\n");
    return;

  case stmt_kind_return:
    toy_fmt_return(dest, stmt->rtn, indent);
    return;

  default:
    strcat(dest, "STMT\n");
  }
}

void toy_fmt_stmts(char *dest, struct stmt_t **stmts, int size, int indent) {
  for (int i = 0; i < size; i++) {
    toy_fmt_stmt(dest, stmts[i], indent);
  }
}

void toy_fmt_block(char *dest, struct block_t *block, int indent) {
  strcat(dest, "{\n");
  toy_fmt_vars(dest, block->var_head, indent + 1);
  toy_fmt_stmts(dest, block->stmts, block->size, indent + 1);
  toy_fmt_indent(dest, indent);
  strcat(dest, "}\n");
}

void toy_fmt_program(char *dest, struct block_t *program) {
  toy_fmt_block(dest, program, 0);
}
