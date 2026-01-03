#include "vm.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


static int32_t pop(VM *vm)
{
  if (vm->sp <= 0) {
    fprintf(stderr, "Error: Stack Underflow\n");
    vm->running = 0;
    return 0;
  }
  return vm->stack[--vm->sp];
}

// Safety wrapper for push
static void push(VM *vm, int32_t v)
{
  if (vm->sp >= STACK_SIZE) {
    fprintf(stderr, "Error: Stack Overflow\n");
    vm->running = 0;
    return;
  }
  vm->stack[vm->sp++] = v;
}

void vm_init(VM *vm)
{
  // Reset registers
  vm->sp = 0;
  vm->pc = 0;
  vm->csp = 0;
  vm->running = 1;
  
  // Clear memory areas for deterministic execution
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
  // Read up to CODE_SIZE bytes
  size_t read = fread(vm->code, 1, CODE_SIZE, f);
  if (read == 0) {
      fprintf(stderr, "Warning: Empty bytecode file.\n");
  }
  fclose(f);
}

void vm_run(VM *vm)
{
  while (vm->running)
  {
    // Fetch opcode
    if (vm->pc >= CODE_SIZE) {
        fprintf(stderr, "Error: PC out of bounds\n");
        vm->running = 0;
        break;
    }
    
    uint8_t op = vm->code[vm->pc++];

    switch (op)
    {
    case OP_PUSH:
    {
      // Read 4 bytes as int32
      int32_t v = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      push(vm, v);
      break;
    }
    case OP_POP:
      pop(vm);
      break;

    case OP_DUP:
    {
       if (vm->sp <= 0) {
           fprintf(stderr, "Error: Stack Underflow on DUP\n");
           vm->running = 0;
       } else {
           push(vm, vm->stack[vm->sp - 1]);
       }
       break;
    }

    case OP_ADD:
    {
      int b = pop(vm);
      int a = pop(vm);
      push(vm, a + b);
      break;
    }
    case OP_SUB:
    {
      int b = pop(vm);
      int a = pop(vm);
      push(vm, a - b);
      break;
    }
    case OP_MUL:
    {
      int b = pop(vm);
      int a = pop(vm);
      push(vm, a * b);
      break;
    }
    case OP_DIV:
    {
      int b = pop(vm);
      int a = pop(vm);
      if (b == 0) {
          fprintf(stderr, "Error: Division by Zero\n");
          vm->running = 0;
      } else {
          push(vm, a / b);
      }
      break;
    }
    case OP_CMP:
    {
      int b = pop(vm);
      int a = pop(vm);
      push(vm, a < b); // Pushes 1 if a < b, else 0
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
      // 1 byte operand for index
      int idx = vm->code[vm->pc++]; 
      if (idx < 0 || idx >= MEM_SIZE) {
          fprintf(stderr, "Error: Memory Store OOB\n");
          vm->running = 0;
      } else {
          vm->memory[idx] = pop(vm);
      }
      break;
    }

    case OP_LOAD:
    {
      // 1 byte operand for index
      int idx = vm->code[vm->pc++];
      if (idx < 0 || idx >= MEM_SIZE) {
          fprintf(stderr, "Error: Memory Load OOB\n");
          vm->running = 0;
      } else {
          push(vm, vm->memory[idx]);
      }
      break;
    }

    case OP_CALL:
    {
      int addr = *(int32_t *)&vm->code[vm->pc];
      vm->pc += 4;
      if (vm->csp >= CALLSTACK_SIZE) {
          fprintf(stderr, "Error: Call Stack Overflow\n");
          vm->running = 0;
      } else {
          vm->callstack[vm->csp++] = vm->pc;
          vm->pc = addr;
      }
      break;
    }

    case OP_RET:
      if (vm->csp <= 0) {
          fprintf(stderr, "Error: Call Stack Underflow (Return from global?)\n");
          vm->running = 0;
      } else {
          vm->pc = vm->callstack[--vm->csp];
      }
      break;

    case OP_HALT:
      vm->running = 0;
      if (vm->sp > 0) {
         printf("VM HALT. Top of stack = %d\n", vm->stack[vm->sp - 1]);
      } else {
         printf("VM HALT. Stack empty.\n");
      }
      break;

    default:
      fprintf(stderr, "Invalid opcode: 0x%x at PC=%d\n", op, vm->pc - 1);
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
