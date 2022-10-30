#include "compiler.h"
#include "../memory/memory.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#define MAXOP 100

#define REG_CAPACITY 4

#define SIZE_POINTER 15
#define SIZE_CONST 4

#define ERROROP(name, op) do {                  \
printf("line %d: %s \n", lineNum + 1, #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return 1; } while(0)

#define ERROR(name) do {                \
printf("line %d: %s \n", lineNum + 1, #name);   \
return 1; } while(0)

#define PUSH_CONST(op) do{              \
const void* ptrRes = getConst(&op[1]);  \
for(int i = 0; i < SIZE_CONST; i++)     \
push(ptrProgram, (char*)ptrRes + i);    \
} while(0)

#define PUSH_PTR(op) do{                        \
const void* ptrRes = getPointer(op, &error);    \
if (error) ERROROP(err ptr, op);                  \
for(int i = 0; i < SIZE_POINTER; i++)           \
push(ptrProgram, (char*)ptrRes + i);            \
} while(0)

///Получает следующий операнд или оператор из файла\n
///Возвращает длину найденной строки\n
int getOp(FILE* input, int op[], unsigned int* ptrLineNum){ //works!
    int c; int i = 0; static char memory = 0;//помнит строку, которую пока не надо учитывать
    if(memory > 0) {
        (*ptrLineNum)++;
        memory--;
    }
    while((c = getc(input)) == ' ' || c == '\t' || c == '\n')
        if(c == '\n') (*ptrLineNum)++;
    if(c != EOF)
        op[i++] = c;
    while((c = getc(input)) != ' ' && c != '\t' && c != '\n' && c != EOF) {
        op[i] = c;
        i++;
    }
    if(c == '\n') memory++;
    op[i] = '\0';
    return i;
}

///Возвращает указатель на константу в 10-ной СС
u_int32_t* getConst10D(const int op[]){
    static u_int32_t num;
    num = 0;
    for(int i = 0; op[i] != '\0'; i++)
        num = num * 10 + op[i] - '0';
    return &num;
}

///Возвращает указатель на константу в 16-ной СС
u_int32_t* getConst16D(const int op[]){
    static u_int32_t num;
    num = 0;
    for(int i = 0; op[i] != '\0'; i++)
        num = num * 16 + op[i] - (isdigit(op[i]) ? '0' : 'A' - 10);
    return &num;
}

///Возвращает указатель на сформированную ссылку из 15 байт
char* getPointer(int op[], char* error){ //FIXME
    static char result[15]; //1 байт под тип и 4 под ссылку

    return result;
}

///Сравнивает строки
int compareStr(const int a[], const char b[]){
    int j;
    int flag = 1;
    for(j = 0; flag && a[j] != '\0' && b[j] != '\0'; j++) {
        if (a[j] != b[j]) flag = 0;
    }
    if(flag && a[j] == '\0' && b[j] == '\0')
        return 1;
    return 0;
}

///Сравнивает обычную строку и строку из стека с началом в index
int compareStrStack(const int a[], Stack* ptrStack, int index){
    int j;
    int flag = 1;
    for(j = 0; flag && a[j] != '\0' && *((int*)stack_r(ptrStack, index) + j) != '\0'; j++) {
        if (a[j] != *((int*)stack_r(ptrStack, index) + j)) flag = 0;
    }
    if(flag && a[j] == '\0' && *((int*)stack_r(ptrStack, index) + j) == '\0')
        return 1;
    return 0;
}

enum count_{
    CountOfRegs = 8,
    CountOfOperators2arg = 1,
    CountOfStackOperators = 2,
    CountOfStackArithmetics = 14,
    CountOfDefineVars = 2,
    CountOfProgramControlArg = 8,
    CountOfProgramControlNoArg = 2
};

enum encodingOps_{
    MOV_reg_reg = 1,
    MOV_reg_mem,
    MOV_mem_reg,
    MOV_reg_const,
    MOV_mem_const,
    PUSH_reg,
    POP_reg,
    ADD, SUB, INC, IMUL, IDIV, AND, OR, XOR, NOT, NEG, SHL, SHR, SHRL, CMP,
    DV, DA,
    JMP, JE, JZ, JG, JGE, JL, JLE, CALL,
    RET, END
};

enum encodingRegs_{
    AX = 0,
    BX,
    CX,
    DX,
    SI,
    DI,
    SP,
    DP
};

enum others_{
    NotDefined = -2, //It may be name of variable
    Error = -1, //it exactly is error
    Register = END + 1, //регистры по задумке не должны кодироваться, записал их для удобства вывода из getType()
    Const16 = Register + CountOfRegs,
    Const10,
    Pointer,
    Label
};

