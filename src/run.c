#include "toy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct expr_t *run_expr(struct block_t *block, struct expr_t *expr);

struct expr_t *run_call(struct block_t *block, call_t call, expr_type_t type) {
  // Execute each of the arguments.
  struct expr_t **args = calloc(call.pairs.size, sizeof(struct expr_t *));
  for (int i = 0; i < call.pairs.size; i++) {
    args[i] = run_expr(block, call.pairs.pairs[i].expr);
  }

  struct expr_t *b = find_variable(block, type);
  if (b != NULL) {
    printf("CALL: %s\n", call_to_string(call));
    struct expr_t *e = new_expr(expr_kind_number, "Number");
    e->number = 1.23;
    return e;
  }

  // Built-in function?
  lib_function_t f = get_function(type);
  if (f != NULL) {
    (*f)(args[0]);
    struct expr_t *e = new_expr(expr_kind_number, "Number");
    e->number = 1.23;
    return e;
  }

  printf("no such function: %s\n", type);
  struct var_t *cursor = all_variables(block);
  while (cursor != NULL) {
    printf("  %s", cursor->name);
    cursor = cursor->next;
  }
  exit(1);

  // lib_printLine_String(call.pairs.pairs[0].expr);

  //   // try {
  //     const b = findVariable(block, realType);
  //     console.log(stmt, realType, b);
  //     // stmt.pairs.map((p) => (b.vars[v] = args.shift()));
  //     return runBlock(b);
  //   // } catch (e) {}

  //   if (functions[realType]) {
  //     return functions[realType](args);
  //   }

  //   throw new Error(`no such function: ${realType}`);
}

struct expr_t *get_variable(struct block_t *block, char *name) {
  return block->var_head->value;
}

struct expr_t *run_expr(struct block_t *block, struct expr_t *expr) {
  switch (expr->kind) {
  case expr_kind_var:
    return get_variable(block, expr->var);
  case expr_kind_block:
    return expr;
  case expr_kind_string:
    return expr;
  // case "assign":
  //   return setVariable(block, stmt.name, runExpression(block, stmt.value));
  // case "break":
  //   block.break = true;
  //   return;
  case expr_kind_call:
    return run_call(block, expr->call, expr->type);
    // case "return":
    //   return (block.vars._return = runExpression(block, stmt.value));
    // case "while":
    //   return runWhile(block, stmt);
  }

  return NULL;
}

void set_variable(struct block_t *block, char *name, struct expr_t *value) {
  struct var_t *var = malloc(sizeof(struct var_t));
  var->name = name;
  var->value = value;

  if (block->var_head == NULL) {
    block->var_head = var;
    var->next = NULL;
  } else {
    struct var_t *head = block->var_head;
    block->var_head = var;
    var->next = head;
  }
}

void run_assign_block(struct block_t *block,
                      struct assign_block_t *assign_block) {
  struct expr_t *e = new_expr(expr_kind_block, "");
  e->block = assign_block->block;
  set_variable(block, assign_block->ref.type, e);
}

void run_stmt(struct block_t *block, struct stmt_t *stmt) {
  switch (stmt->kind) {
  case stmt_kind_assign_block:
    run_assign_block(block, stmt->assign_block);
    return;
  // case "break":
  //   block.break = true;
  //   return;
  case stmt_kind_expr:
    run_expr(block, stmt->expr);
    return;
    // case "return":
    //   return (block.vars._return = runExpression(block, stmt.value));
    // case "while":
    //   return runWhile(block, stmt);
  }
}

void run_block(struct block_t *block) {
  for (int i = 0; i < block->size; i++) {
    run_stmt(block, block->stmts[i]);
    // if (stmt.t === "return") {
    //   return block.vars._return;
    // }
  }
}

void run_program(struct block_t *block) { run_block(block); }
