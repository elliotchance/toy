#include <stdio.h>
#include <stdlib.h>

char *substr(const char *string, int position, int length) {
  int c;
  char *p = malloc(length + 1);

  for (c = 0; c < length; c++) {
    *(p + c) = *(string + position - 1);
    string++;
  }

  *(p + c) = '\0';

  return p;
}

char *read_file(const char *path) {
  FILE *infile = fopen(path, "r");
  if (infile == NULL) {
    printf("error: cannot open file: %s", path);
    return NULL;
  }

  fseek(infile, 0L, SEEK_END);
  long size = ftell(infile);
  fseek(infile, 0L, SEEK_SET);

  char *buffer = (char *)calloc(size + 1, sizeof(char));
  fread(buffer, sizeof(char), size, infile);
  fclose(infile);

  buffer[size] = 0;
  return buffer;
}
