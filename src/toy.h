#ifndef _TOY_H
#define _TOY_H

#include <stdlib.h>

typedef char token_kind_t;
#define TokenNumber 'N'
#define TokenString 'S'
#define TokenWord 'W'

typedef struct {
  token_kind_t kind;
  char *value;
} token_t;

typedef char *expr_type_t;

typedef struct {
  int size, cap, pos;
  token_t *tokens;
} tokens_t;

struct pair_t;

typedef struct {
  int size, cap;
  struct pair_t *pairs;
} pairs_t;

typedef struct {
  int size, cap;
  struct ref_pair_t *pairs;
} ref_pairs_t;

typedef struct {
  expr_type_t type;
  pairs_t pairs;
} call_t;

// Represents a "@[plus:(a)Number b:Number]: String"
struct ref_t {
  expr_type_t type; // "[plus:Number b:Number]"
  ref_pairs_t pairs;
  expr_type_t returnType; // "String"
};

struct expr_t;

struct assign_expr_t {
  char *var;
  struct expr_t *expr;
};

struct assign_block_t {
  struct ref_t ref;
  struct block_t *block;
};

struct binary_t {
  token_kind_t op;
  struct expr_t *left, *right;
};

typedef char expr_kind_t;
#define expr_kind_call 1
#define expr_kind_string 2
#define expr_kind_var 3
#define expr_kind_block 4
#define expr_kind_binary 5
#define expr_kind_number 6
#define expr_kind_ref 7

struct block_t;

struct expr_t {
  expr_kind_t kind;
  expr_type_t type;
  call_t call;
  char *string;
  char *var;
  struct block_t *block;
  struct binary_t binary;
  double number;
  struct ref_t ref;
};

struct pair_t {
  char *name;
  struct expr_t *expr;
};

struct ref_pair_t {
  char *name;
  char *alias;
  char *type;
};

typedef char stmt_kind_t;
#define stmt_kind_expr 1
#define stmt_kind_assign_block 2
#define stmt_kind_assign_expr 3
#define stmt_kind_return 4

struct stmt_t {
  stmt_kind_t kind;
  struct expr_t *expr;
  struct assign_block_t *assign_block;
  struct assign_expr_t *assign_expr;
  struct expr_t *rtn;
};

struct var_t {
  char *name;
  struct expr_t *value;
  struct var_t *next;
};

struct block_t {
  int size, cap;
  struct stmt_t **stmts;
  struct var_t *var_head;
  struct block_t *parent;
};

// fmt.c
char *expr_to_string(struct expr_t *expr);
char *call_to_string(call_t call);
void toy_fmt_program(char *dest, struct block_t *program);

// lexer.c
int has_more(tokens_t *tokens);
token_t peek(tokens_t *tokens);
tokens_t *tokenize(const char *s, int len);
void print_tokens(tokens_t *tokens);

// lib.c
typedef void (*lib_function_t)(struct expr_t *);
lib_function_t get_function(char *name);

// parser.c
struct block_t *parse(tokens_t *tokens);
char *call_type(call_t call);
struct expr_t *new_expr(expr_kind_t kind, expr_type_t type);
struct expr_t *find_variable(struct block_t *block, char *name);
struct var_t *all_variables(struct block_t *block);

// run.c
void set_variable(struct block_t *block, char *name, struct expr_t *value);
void run_program(struct block_t *block);

// utils.c
char *substr(const char *string, int position, int length);
char *read_file(const char *path);

#endif
