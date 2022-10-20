#ifndef PROCESSOR_EMULATOR_MEMORY_H
#define PROCESSOR_EMULATOR_MEMORY_H

typedef struct Stack_ Stack;

void stackErrorPrint(Stack *ptrStack);
int stackErrorCheck(Stack *ptrStack);
void *stackInit(int size);
void *stack_r(Stack *ptrStack, int x);
void *stack_w(Stack *ptrStack, int x, void *ptrValue);
void *push(Stack *ptrStack, void *ptrValue);
void *pop(Stack *ptrStack);
void *getLast(Stack *ptrStack);
int getsize(Stack *ptrStack);
void stackFree(Stack *ptrStack);

#endif //PROCESSOR_EMULATOR_MEMORY_H
