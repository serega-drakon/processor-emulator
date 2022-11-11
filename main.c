#include <stdio.h>
#include "compiler/compiler.h"

int main() {
    FILE* input = fopen("../inputCode.txt", "r");
    Stack* ptrProgram = stackInit(1);
    Stack* ptrStack = stackInit(1);
    Stack* ptrVar = stackInit(1);

    printf("||%d:",compileFile(input, ptrProgram)); //debug
    for(int i = 0; i < getsize(ptrProgram); i++)
        printf("|%d ", *((char*)stack_r(ptrProgram,i)));

    stackFree(ptrProgram);
    stackFree(ptrStack);
    stackFree(ptrVar);
    fclose(input);
    return 0;
}
