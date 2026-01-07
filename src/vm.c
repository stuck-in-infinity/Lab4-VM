#include "vm.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- Stack helpers ---------- */

static int32_t pop(VM *vm)
{
  if (vm->sp <= 0)
  {
    fprintf(stderr, "Error: Stack Underflow\n");
    vm->error = 1;
    vm->running = 0;
    return 0;
  }
  return vm->stack[--vm->sp];
}

static void push(VM *vm, int32_t v)
{
  if (vm->sp >= STACK_SIZE)
  {
    fprintf(stderr, "Error: Stack Overflow\n");
    vm->error = 1;
    vm->running = 0;
    return;
  }
  vm->stack[vm->sp++] = v;
}

/* ---------- VM lifecycle ---------- */

void vm_init(VM *vm)
{
  vm->sp = vm->pc = vm->csp = 0;
  vm->running = 1;
  vm->error = 0;

  vm->instr_count = 0;
  vm->byte_count = 0;

  memset(vm->stack, 0, sizeof(vm->stack));
  memset(vm->memory, 0, sizeof(vm->memory));
  memset(vm->callstack, 0, sizeof(vm->callstack));
  memset(vm->code, 0, sizeof(vm->code));
}

void vm_load(VM *vm, const char *filename)
{
  FILE *f = fopen(filename, "rb");
  if (!f)
  {
    perror("Failed to load bytecode");
    exit(1);
  }
  fread(vm->code, 1, CODE_SIZE, f);
  fclose(f);
}

/* ---------- VM execution ---------- */

void vm_run(VM *vm)
{
  clock_t start = clock();

  while (vm->running)
  {
    uint8_t op = vm->code[vm->pc++];
    vm->instr_count++;
    vm->byte_count++;

    switch (op)
    {
    case OP_PUSH:
    {
      int32_t v = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      vm->byte_count += 4;
      push(vm, v);
      break;
    }

    case OP_POP:
      pop(vm);
      break;

    case OP_DUP:
      if (vm->sp <= 0)
      {
        fprintf(stderr, "Error: Stack Underflow on DUP\n");
        vm->error = 1;
        vm->running = 0;
      }
      else
      {
        push(vm, vm->stack[vm->sp - 1]);
      }
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
      if (b == 0)
      {
        fprintf(stderr, "Error: Division by zero\n");
        vm->error = 1;
        vm->running = 0;
      }
      else
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
      vm->byte_count += 4;
      break;

    case OP_JZ:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      vm->byte_count += 4;
      if (pop(vm) == 0)
        vm->pc = addr;
      break;
    }

    case OP_JNZ:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      vm->byte_count += 4;
      if (pop(vm) != 0)
        vm->pc = addr;
      break;
    }

    case OP_STORE:
    {
      int idx = vm->code[vm->pc++];
      vm->byte_count++;
      if (idx < 0 || idx >= MEM_SIZE)
      {
        fprintf(stderr, "Error: Memory Store OOB\n");
        vm->error = 1;
        vm->running = 0;
      }
      else
        vm->memory[idx] = pop(vm);
      break;
    }

    case OP_LOAD:
    {
      int idx = vm->code[vm->pc++];
      vm->byte_count++;
      if (idx < 0 || idx >= MEM_SIZE)
      {
        fprintf(stderr, "Error: Memory Load OOB\n");
        vm->error = 1;
        vm->running = 0;
      }
      else
        push(vm, vm->memory[idx]);
      break;
    }

    case OP_CALL:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      vm->byte_count += 4;
      vm->callstack[vm->csp++] = vm->pc;
      vm->pc = addr;
      break;
    }

    case OP_RET:
      vm->pc = vm->callstack[--vm->csp];
      break;

    case OP_HALT:
      vm->running = 0;
      if (!vm->error)
      {
        if (vm->sp > 0)
          printf("VM HALT. Top of stack = %d\n", vm->stack[vm->sp - 1]);
        else
          printf("VM HALT. Stack empty.\n");
      }
      break;

    default:
      fprintf(stderr, "Invalid opcode: 0x%x\n", op);
      vm->error = 1;
      vm->running = 0;
    }
  }

  clock_t end = clock();
  double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

  printf("\n=== VM BENCHMARK RESULTS ===\n");
  printf("Instructions executed : %lu\n", vm->instr_count);
  printf("Bytes executed        : %lu\n", vm->byte_count);
  printf("Execution time (sec)  : %f\n", time_spent);
}

/* ---------- main ---------- */

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
