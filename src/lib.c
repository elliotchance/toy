#include "toy.h"
#include <stdio.h>
#include <string.h>

void lib_printLine_String(struct expr_t *e) {
  printf("%s\n", expr_to_string(e));
}

void lib_print_String(struct expr_t *e) { printf("%s", expr_to_string(e)); }

lib_function_t get_function(char *name) {
  if (strcmp(name, "[printLine:String]") == 0) {
    return &lib_printLine_String;
  }

  if (strcmp(name, "[printLine:Number]") == 0) {
    return &lib_printLine_String;
  }

  if (strcmp(name, "[print:String]") == 0) {
    return &lib_print_String;
  }

  return NULL;
}
