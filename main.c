#include <stdio.h>
#include "compiler/compiler.h"
#include "processor/processor.h"

int main() {
    FILE* input = fopen("../inputCode.txt", "r");
    Stack* ptrProgram = stackInit(1);
    Stack* ptrStack = stackInit(1);
    Stack* ptrVar = stackInit(1);

    u_int32_t bytesForVar;
    int status;

    printf("||%d:", status = compileFile(input, ptrProgram, &bytesForVar)); //debug
    for(int i = 0; i < getsize(ptrProgram); i++)
        printf("|%d ", *((char*)stack_r(ptrProgram,i)));
    printf("\n");

    if(!status)
        processor_main(ptrProgram, bytesForVar);

    stackFree(ptrProgram);
    stackFree(ptrStack);
    stackFree(ptrVar);
    fclose(input);
    return 0;
}
