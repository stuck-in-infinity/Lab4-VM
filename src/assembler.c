#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Helper to write a 4-byte integer to the file
void write_int(FILE *out, int val) {
    fwrite(&val, sizeof(int), 1, out);
}

// Helper to write a 1-byte operand to the file
void write_byte(FILE *out, int val) {
    uint8_t b = (uint8_t)val;
    fwrite(&b, sizeof(uint8_t), 1, out);
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Usage: %s input.asm output.bc\n", argv[0]);
    return 1;
  }

  FILE *in = fopen(argv[1], "r");
  if (!in) {
      perror("Failed to open input file");
      return 1;
  }
  
  FILE *out = fopen(argv[2], "wb");
  if (!out) {
      perror("Failed to open output file");
      fclose(in);
      return 1;
  }

  char op[32];
  int val;

  while (fscanf(in, "%s", op) == 1)
  {
    // Data Movement
    if (!strcmp(op, "PUSH")) {
      fscanf(in, "%d", &val);
      fputc(OP_PUSH, out);
      write_int(out, val); // 4 bytes
    }
    else if (!strcmp(op, "POP")) {
      fputc(OP_POP, out);
    }
    else if (!strcmp(op, "DUP")) {
      fputc(OP_DUP, out);
    }

    // Arithmetic
    else if (!strcmp(op, "ADD")) fputc(OP_ADD, out);
    else if (!strcmp(op, "SUB")) fputc(OP_SUB, out);
    else if (!strcmp(op, "MUL")) fputc(OP_MUL, out);
    else if (!strcmp(op, "DIV")) fputc(OP_DIV, out);
    else if (!strcmp(op, "CMP")) fputc(OP_CMP, out);

    // Control Flow (Takes 4-byte address)
    else if (!strcmp(op, "JMP")) {
      fscanf(in, "%d", &val);
      fputc(OP_JMP, out);
      write_int(out, val);
    }
    else if (!strcmp(op, "JZ")) {
      fscanf(in, "%d", &val);
      fputc(OP_JZ, out);
      write_int(out, val);
    }
    else if (!strcmp(op, "JNZ")) {
      fscanf(in, "%d", &val);
      fputc(OP_JNZ, out);
      write_int(out, val);
    }

    // Memory & Functions
    else if (!strcmp(op, "STORE")) {
      fscanf(in, "%d", &val);
      fputc(OP_STORE, out);
      write_byte(out, val); // Stores take a 1-byte index
    }
    else if (!strcmp(op, "LOAD")) {
      fscanf(in, "%d", &val);
      fputc(OP_LOAD, out);
      write_byte(out, val); // Loads take a 1-byte index
    }
    else if (!strcmp(op, "CALL")) {
      fscanf(in, "%d", &val);
      fputc(OP_CALL, out);
      write_int(out, val);
    }
    else if (!strcmp(op, "RET")) {
      fputc(OP_RET, out);
    }
    
    // System 
    else if (!strcmp(op, "HALT")) {
      fputc(OP_HALT, out);
    }
    else {
      printf("Unknown instruction: %s\n", op);
      fclose(in);
      fclose(out);
      exit(1);
    }
  }

  fclose(in);
  fclose(out);
  return 0;
}
