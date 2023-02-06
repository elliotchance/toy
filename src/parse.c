#include "toy.h"
#include <stdio.h>
#include <string.h>

struct expr_t *consume_expr(tokens_t *tokens, struct block_t *block);
void consume_block(tokens_t *tokens, struct block_t *block);

token_t consume(tokens_t *tokens, token_kind_t expected) {
  token_t token = tokens->tokens[tokens->pos++];
  if (token.kind != expected) {
    printf("error: expected %c, but found %c at %d\n", expected, token.kind,
           tokens->pos);
    exit(1);
  }
  return token;
}

struct ref_pair_t consume_ref_pair(tokens_t *tokens, struct block_t *block) {
  struct ref_pair_t result;
  result.name = result.alias = consume(tokens, TokenWord).value;
  consume(tokens, ':');
  if (peek(tokens).kind == '(') {
    ++tokens->pos; // skip (
    result.alias = consume(tokens, TokenWord).value;
    consume(tokens, ')');
  }
  result.type = consume(tokens, TokenWord).value;

  return result;
}

struct pair_t consume_pair(tokens_t *tokens, struct block_t *block) {
  struct pair_t result;
  result.name = consume(tokens, TokenWord).value;
  consume(tokens, ':');
  result.expr = consume_expr(tokens, block);

  return result;
}

struct expr_t *new_expr(expr_kind_t kind, expr_type_t type) {
  struct expr_t *e = malloc(sizeof(struct expr_t));
  e->kind = kind;
  e->type = type;

  return e;
}

struct expr_t *consume_call(tokens_t *tokens, struct block_t *block) {
  consume(tokens, '[');
  call_t call;
  call.pairs.size = 0;
  call.pairs.cap = 8;
  call.pairs.pairs = calloc(call.pairs.cap, sizeof(struct pair_t));

  char *type = malloc(100);
  type[0] = '[';
  type[1] = 0;

  for (int i = 0; peek(tokens).kind != ']'; i++) {
    if (i > 0) {
      strcat(type, " ");
    }
    struct pair_t pair = consume_pair(tokens, block);
    strcat(type, pair.name);
    strcat(type, ":");
    strcat(type, pair.expr->type);
    call.pairs.pairs[call.pairs.size++] = pair;
  }

  consume(tokens, ']');
  strcat(type, "]");

  struct expr_t *e = new_expr(expr_kind_call, type);
  e->call = call;
  e->type = "Number";

  return e;
}

char *call_type(call_t call) {
  char *s = malloc(100);
  s[0] = 0;

  strncat(s, "[", 1);
  for (int i = 0; i < call.pairs.size; i++) {
    if (i > 0) {
      strncat(s, " ", 1);
    }

    strncat(s, call.pairs.pairs[i].name, strlen(call.pairs.pairs[i].name));
    strncat(s, ":", 1);
    switch (call.pairs.pairs[i].expr->kind) {
    case expr_kind_var:
      strncat(s, call.pairs.pairs[i].expr->var,
              strlen(call.pairs.pairs[i].expr->var));
    }
  }
  strncat(s, "]", 1);

  return s;
}

// @[plus:(a)Number b:Number]: String
//
// ref_t:
//  pairs:
//    size: 2
//    cap: 8
//    pairs:
//      name: "plus"
//      alias: "a"
//      type: "Number"
//      name: "b"
//      alias: "b"
//      type: "Number"
//  returnType: "String"
struct ref_t consume_ref(tokens_t *tokens, struct block_t *block) {
  consume(tokens, '@');
  consume(tokens, '[');
  struct ref_t ref;
  ref.pairs.size = 0;
  ref.pairs.cap = 8;
  ref.pairs.pairs = calloc(ref.pairs.cap, sizeof(struct ref_t));

  char *t = malloc(100);
  t[0] = '[';
  t[1] = 0;

  for (int i = 0; peek(tokens).kind != ']'; i++) {
    if (i > 0) {
      strcat(t, " ");
    }
    struct ref_pair_t pair = consume_ref_pair(tokens, block);
    strcat(t, pair.name);
    strcat(t, ":");
    strcat(t, pair.type);
    ref.pairs.pairs[ref.pairs.size++] = pair;
  }

  consume(tokens, ']');
  strcat(t, "]");

  if (peek(tokens).kind == ':') {
    consume(tokens, ':');
    ref.returnType = consume(tokens, TokenWord).value;
  }

  ref.type = t;

  return ref;
}

struct expr_t *consume_binary_expr(tokens_t *tokens, struct block_t *block) {
  consume(tokens, '(');
  struct expr_t *e = new_expr(expr_kind_binary, "Number");
  e->binary.left = consume_expr(tokens, block);
  e->binary.op = tokens->tokens[tokens->pos++].kind;
  e->binary.right = consume_expr(tokens, block);
  consume(tokens, ')');

  return e;
}

struct expr_t *find_variable(struct block_t *block, char *name) {
  if (strcmp(name, "Number") == 0) {
    struct expr_t *type = new_expr(expr_kind_string, name);
    type->string = "Number";
    return type;
  }

  return NULL;

  struct var_t *cursor = block->var_head;
  while (cursor != NULL) {
    if (strcmp(cursor->name, name) == 0) {
      return cursor->value;
    }
    cursor = cursor->next;
  }

  return NULL;
}

struct var_t *new_var(char *name, struct expr_t *value) {
  struct var_t *v = malloc(sizeof(struct var_t));
  v->name = name;
  v->value = value;
  v->next = NULL;
  return v;
};

