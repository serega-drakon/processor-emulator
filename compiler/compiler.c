#include "compiler.h"
#include "../encoding/encodings.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#define MAXOP 100
#define NONE (-1)

///Вывод ошибки name с выводом операнда op, возвращает 1 (с выводом строки)
#define PRINT_ERROROP_1(name, op) do {                  \
printf("line %d: %s \n", lineNum + 1, #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return 1; } while(0)

///Вывод ошибки name с выводом операнда op, возвращает NULL (с выводом строки)
#define PRINT_ERROROP2_1(name, op) do {                  \
printf("line %d: %s \n", *lineNum + 1, #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return 1; } while(0)

///Вывод ошибки name без вывода операнда (с выводом строки)
#define PRINT_ERROR(name) do {                \
printf("line %d: %s \n", lineNum + 1, #name);   \
return 1; } while(0)

///Вывод ошибки name без вывода операнда (с выводом строки)
#define PRINT_ERROR2(name) do {                \
printf("line %d: %s \n", *lineNum + 1, #name);   \
return 1; } while(0)


///Вывод ошибки name с выводом операнда op, возвращает NULL \n
///Использую в функции getPointer
#define PRINT_ERROROPNL_1(name, op) do {                  \
printf("%s \n", #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return 1; } while(0)

///Представления команд
const char *operators_[] = {
        "MOV",
        "PUSH",
        "POP",
        "ADD",
        "SUB",
        "INC",
        "DEC",
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
        "JMP",
        "JE",
        "JNE",
        "JZ",
        "JG",
        "JGE",
        "JL",
        "JLE",
        "CALL",
        "RET",
        "END",
        "PRINT",
        "QUAD",
        "DV",
        "DA",
};

///Коды соответствующих операторов в operators_[]
enum strOps_{
    strMOV,
    strPUSH,
    strPOP,
    strADD,
    strSUB,
    strINC,
    strDEC,
    strIMUL,
    strIDIV,
    strAND,
    strOR,
    strXOR,
    strNOT,
    strNEG,
    strSHL,
    strSHR,
    strSHRL,
    strCMP,
    strJMP,
    strJE,
    strJNE,
    strJZ,
    strJG,
    strJGE,
    strJL,
    strJLE,
    strCALL,
    strRET,
    strEND,
    strPRINT,
    strQUAD,
    strDV,
    strDA,
    MAXstrOP
};

///Представления регистров %..
const char *registers_[] = { //емае пошло говно по трубам
        "ax",
        "bx",
        "cx",
        "dx",
        "si",
        "di",
        "sp",
        "dp"
};

///Сравнивает строки    \n
///Возвращет 1 если совпали, иначе - 0
int compareStrIntChar(const int a[], const char b[]){
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
int compareStrStack(const int a[], Stack* ptrStack, u_int32_t index){
    int j;
    j = 0;
    while(a[j] != '\0' && *stack_r_int32(ptrStack, index, j) != '\0'
          && a[j] == *stack_r_int32(ptrStack, index, j)) {
        j++;
    }
    if(a[j] == '\0' && *stack_r_int32(ptrStack, index, j) == '\0')
        return 1;
    return 0;
}

///Получает следующий операнд или оператор из файла\n
///Возвращает длину найденной строки\n
int getOp(FILE *input, unsigned int *ptrLineNum, int op[]) {
    int c;
    int i = 0;
    char hasEnclosedBraces = 0; // есть ли незакрытая скобка ()
    while((c = getc(input)) == ' ' || c == '\t' || c == '\n') {
        if (c == '\n') (*ptrLineNum)++;
    }
    if(c != EOF)
        op[i++] = c;
    if(c == '(') hasEnclosedBraces = 1;
    while(((c = getc(input)) != ' ' && c != '\t' && c != '\n' && c != EOF) || hasEnclosedBraces) {
        if(c == '(') hasEnclosedBraces = 1;
        else if(c == ')') hasEnclosedBraces = 0;
        op[i] = c;
        i++;
    }
    if(c == '\n')
        ungetc(c, input);
    op[i] = '\0';
    return i;
}

///Подфункция getType
int checkConst16(const int op[]){
    if(op[1] == '\0')
        return Error;
    for (int i = 1; op[i] != '\0'; i++)
        if (!isdigit(op[i]) && (op[i] < 'A' || op[i] > 'F')) return Error;
    return Const16;
}

///Подфункция getType
int checkConst10(const int op[]){
    if(op[1] == '\0')
        return Error;
    for(int i = 1; op[i] != '\0'; i++)
        if(!isdigit(op[i])) return Error;
    return Const10;
}

///Подфункция getType
int checkReg(const int op[]){
    for(int i = 0; i < CountOfRegs; i++){
        if(compareStrIntChar(&op[1], registers_[i]))
            return Register + i;
    }
    return Error;
}

///Подфункция getType
int checkPtr(const int op[]){
    int i;
    int endIsNotReached = 1;
    int count = 0; //кол-во ';'
    for(i = 1; op[i] != '\0' && endIsNotReached && count < 3; i++){
        if(op[i] == ';')
            count++;
        else if(op[i] == ')')
            endIsNotReached = 0;
        else if(op[i] == '(')
            return Error;
    }
    if(op[i] == '\0' && !endIsNotReached)
        return Pointer;
    else
        return Error;
}

///Сопоставляет список из строк кодировке
int checkOthers(const int op[]){
    int type;
    for(type = 0; type < MAXstrOP && !compareStrIntChar(op, operators_[type]); type++)
        ;
    switch (type) {
        case strMOV:
            return MOV_reg_reg;
        case strPUSH:
            return PUSH_reg;
        case strPOP:
            return POP_reg;
        case strADD:
            return ADD;
        case strSUB:
            return SUB;
        case strINC:
            return INC;
        case strDEC:
            return DEC;
        case strIMUL:
            return IMUL;
        case strIDIV:
            return IDIV;
        case strAND:
            return AND;
        case strOR:
            return OR;
        case strXOR:
            return XOR;
        case strNOT:
            return NOT;
        case strNEG:
            return NEG;
        case strSHL:
            return SHL;
        case strSHR:
            return SHR;
        case strSHRL:
            return SHRL;
        case strCMP:
            return CMP;
        case strJMP:
            return JMP_lbl;
        case strJE:
            return JE_lbl;
        case strJNE:
            return JNE_lbl;
        case strJZ:
            return JZ_lbl;
        case strJG:
            return JG_lbl;
        case strJGE:
            return JGE_lbl;
        case strJL:
            return JL_lbl;
        case strJLE:
            return JLE_lbl;
        case strCALL:
            return CALL_lbl;
        case strRET:
            return RET;
        case strEND:
            return END;
        case strPRINT:
            return PRINT_reg;
        case strQUAD:
            return QUAD;
        case strDV:
            return DV;
        case strDA:
            return DA;
        case MAXstrOP:
            return NotDefined;
        default:
            assert(0);
    }
}

///Отображает множество строк на множество целых чисел *три крутых смайлика*\n
///Ну или это словарь операторов и операндов, если по-человечески
int getType(const int op[]){
    if(op[0] == '$') //const 16-digit
        return checkConst16(op);
    if(op[0] == '!') //const 10-digit
        return checkConst10(op);
    if(op[0] == '%') //reg
        return checkReg(op);
    if(op[0] == '(') //ptr
        return checkPtr(op);
    if(op[0] == '.')
        return Label;
    int buffType = checkOthers(op);
    if(op[0] != '\0')
        return buffType;
    else
        return Nothing;
}

///Набор динамических массивов, которые я использую
struct Defines_{
    Stack* ptrVariableNames;    ///<имена переменных
    Stack* ptrVariableValues;   ///<величины ссылок на переменные
    Stack* ptrLabelDefinedNames;       ///<имена меток
    Stack* ptrLabelDefinedValues;      ///<величины ссылок меток
    Stack* ptrLabelUsedNames;
    Stack* ptrLabelUsedValuesPtr;
};

typedef struct Defines_ Defines;

///Конструктор
void definesInit(struct Defines_ *def){
    def->ptrVariableNames = stackInit(MAXOP * sizeof(int));   ///<имена переменных
    def->ptrVariableValues = stackInit(REG_SIZE);         ///<величины ссылок на переменные
    def->ptrLabelDefinedNames = stackInit(MAXOP * sizeof(int));      ///<имена меток
    def->ptrLabelDefinedValues = stackInit(sizeof(int32_t));            ///<величины ссылок меток
    def->ptrLabelUsedNames = stackInit(MAXOP * sizeof(int));
    def->ptrLabelUsedValuesPtr = stackInit(sizeof(Stack*));
}

///Деструктор
void definesFree(struct Defines_ *def){
    stackFree(def->ptrVariableNames);
    stackFree(def->ptrVariableValues);
    stackFree(def->ptrLabelDefinedNames);
    stackFree(def->ptrLabelDefinedValues);
    stackFree(def->ptrLabelUsedNames);
    const u_int32_t size = getsize(def->ptrLabelUsedValuesPtr);
    for(int i = 0; i < size; i++)
        stackFree(*(void**)stack_r(def->ptrLabelUsedValuesPtr, i));
    stackFree(def->ptrLabelUsedValuesPtr);
}

///Смотрит по массиву есть ли данная "op" в списке и возвращает ее индекс, иначе возвращает NONE
int searchFor(Stack *ptrNames, int op[]){
    int i;
    for(i = 0; i < getsize(ptrNames); i++)
        if(compareStrStack(op, ptrNames, i)) return i;
    return NONE;
}

///Возвращает константу в 10-ной СС
u_int32_t getConst10D(const int op[]){
    u_int32_t value = 0;
    for(int i = 1; op[i] != '\0'; i++)
        value = value * 10 + op[i] - '0';
    return value;
}

///Возвращает константу в 16-ной СС
u_int32_t getConst16D(const int op[]){
    u_int32_t value = 0;
    for(int i = 1; op[i] != '\0'; i++)
        value = value * 16 + op[i] - (isdigit(op[i]) ? '0' : 'A' - 10);
    return value;
}

///Записывает метку в таблицу использованных меток
void addLabelUsage(struct Defines_ def, u_int32_t place, int op[]){
    int search = searchFor(def.ptrLabelUsedNames, op);
    Stack* ptrValue;
    if(search == NONE){
        push(def.ptrLabelUsedNames, op);
        ptrValue = stackInit(sizeof(int32_t));
        push(def.ptrLabelUsedValuesPtr, &ptrValue);
    }
    else
        ptrValue = *(void**)stack_r(def.ptrLabelUsedValuesPtr, search);

    push(ptrValue, &place);
}

///Пушит нули в данный стек
void pushZeros(Stack* ptrProgram, unsigned int count){
    const char zero = 0;
    for(int i = 0; i < count; i++)
        push(ptrProgram, (void*)&zero);
}

///Обработка метки
void processLabelUse(Stack* ptrProgram, Defines def, int op[]){
    addLabelUsage(def, getsize(ptrProgram), op);
    pushZeros(ptrProgram, sizeof(int32_t));
}

///Обработка 16-й константы
void processConst16D(Stack* ptrProgram, int op[]){ //задачу проверки сложил на функцию getType
    u_int32_t value = getConst16D(op);
    for(int i = 0; i < sizeof(int32_t); i++)
        push(ptrProgram, (char*)&value + i);
}

///Обработка 10-й константы
void processConst10D(Stack* ptrProgram, int op[]){
    u_int32_t value = getConst10D(op);
    for(int i = 0; i < sizeof(int32_t); i++)
        push(ptrProgram, (char*)&value + i);
}

///Обработка переменной
int processVarUse(Stack* ptrProgram, Defines def, int op[]){
    int search = searchFor(def.ptrVariableNames, op);
    if(search != NONE){
        char *ptrByteOfValue;
        for(u_int32_t i = 0; i < sizeof(int32_t); i++) {
            ptrByteOfValue = stack_r_char(def.ptrVariableValues, search, i);
            push(ptrProgram, ptrByteOfValue);
        }
    }
    else
        PRINT_ERROROPNL_1(Undefined variable, op);
    return 0;
}

///Обработка сложной адресации при данной метке (подфункция processPointer)
void processLabelPtr(Stack* ptrProgram, Defines def, u_int32_t codePos, int op[]){
    unsigned char code = Ptr_const_const_const;
    stack_w(ptrProgram, codePos, &code);
    processLabelUse(ptrProgram, def, op);
    pushZeros(ptrProgram, 4);
    code = 1;
    push(ptrProgram, &code);
    pushZeros(ptrProgram, 3);
}

///Обработка сложной адресации при данной переменной (подфункция processPointer)
int processNotDefPtr(Stack* ptrProgram, Defines def, u_int32_t codePos, int op[]){
    unsigned char code = Ptr_const_const_const;
    stack_w(ptrProgram, codePos, &code);
    if(processVarUse(ptrProgram, def, op))
        return 1;
    pushZeros(ptrProgram, 4);
    code = 1;
    push(ptrProgram, &code);
    pushZeros(ptrProgram, 3);
    return 0;
}

///Разбивает op[] на три операнда и достает их типы (подфункция processPointer)
int getOpsPtr(int op[], int buff1[], int buff2[], int buff3[]){
    int i;
    int j = 0;
    //достаю 1й операнд
    for(i = 1; op[i] == '\t' || op[i] == ' '; i++);
    for (; op[i] != ';' && op[i] != ')' && op[i] != ' ' && op[i] != '\t'; i++, j++)
        buff1[j] = op[i];
    buff1[j] = '\0';

    //достаю 2й операнд
    for(; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] == ';')
        for(i++; op[i] == '\t' || op[i] == ' '; i++);
    j = 0;
    if(op[i] != ')'){
        for (; op[i] != ';' && op[i] != ')' && op[i] != ' ' && op[i] !='\t'; i++, j++)
            buff2[j] = op[i];
    }
    buff2[j] = '\0';

    //достаю 3й операнд
    for(; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] == ';')
        for(i++; op[i] == '\t' || op[i] == ' '; i++);
    j = 0;
    if(op[i] != ')'){
        for (; op[i] != ';' && op[i] != ')' && op[i] != ' ' && op[i] != '\t'; i++, j++)
            buff3[j] = op[i];
        if(op[i] == ';') PRINT_ERROROPNL_1(Excess separator, op);
    }
    buff3[j] = '\0';
    return 0;
}

///Делает обработку по типу первого операнда (подфункция processPointer)
int processType1Ptr(Stack* ptrProgram, Defines def, int type1, char *code, int buff1[]){
    char temp;
    switch(type1){
        case RegAX: case RegBX: case RegCX: case RegDX:
        case RegSI: case RegDI: case RegSP: case RegDP:
            *code += 0b000;
            temp = (char)(type1 - Register);
            push(ptrProgram, &temp);
            break;
        case Label:
            *code += 0b100;
            processLabelUse(ptrProgram, def, buff1);
            break;
        case NotDefined:
            *code += 0b100;
            processVarUse(ptrProgram, def, buff1);
            break;
        default:
            PRINT_ERROROPNL_1(What the fuck 1st ?, buff1);
    }
    return 0;
}

///Делает обработку по типу второго операнда (подфункция processPointer)
int processType2Ptr(Stack *ptrProgram,int type2, char *code, int buff2[]){
    char temp;
    switch(type2){
        case RegAX: case RegBX: case RegCX: case RegDX:
        case RegSI: case RegDI: case RegSP: case RegDP:
            *code += 0b000;
            temp =  (char)(type2 - Register);
            push(ptrProgram, &temp);
            break;
        case Const16:
            *code +=0b010;
            processConst16D(ptrProgram, buff2);
            break;
        case Const10:
            *code +=0b010;
            processConst10D(ptrProgram, buff2);
            break;
        case Nothing:
            *code +=0b010;
            pushZeros(ptrProgram, 4);
            break;
        default:
            PRINT_ERROROPNL_1(What the fuck 2nd ?, buff2);
    }
    return 0;
}

///Делает обработку по типу третьего операнда (подфункция processPointer)
int processType3Ptr(Stack* ptrProgram, int type3, char *code, int buff3[]){
    char temp;
    switch(type3){
        case RegAX: case RegBX: case RegCX: case RegDX:
        case RegSI: case RegDI: case RegSP: case RegDP:
            *code += 0b000;
            temp = (char)(type3 - Register);
            push(ptrProgram, &temp);
            break;
        case Const16:
            *code +=0b001;
            processConst16D(ptrProgram, buff3);
            break;
        case Const10:
            *code +=0b001;
            processConst10D(ptrProgram, buff3);
            break;
        case Nothing:
            *code +=0b001;
            temp = 1;
            push(ptrProgram, &temp);
            pushZeros(ptrProgram, 3);
            break;
        default:
            PRINT_ERROROPNL_1(What the fuck 3rd ?, buff3);
    }
    return 0;
}

///Обрабатывает сложную адресацию, возвращает 0, если успешно, 1, если ошибка
int processPointer(Stack* ptrProgram, Defines def, const u_int32_t *lineNum, int op[MAXOP]){
    char code = 0;
    int type1;
    u_int32_t codePos = getsize(ptrProgram);

    push(ptrProgram, &code);

    type1 = getType(op);
    if(type1 == Label){
        processLabelPtr(ptrProgram, def, codePos, op);
        return 0;
    }
    if(type1 == NotDefined){ //переменная
        if(processNotDefPtr(ptrProgram, def, codePos, op))
            PRINT_ERROR2(Error ptr);
        return 0;
    }
    assert(type1 == Pointer);

    int type2, type3;
    int buff1[MAXOP], buff2[MAXOP], buff3[MAXOP];

    if(getOpsPtr(op, buff1, buff2, buff3)) PRINT_ERROR2(Error ops);

    type1 = getType(buff1);
    type2 = getType(buff2);
    type3 = getType(buff3);

    if(processType1Ptr(ptrProgram, def, type1, &code, buff1)) PRINT_ERROR2(Error ptr 1st op);

    if(processType2Ptr(ptrProgram, type2, &code, buff2)) PRINT_ERROR2(Error ptr 2nd op);

    if(processType3Ptr(ptrProgram, type3, &code, buff3)) PRINT_ERROR2(Error ptr 3nd op);

    stack_w(ptrProgram, codePos, &code);

    return 0;
}

///Получает код соответствующего мова для операции (подфункция processMov)
unsigned char getCodeMov(int type1, int type2, int* errorCheck){
    char code;
    if (type1 >= Register && type1 < Register + CountOfRegs) {
        if (type2 >= Register && type2 < Register + CountOfRegs)
            code = MOV_reg_reg;
        else if (type2 == Const16 || type2 == Const10 || type2 == Label)
            code = MOV_reg_const;
        else if (type2 == Pointer || type2 == NotDefined)
            code = MOV_reg_mem;
        else{
            code = 0;
            *errorCheck = 2;
        }
    } else if (type1 == Pointer || type1 == NotDefined) {
        if (type2 >= Register && type2 < Register + CountOfRegs)
            code = MOV_mem_reg;
        else if (type2 == Const16 || type2 == Const10 || type2 == Label)
            code = MOV_mem_const;
        else{
            code = 0;
            *errorCheck = 2;
        }
    } else{
        code = 0;
        *errorCheck = 1;
    }
    return code;
}

///Делает обработку по типу первого операнда (подфункция processMov)
int processType1Mov(Stack* ptrProgram, Defines def, int type1, u_int32_t *lineNum, int op[]){
    unsigned char code;
    switch (type1) {
        case RegAX: case RegBX: case RegCX: case RegDX:
        case RegSI: case RegDI: case RegSP: case RegDP:
            code = (unsigned char) (type1 - Register);
            push(ptrProgram, &code);
            break;
        case Pointer:
        case NotDefined:
            if(processPointer(ptrProgram, def, lineNum, op))
                PRINT_ERROROP2_1(Error ptr, op);
            break;
        default:
            assert(0);
    }
    return 0;
}

///Делает обработку по типу второго операнда (подфункция processMov)
int processType2Mov(Stack* ptrProgram, Defines def, int type2, u_int32_t *lineNum, int op2[]){
    unsigned char code;
    switch (type2) {
        case RegAX: case RegBX: case RegCX: case RegDX:
        case RegSI: case RegDI: case RegSP: case RegDP:
            code = (unsigned char) (type2 - Register);
            push(ptrProgram, &code);
            break;
        case Const16:
            processConst16D(ptrProgram, op2);
            break;
        case Const10:
            processConst10D(ptrProgram, op2);
            break;
        case Pointer:
        case NotDefined:
            if(processPointer(ptrProgram, def, lineNum, op2))
                PRINT_ERROROP2_1(Error ptr, op2);
            break;
        case Label:
            processLabelUse(ptrProgram, def, op2);
            break;
        default:
            assert(0);
    }
    return 0;
}

///Обработка MOV
int processMov(FILE* input, Stack* ptrProgram, Defines def, unsigned int* lineNum){
    int op[MAXOP];
    int op2[MAXOP];
    int type1;
    int type2;
    unsigned char code;
    int errorCheck = 0;

    if (getOp(input, lineNum, op) > 0) {
        type1 = getType(op);
    } else
        PRINT_ERROR2(There are no operand);

    if (getOp(input, lineNum, op2) > 0) {
        type2 = getType(op2);
    } else
        PRINT_ERROR2(There are no operand);

    code = getCodeMov(type1, type2, &errorCheck);

    if(errorCheck == 1) PRINT_ERROROP2_1(Invalid 1 argument to MOV, op);
    else if(errorCheck == 2) PRINT_ERROROP2_1(Invalid 2 argument to MOV, op2);

    push(ptrProgram, &code);

    if(processType1Mov(ptrProgram, def, type1, lineNum, op)) PRINT_ERROR2(Error mov 1st op);

    if(processType2Mov(ptrProgram, def, type2, lineNum, op2)) PRINT_ERROR2(Error mov 2nd op);

    return 0;
}

///Обработка PUSH и POP
int processPushPop(FILE* input, Stack* ptrProgram, unsigned int* lineNum, int type){
    int op[MAXOP];
    unsigned char code = (char) type;
    push(ptrProgram, &code);
    if (getOp(input, lineNum, op) > 0) {
        type = getType(op);
        if (type >= Register && type < Register + CountOfRegs) {
            code = (char) (type - Register); //num of reg
            push(ptrProgram, &code);
        } else
            PRINT_ERROROP2_1(Invalid argument, op);
    } else
        PRINT_ERROROP2_1(there are no arg, op);
    return 0;
}

///Обработка однобайтовых операций
void processOneByte(Stack* ptrProgram, int type){
    unsigned char code = (char) type;
    push(ptrProgram, &code);
}

///Обработка JMP
int processJmp(FILE* input, Stack* ptrProgram, Defines def, unsigned int* lineNum, int type){
    int op[MAXOP];
    unsigned char code = (char) type;
    if (getOp(input, lineNum, op) > 0) {
        type = getType(op);
        if (type == Label) {
            push(ptrProgram, &code);
            processLabelUse(ptrProgram, def, op);
        } else if (type == Pointer || type == NotDefined) {
            code = code - JMP_lbl + JMP_ptr;
            push(ptrProgram, &code);
            if(processPointer(ptrProgram, def, lineNum, op))
                PRINT_ERROROP2_1(Error ptr, op);
        } else
            PRINT_ERROROP2_1(This is not a lable or pointer, op);
    } else
        PRINT_ERROR2(There are no operand);
    return 0;
}

///Подфункция для processDV и processDA
int processVarDef(Defines  def,int op[], const u_int32_t varCounter) {
    u_int32_t value;
    u_int32_t search;
    if (getType(op) != NotDefined) PRINT_ERROROPNL_1(This is not name, op);
    search = searchFor(def.ptrVariableNames, op);
    if (search == NONE) {
        value = varCounter | 0xFF000000;
        push(def.ptrVariableNames, op);
        push(def.ptrVariableValues, &value);
        return 0;
    } else
        PRINT_ERROROPNL_1(Variable has already been defined, op);
}

///Обработка определения переменных
int processDV(FILE* input, Defines def, unsigned int *lineNum, u_int32_t *varCounter){
    int op[MAXOP];

    if (getOp(input, lineNum, op) > 0) {
        if(processVarDef(def, op, *varCounter))
            PRINT_ERROR2(Error DV);
        *varCounter += sizeof(int32_t);
    }
    else
        PRINT_ERROR2(there are no arg);
    return 0;
}

///Обработка определения массивов
int processDA(FILE* input, Defines def, unsigned int *lineNum, u_int32_t *varCounter) {
    int op[MAXOP];
    int type;

    if (getOp(input, lineNum, op) > 0){
        if (processVarDef(def, op, *varCounter))
            PRINT_ERROR2(Error DA);
    }else
        PRINT_ERROR2(there are no arg: name of variable);

    if (getOp(input, lineNum, op) > 0) {
        u_int32_t tempConst;

        if ((type = getType(op)) == Const10)
            tempConst = getConst10D(op);
        else if (type == Const16)
            tempConst = getConst16D(op);
        else
            PRINT_ERROROP2_1(This is not count, op);

        if (tempConst == 0) PRINT_ERROROP2_1(It cant be zero value, op);
        *varCounter += tempConst * sizeof(int32_t);
        if ((*varCounter) > 0x00FFFFFF) PRINT_ERROR2(Too many variables);
    } else
        PRINT_ERROR2(there are no arg: count of elements);
    return 0;
}

///Обработка определения меток
int processLabelDef(Stack *ptrProgram, Defines def, const unsigned int *lineNum, int op[]){
    u_int32_t search = searchFor(def.ptrLabelDefinedNames, op);
    u_int32_t value;
    if(search == NONE){
        if (getsize(ptrProgram) < 0x00FFFFFF) {
            push(def.ptrLabelDefinedNames, op);
            value = getsize(ptrProgram);
            push(def.ptrLabelDefinedValues, &value);
        } else
            PRINT_ERROR2(file is too large);
    } else
        PRINT_ERROROP2_1(Label has already been defined, op);
    return 0;
}

///Обработка QUAD
int processQuad(FILE* input, Stack* ptrProgram,Defines def, unsigned int *lineNum){
    int op[MAXOP];
    int type;
    if(getOp(input, lineNum, op) > 0){
        type = getType(op);
        switch(type){
            case Const16:
                processConst16D(ptrProgram, op);
                break;
            case Const10:
                processConst10D(ptrProgram, op);
                break;
            case Label:
                processLabelUse(ptrProgram, def, op);
                break;
            case NotDefined:
                if(processVarUse(ptrProgram, def, op))
                    PRINT_ERROROP2_1(Unknown variable, op);
                break;
            default:
                PRINT_ERROROP2_1(Invalid arg for QUAD, op);
        }
        return 0;
    }
    else
        PRINT_ERROR2(There are no arg for QUAD);
}

int processPrintReg(FILE* input, Stack* ptrProgram, unsigned  int *lineNum){
    int op[MAXOP];
    int type;
    unsigned char code = PRINT_reg;
    push(ptrProgram, &code);
    if(getOp(input, lineNum, op) > 0){
        type = getType(op);
        switch(type){
            case RegAX: case RegBX: case RegCX: case RegDX:
            case RegSI: case RegDI: case RegSP: case RegDP:
                code = (unsigned char) (type - Register);
                push(ptrProgram, &code);
                break;
            default:
                PRINT_ERROROP2_1(Invalid arg for PRINT, op);
        }
    }
    else
        PRINT_ERROR2(There are no arg for PRINT);
    return 0;
}

///Сверка таблиц использованных и объявленных меток
int checkLabelTable(Stack* ptrProgram, Defines def, const unsigned int *lineNum){
    u_int32_t size = getsize(def.ptrLabelUsedNames);
    u_int32_t place;
    u_int32_t value;
    u_int32_t search;
    void *ptrOp;
    Stack* ptrStackPlaces;

    for(int i = 0; i < size; i++) {
        ptrOp = stack_r(def.ptrLabelUsedNames, i);
        search = searchFor(def.ptrLabelDefinedNames, ptrOp);
        if(search == NONE) PRINT_ERROROP2_1(unknown label, ((int*)ptrOp)); //здесь немного кривое сообщение об ошибке

        value = *(u_int32_t*)stack_r(def.ptrLabelDefinedValues, search);

        ptrStackPlaces = *(void**)stack_r(def.ptrLabelUsedValuesPtr, i);
        while(getsize(ptrStackPlaces) > 0){
            place = *(u_int32_t*)pop(ptrStackPlaces);
            for(int j = 0; j < sizeof(int32_t); j++)
                stack_w(ptrProgram, place + j, (char*)&value + j);
        }
    }
    return 0;
}

///Основная функция, вызывает все остальные
int compileFile(FILE *input, Stack *ptrProgram, u_int32_t *ptrBytesForVar) {
    int op[MAXOP];
    int type;
    unsigned int lineNum = 0;
    char endFlag = 0;

    struct Defines_ def;
    definesInit(&def);

    *ptrBytesForVar = 0; ///<содержит кол-во уже занятых байтов переменными (= указатель на след свободный бит)

    while(getOp(input, &lineNum, op) > 0) {
        type = getType(op);
        switch (type) {
            case MOV_reg_reg: //операнды с переменной длиной 3-21 байт
                if(processMov(input, ptrProgram, def, &lineNum))
                    PRINT_ERROR(Error mov);
                break;
            case PUSH_reg:
            case POP_reg: //операнды с фиксированной длиной, состоящие из 2-х байт
                if(processPushPop(input, ptrProgram, &lineNum, type))
                    PRINT_ERROR(Error push pop);
                break;
            case ADD: case SUB: case INC: case DEC: case IMUL: case IDIV: case AND:
            case OR: case XOR: case NOT: case NEG: case SHL: case SHR: case SHRL:
            case CMP: case RET: //однобайтовые операнды
                processOneByte(ptrProgram, type);
                break;
            //операнды с переменной длиной, весят по 5 байт в случае если переход по метке, 14 байт, если переход по ссылке
            case JMP_lbl: case JE_lbl: case JNE_lbl: case JZ_lbl: case JG_lbl:
            case JGE_lbl: case JL_lbl: case JLE_lbl: case CALL_lbl:
                if(processJmp(input, ptrProgram, def, &lineNum, type))
                    PRINT_ERROR(Error Jmp);
                break;
            case DV: //инициализация переменных
                if(processDV(input, def, &lineNum, ptrBytesForVar))
                    PRINT_ERROR(Error DV);
                break;
            case DA: //инициализация массивов
                if(processDA(input, def, &lineNum, ptrBytesForVar))
                    PRINT_ERROR(Error DA);
                break;
            case Label:
                if(processLabelDef(ptrProgram, def, &lineNum, op))
                    PRINT_ERROR(Error label);
                break;
            case QUAD:
                if(processQuad(input, ptrProgram, def, &lineNum))
                    PRINT_ERROR(Error Quad);
                break;
            case END:
                processOneByte(ptrProgram, type);
                endFlag = 1;
                break;
            case PRINT_reg:
                if(processPrintReg(input, ptrProgram, &lineNum))
                    PRINT_ERROR(Error PRINT_reg);
                break;
            default:
                PRINT_ERROROP_1(Unknown operator, op);
        }
    }

    if(checkLabelTable(ptrProgram, def, &lineNum))
        PRINT_ERROR(Error label table);

    if(!endFlag) PRINT_ERROR(No END);

    definesFree(&def);
    return 0;
}