const char *operators_[] = {
        "MOV",
        "PUSH",
        "POP",
        "ADD",
        "SUB",
        "INC",
        "IMUL",
        "IDIV",
        "AND",
        "OR",
        "XOR",
        "NOT",
        "NEG",
        "SHL",
        "SHR",
        "SHRL",
        "CMP",
        "DV",
        "DA",
        "JMP",
        "JE",
        "JZ",
        "JG",
        "JGE",
        "JL",
        "JLE",
        "CALL",
        "RET",
        "END"
};

const char *registers_[] = { //емае емае пошло говно по трубам
        "ax",
        "bx",
        "cx",
        "dx",
        "si",
        "di",
        "sp",
        "dp"
};

///Отображает множество строк на множество целых чисел *три крутых смайлика*\n
///Ну или это словарь операторов и операндов, если по-человечески
int getType(int op[]){
    int i;
    if(op[0] == '$') { //const 16-digit
        if(op[1] == '\0')
            return Error;
        for (i = 1; op[i] != '\0'; i++)
            if (!isdigit(op[i]) && (op[i] < 'A' || op[i] > 'F')) return Error;
        return Const16;
    }
    if(op[0] == '!'){
        if(op[1] == '\0')
            return Error;
        for(i = 1; op[i] != '\0'; i++)
            if(!isdigit(op[i])) return Error;
        return Const10;
    }
    if(op[0] == '%'){ //reg
        for(i = 0; i < CountOfRegs; i++){
            if(compareStr(&op[1], registers_[i]))
                return Register + i;
        }
        return Error;
    }
    if(op[0] == '('){ //ptr
        int endFlag = 1;
        int count = 0; //кол-во ';'
        for(i = 1; op[i] != '\0' && endFlag && count < 3; i++){
            if(op[i] == ';')
                count++;
            else if(op[i] == ')')
                endFlag = 0;
            else if(op[i] == '(')
                return Error;
        }
        if(op[i] == '\0' && !endFlag)
            return Pointer;
        else
            return Error;
    }
    if(op[0] == '.')
        return Label;
    if(compareStr(op, operators_[0]))
        return MOV_reg_reg;
    if(compareStr(op, operators_[1]))
        return PUSH_reg;
    if(compareStr(op, operators_[2]))
        return POP_reg;
    for(i = 0; i < 26; i++){
        if(compareStr(op, operators_[3 + i]))
            return ADD + i; //возвращает код операнда
    }
    return NotDefined;
}

int compileFile(FILE* input, Stack* ptrProgram){
    int op [ MAXOP ];
    int type;
    unsigned int lineNum = 0;

    Stack* ptrVariableNames = stackInit(MAXOP * sizeof(int));   //имена переменных
    Stack* ptrVariableValues = stackInit(REG_CAPACITY);         //величины ссылок на переменные
    Stack* ptrArrayNames = stackInit(MAXOP * sizeof(int));      //имена массивов
    Stack* ptrArrayValues = stackInit(REG_CAPACITY);            //величины ссылок на начала массивов
    Stack* ptrLabelNames = stackInit(MAXOP * sizeof(int));      //имена меток
    Stack* ptrLabelValues = stackInit(REG_CAPACITY);            //величины ссылок меток

    int32_t varCounter = 0; //содержит кол-во уже занятых битов переменными (= указатель на след свободный бит)

    while(getOp(input, op, &lineNum) > 0){ //dont forget about NotDefined = -2!
        type = getType(op);
        if(type == MOV_reg_reg){ //mov
            char error = 0;
            //FIXME
        }
        else if(type == PUSH_reg || type == POP_reg){ //stack ops
            char code = (char) type;
            push(ptrProgram, &code);
            if(getOp(input, op, &lineNum) > 0) {
                type = getType(op);
                if(type == Register){
                    code = (char) (type - Register); //num of reg
                    push(ptrProgram, &code);
                }
                else
                    ERROROP(Invalid argument, op);
            }
            else
                ERROROP(there are no arg, op);
        }
        else if(type >= ADD && type <= CMP){ //arithmetic ops
            char code = (char) type;
            push(ptrProgram, &code);
        }
        else if(type == DV){
            if(getOp(input, op, &lineNum) > 0){
                if(getType(op) != NotDefined) ERROROP(This is not name, op);
                char found = 0;
                for(int i = 0; i < getsize(ptrVariableNames) && !found; i++)
                    if(compareStrStack(op, ptrVariableNames, i)) found = 1;
                if(!found){
                    push(ptrVariableNames, op);
                    push(ptrVariableValues, &varCounter);
                    varCounter += sizeof(int32_t);
                }
                else
                    ERROROP(Variable is still defined, op);
            }
            else
                ERROR(there are no arg);
        }
        else if(type == DA){
            //FIXME
        }
        else if(type >= JMP && type <= CALL){
            char code = (char) type;
            push(ptrProgram, &code);
            if(getOp(input, op, &lineNum) > 0){
                type = getType(op);
                if(type == Label){
                    char found = 0;
                    int i;
                    for(i = 0; i < getsize(ptrLabelNames) && !found; i++)
                        if(compareStrStack(op, ptrLabelNames, i)) found = 1;
                    if(found){
                        const void* temp = stack_r(ptrLabelValues, i - 1);
                        for(i = 0; i < REG_CAPACITY; i++)
                            push(ptrProgram, (char*)temp + i);
                    }
                    else
                        ERROROP(Undefined label, op);
                }
                else
                    ERROROP(This is not a lable,op);
            }
            else
                ERROR(There are no operand);
        }
        else if(type == RET || type == END){
            char code = (char) type;
            push(ptrProgram, &code);
        }
        else if(type == Label){
            char found = 0;
            int i;
            int32_t temp;
            for(i = 0; i < getsize(ptrLabelNames) && !found; i++)
                if(compareStrStack(op, ptrLabelNames, i)) found = 1;
            if(found){
                temp = *((int32_t*)stack_r(ptrLabelValues, i - 1));
                for(i = 0; i < REG_CAPACITY; i++)
                    push(ptrProgram, (char*)&temp + i);
            }
            else{
                if(getsize(ptrProgram) < 0x00FFFFFF){
                    push(ptrLabelNames, op);
                    temp = getsize(ptrProgram);
                    push(ptrLabelValues, &temp);
                }
                else
                    ERROR(file is too large);
            }
        }
        else if(type == Const16){
            void* temp = getConst16D(&op[1]);
            for(int i = 0; i < REG_CAPACITY; i++)
                push(ptrProgram, (char*)temp + i);
        }
        else if(type == Const10){
            void* temp = getConst10D(&op[1]);
            for(int i = 0; i < REG_CAPACITY; i++)
                push(ptrProgram, (char*)temp + i);
        }
        else
            ERROROP(err, op);
    }

    /*for(int i = 0; i < getsize(ptrVariableValues); i++)           //debug
    printf("|%d ", *((int32_t*)stack_r(ptrVariableValues,i)));*/

    stackFree(ptrVariableNames);
    stackFree(ptrVariableValues);
    stackFree(ptrArrayNames);
    stackFree(ptrArrayValues);
    stackFree(ptrLabelNames);
    stackFree(ptrLabelValues);

    return 0;
}

