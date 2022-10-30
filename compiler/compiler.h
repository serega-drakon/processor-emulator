#ifndef PROCESSOR_EMULATOR_COMPILER_H
#define PROCESSOR_EMULATOR_COMPILER_H

#include <stdio.h>
#include "../memory/memory.h"

int compileFile(FILE* input, Stack* ptrProgram);

#endif //PROCESSOR_EMULATOR_COMPILER_H
