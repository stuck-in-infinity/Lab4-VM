#include "vm.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>

static int32_t pop(VM *vm)
{
  return vm->stack[--vm->sp];
}

static void push(VM *vm, int32_t v)
{
  vm->stack[vm->sp++] = v;
}

void vm_init(VM *vm)
{
  vm->sp = 0;
  vm->pc = 0;
  vm->csp = 0;
  vm->running = 1;
}

void vm_load(VM *vm, const char *filename)
{
  FILE *f = fopen(filename, "rb");
  if (!f)
  {
    perror("bytecode");
    exit(1);
  }
  fread(vm->code, 1, CODE_SIZE, f);
  fclose(f);
}

void vm_run(VM *vm)
{
  while (vm->running)
  {
    uint8_t op = vm->code[vm->pc++];

    switch (op)
    {
    case OP_PUSH:
    {
      int32_t v = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      push(vm, v);
      break;
    }
    case OP_POP:
      pop(vm);
      break;

    case OP_DUP:
      push(vm, vm->stack[vm->sp - 1]);
      break;

    case OP_ADD:
    {
      int b = pop(vm), a = pop(vm);
      push(vm, a + b);
      break;
    }
    case OP_SUB:
    {
      int b = pop(vm), a = pop(vm);
      push(vm, a - b);
      break;
    }
    case OP_MUL:
    {
      int b = pop(vm), a = pop(vm);
      push(vm, a * b);
      break;
    }
    case OP_DIV:
    {
      int b = pop(vm), a = pop(vm);
      push(vm, a / b);
      break;
    }
    case OP_CMP:
    {
      int b = pop(vm), a = pop(vm);
      push(vm, a < b);
      break;
    }

    case OP_JMP:
      vm->pc = *(int32_t *)&vm->code[vm->pc];
      break;

    case OP_JZ:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      if (pop(vm) == 0)
        vm->pc = addr;
      break;
    }

    case OP_JNZ:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      if (pop(vm) != 0)
        vm->pc = addr;
      break;
    }

    case OP_STORE:
    {
      int idx = vm->code[vm->pc++];
      vm->memory[idx] = pop(vm);
      break;
    }

    case OP_LOAD:
    {
      int idx = vm->code[vm->pc++];
      push(vm, vm->memory[idx]);
      break;
    }

    case OP_CALL:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      vm->callstack[vm->csp++] = vm->pc;
      vm->pc = addr;
      break;
    }

    case OP_RET:
      vm->pc = vm->callstack[--vm->csp];
      break;

    case OP_HALT:
      vm->running = 0;
      printf("VM HALT. Top of stack = %d\n", vm->stack[vm->sp - 1]);
      break;

    default:
      fprintf(stderr, "Invalid opcode: 0x%x\n", op);
      vm->running = 0;
    }
  }
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Usage: %s program.bc\n", argv[0]);
    return 1;
  }
  VM vm;
  vm_init(&vm);
  vm_load(&vm, argv[1]);
  vm_run(&vm);
  return 0;
}