/*
int compileFile(FILE* input, Stack* ptrProgram, Stack* ptrVar){
    int op [ MAXOP ];
    int type;
    char code;
    char error = 0;
    unsigned int lineNum = 1;
    while(getOp(input, op, &lineNum) > 0){
        type = getType(op);
        if(type >= Register && type < Register + CountOfRegs)   //обработка в печать 1 бита
        {
            push(ptrProgram, &type);
        }
        else if(type >= Operator2arg && type < Operator2arg + CountOfOperators2arg)//обработка оператор + 2 операнда
        {
            push(ptrProgram, &type);
            int type2;
            int op2 [ MAXOP ];
            if (getOp(input, op, &lineNum) > 0)
            {
                type = getType(op);
                if(type >= Register && type < Register + CountOfRegs){
                    code = REGISTER;
                    push(ptrProgram,&code);
                }
                else if(type == Const16){
                    ERROROP(err first op cannot be const, op);
                }
                else if(type == Pointer){
                    code = POINTER;
                    push(ptrProgram, &code);
                }
                else ERROROP(err, op);
            }
            else
                ERROROP(err, op);
            if (getOp(input, op2, &lineNum) > 0)
            {
                type2 = getType(op2);
                if(type2 >= Register && type2 < Register + CountOfRegs){
                    code = REGISTER;
                    push(ptrProgram,&code);
                }
                else if(type2 == Const16 && type != Const16){
                    code = CONST;
                    push(ptrProgram, &code);
                }
                else if(type2 == Pointer && type != Pointer){
                    code = POINTER;
                    push(ptrProgram, &code);
                }
                else ERROROP(err, op2);
            }
            else
                ERROROP(err, op2);

            if(type >= Register && type < Register + CountOfRegs)
                push(ptrProgram, &type);
            else if(type == Const16)
                PUSH_CONST(op);
            else if(type == Pointer)
                PUSH_PTR(op);

            if(type2 >= Register && type2 < Register + CountOfRegs)
                push(ptrProgram, &type2);
            else if(type2 == Const16 && type != Const16)
                PUSH_CONST(op2);
            else if(type2 == Pointer && type != Pointer)
                PUSH_PTR(op2);

        }
        else if(type == Const16){
            PUSH_CONST(op);
        }
        else if(type == Pointer){
            PUSH_PTR(op);
        }
        else if(type == Error){
            ERROROP(err unknown op, op);
        }
        else assert(0);
    }
    return 0;
}
*/