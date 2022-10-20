#include <stdio.h>
#include "compilator/compilator.h"

int getOp(FILE* input, int op[]);

int main() {
    FILE* input = fopen("../inputCode.txt", "r");
    compileFile(input, NULL);
    fclose(input);
    return 0;
}
