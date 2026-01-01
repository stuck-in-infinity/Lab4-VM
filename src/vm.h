#ifndef VM_H
#define VM_H

#include <stdint.h>

#define STACK_SIZE 1024
#define MEM_SIZE 256
#define CODE_SIZE 4096
#define CALLSTACK_SIZE 256

typedef struct
{
  int32_t stack[STACK_SIZE];
  int sp;

  int32_t memory[MEM_SIZE];

  uint8_t code[CODE_SIZE];
  int pc;

  int callstack[CALLSTACK_SIZE];
  int csp;

  int running;
} VM;

void vm_init(VM *vm);
void vm_load(VM *vm, const char *filename);
void vm_run(VM *vm);

#endif
