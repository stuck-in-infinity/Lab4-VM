#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Usage: %s input.asm output.bc\n", argv[0]);
    return 1;
  }

  FILE *in = fopen(argv[1], "r");
  FILE *out = fopen(argv[2], "wb");

  char op[32];
  int val;

  while (fscanf(in, "%s", op) == 1)
  {
    if (!strcmp(op, "PUSH"))
    {
      fscanf(in, "%d", &val);
      fputc(OP_PUSH, out);
      fwrite(&val, 4, 1, out);
    }
    else if (!strcmp(op, "ADD"))
      fputc(OP_ADD, out);
    else if (!strcmp(op, "SUB"))
      fputc(OP_SUB, out);
    else if (!strcmp(op, "MUL"))
      fputc(OP_MUL, out);
    else if (!strcmp(op, "DIV"))
      fputc(OP_DIV, out);
    else if (!strcmp(op, "DUP"))
      fputc(OP_DUP, out);
    else if (!strcmp(op, "HALT"))
      fputc(OP_HALT, out);
    else
    {
      printf("Unknown instruction: %s\n", op);
      exit(1);
    }
  }

  fclose(in);
  fclose(out);
  return 0;
}
