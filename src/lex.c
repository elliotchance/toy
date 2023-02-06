#include "toy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tokens_t *new_tokens(int cap) {
  tokens_t *t = malloc(sizeof(tokens_t));
  t->size = 0;
  t->pos = 0;
  t->cap = cap;
  t->tokens = calloc(t->cap, sizeof(token_t));
  return t;
}

void push_token(tokens_t *tokens, token_kind_t kind, char *value) {
  if (tokens->size >= tokens->cap) {
    printf("tokens full!");
    exit(1);
  }
  tokens->tokens[tokens->size].kind = kind;
  tokens->tokens[tokens->size].value = value;
  ++tokens->size;
}

tokens_t *tokenize(const char *s, int len) {
  tokens_t *tokens = new_tokens(1000000);
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case ' ':
    case '\r':
    case '\n':
      continue;

    case '[':
    case ']':
    case '(':
    case ')':
    case '{':
    case '}':
    case '=':
    case ':':
    case '@':
    case '<':
    case '>':
    case '+':
    case '-':
    case '*':
    case '/':
      push_token(tokens, s[i], NULL);
      continue;

    case '"': {
      int start = ++i + 1;
      while (i < len && s[i] != '"') {
        i++;
      }
      push_token(tokens, TokenString, substr(s, start, i - start + 1));
      continue;
    }

    default: {
      int start = ++i;
      while (i < len &&
             ((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') ||
              (s[i] >= '0' && s[i] <= '9') || s[i] == '.')) {
        i++;
      }

      char *value = substr(s, start, i - start + 1);
      if (value[0] < '0' || value[0] > '9') {
        push_token(tokens, TokenWord, value);
      } else {
        push_token(tokens, TokenNumber, value);
      }
      i--;
    }
    }
  }
  return tokens;
}

void print_tokens(tokens_t *tokens) {
  for (int i = 0; i < tokens->size; i++) {
    printf("%04d %c %s\n", i + 1, tokens->tokens[i].kind,
           tokens->tokens[i].value);
  }
}

int has_more(tokens_t *tokens) { return tokens->pos < tokens->size - 1; }

token_t peek(tokens_t *tokens) { return tokens->tokens[tokens->pos]; }