struct var_t *all_variables(struct block_t *block) {
  struct var_t *head = NULL;
  struct var_t *tail = NULL;
  struct var_t *cursor = block->var_head;

  while (block != NULL) {
    while (cursor != NULL) {
      if (head == NULL) {
        head = tail = new_var(cursor->name, cursor->value);
      } else {
        tail->next = new_var(cursor->name, cursor->value);
      }
      cursor = cursor->next;
    }

    block = block->parent;
  }

  return head;
}

struct expr_t *consume_expr(tokens_t *tokens, struct block_t *block) {
  switch (peek(tokens).kind) {
  case TokenWord: {
    char *varName = consume(tokens, TokenWord).value;
    struct expr_t *var = find_variable(block, varName);
    if (var == NULL) {
      struct expr_t *expr = new_expr(expr_kind_number, "Number");
      expr->number = 0;
      return expr;
      // printf("no such variable: %s\n", varName);
      // struct var_t *cursor = all_variables(block);
      // while (cursor != NULL) {
      //   printf("  %s", cursor->name);
      //   cursor = cursor->next;
      // }
      // exit(1);
    }
    struct expr_t *e = new_expr(expr_kind_var, var->type);
    e->var = varName;
    return e;
  }

  // case "Boolean":
  case TokenNumber: {
    struct expr_t *e = new_expr(expr_kind_number, "Number");
    e->number = atof(consume(tokens, TokenNumber).value);
    return e;
  }
  case TokenString: {
    struct expr_t *e = new_expr(expr_kind_string, "String");
    e->string = consume(tokens, TokenString).value;
    return e;
  }

  case '[':
    return consume_call(tokens, block);

  case '(':
    return consume_binary_expr(tokens, block);

    // case "{":
    // return consumeBlock(tokens, block);

    // case '@':
    //   return consume_ref(tokens, block);
  }

  printf("error at %c\n", peek(tokens).kind);
  exit(1);

  return NULL;
}

struct block_t *new_block(struct block_t *parent) {
  struct block_t *block = malloc(sizeof(struct block_t));
  block->size = 0;
  block->cap = 1000000;
  block->stmts = calloc(block->cap, sizeof(struct stmt_t *));
  block->var_head = NULL;
  block->parent = parent;

  return block;
}

void push_stmt(struct block_t *block, struct stmt_t *stmt) {
  if (block->size >= block->cap) {
    printf("block full!");
    exit(1);
  }
  block->stmts[block->size] = stmt;
  ++block->size;
}

struct stmt_t *new_stmt(stmt_kind_t kind) {
  struct stmt_t *stmt = malloc(sizeof(struct stmt_t));
  stmt->kind = kind;
  return stmt;
}

struct stmt_t *consume_return(tokens_t *tokens, struct block_t *block) {
  consume(tokens, '<');
  struct stmt_t *stmt = new_stmt(stmt_kind_return);
  stmt->rtn = consume_expr(tokens, block);
  return stmt;
}

struct assign_block_t *consume_assign_block(tokens_t *tokens,
                                            struct block_t *block) {
  struct ref_t ref = consume_ref(tokens, block);
  consume(tokens, '=');

  struct block_t *b = new_block(block);
  for (int i = 0; i < ref.pairs.size; i++) {
    struct expr_t *e = new_expr(expr_kind_number, ref.pairs.pairs[i].type);
    e->number = 0;
    set_variable(b, ref.pairs.pairs[i].alias, e);
  }

  consume_block(tokens, b);

  struct assign_block_t *assign = malloc(sizeof(struct assign_block_t));
  assign->ref = ref;
  assign->block = b;

  struct expr_t *e = new_expr(expr_kind_block, ref.returnType);
  e->block = b;
  set_variable(block, ref.returnType, e);

  return assign;
}

struct assign_expr_t *consume_assign_expr(tokens_t *tokens,
                                          struct block_t *block) {
  char *var = consume(tokens, TokenWord).value;
  consume(tokens, '=');
  struct expr_t *expr = consume_expr(tokens, block);

  struct assign_expr_t *assign = malloc(sizeof(struct assign_expr_t));
  assign->var = var;
  assign->expr = expr;

  set_variable(block, var, expr);

  return assign;
}

struct stmt_t *consume_stmt(tokens_t *tokens, struct block_t *block) {
  struct stmt_t *stmt = new_stmt(stmt_kind_expr);

  switch (peek(tokens).kind) {
  case '<':
    return consume_return(tokens, block);

  case '@':
    stmt->kind = stmt_kind_assign_block;
    stmt->assign_block = consume_assign_block(tokens, block);
    break;

  case TokenWord:
    stmt->kind = stmt_kind_assign_expr;
    stmt->assign_expr = consume_assign_expr(tokens, block);
    break;

  default:
    stmt->expr = consume_expr(tokens, block);
  }

  return stmt;
}

void consume_stmts(tokens_t *tokens, struct block_t *block) {
  while (has_more(tokens) && peek(tokens).kind != '}') {
    push_stmt(block, consume_stmt(tokens, block));
  }
}

void consume_block(tokens_t *tokens, struct block_t *block) {
  consume(tokens, '{');
  consume_stmts(tokens, block);
  consume(tokens, '}');
}

struct block_t *parse(tokens_t *tokens) {
  struct block_t *block = new_block(NULL);
  consume_stmts(tokens, block);
  return block;
}
