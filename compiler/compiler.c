#include "compiler.h"
#include "../encoding/encodings.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#define MAXOP 100
#define REG_SIZE 4
#define POINTER_SIZE 13
#define NONE (-1)

///Вывод ошибки name с выводом операнда op, возвращает 1
#define ERROROP(name, op) do {                  \
printf("line %d: %s \n", lineNum + 1, #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return 1; } while(0)

///Вывод ошибки name с выводом операнда op, возвращает NULL \n
///Использую в функции getPointer
#define ERROROP2(name, op) do {                  \
printf("line %d: %s \n", lineNum + 1, #name);   \
for (int index = 0; op[index] != '\0'; index++) \
    printf("%c", op[index]);                    \
printf("\n");                                   \
return NULL; } while(0)

///Вывод ошибки name без вывода операнда
#define ERROR(name) do {                \
printf("line %d: %s \n", lineNum + 1, #name);   \
return 1; } while(0)

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

///Получает следующий операнд или оператор из файла\n
///Возвращает длину найденной строки\n
int getOp(FILE* input, int op[], unsigned int* ptrLineNum){ //works!
    int c;
    int i = 0;
    char flag = 0; // есть ли незакрытая скобка ()
    static char memory = 0;//помнит строку, которую пока не надо учитывать
    if(memory > 0) {
        (*ptrLineNum)++;
        memory--;
    }
    while((c = getc(input)) == ' ' || c == '\t' || c == '\n') {
        if (c == '\n') (*ptrLineNum)++;
    }
    if(c != EOF)
        op[i++] = c;
    if(c == '(') flag = 1;
    while(((c = getc(input)) != ' ' && c != '\t' && c != '\n' && c != EOF) || flag) {
        if(c == '(') flag = 1;
        else if(c == ')') flag = 0;
        op[i] = c;
        i++;
    }
    if(c == '\n') memory++;
    op[i] = '\0';
    return i;
}

///Отображает множество строк на множество целых чисел *три крутых смайлика*\n
///Ну или это словарь операторов и операндов, если по-человечески
int getType(const int op[]){
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
    for(i = 0; i < 24; i++){
        if(compareStr(op, operators_[3 + i]))
            return ADD + i; //возвращает код операнда
    }
    if(compareStr(op, operators_[27]))
        return RET;
    if(compareStr(op, operators_[28]))
        return END;
    if(op[0] == '\0') return Error;
    return NotDefined;
}

///Набор динамических массивов, которые я использую
struct Defines_{
    Stack* ptrVariableNames;    ///<имена переменных
    Stack* ptrVariableValues;   ///<величины ссылок на переменные
    Stack* ptrLabelNames;       ///<имена меток
    Stack* ptrLabelValues;      ///<величины ссылок меток
};

///Смотрит по массиву меток есть ли данная "op" в списке и возвращает ее индекс, иначе возвращает NONE
int searchForLabel(struct Defines_ def, int op[]){
    int i; char found = 0;
    for(i = 0; i < getsize(def.ptrLabelNames) && !found; i++)
        if(compareStrStack(op, def.ptrLabelNames, i)) found = 1;
    if(found) return i - 1;
    else return NONE;
}

///Смотрит по массиву переменных есть ли данная "op" в списке и возвращает ее индекс, иначе возвращает NONE
int searchForVariable(struct Defines_ def, int op[]){
    int i; char found = 0;
    for(i = 0; i < getsize(def.ptrVariableNames) && !found; i++)
        if(compareStrStack(op, def.ptrVariableNames, i)) found = 1;
    if(found) return i - 1;
    else return NONE;
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

///Возвращает указатель на сформированную ссылку из 13 байт (может лепить ссылки из переменных и меток)
char* getPointer(const int op[], const struct Defines_ def, const unsigned int lineNum){
    static char result[13]; //1 байт под тип и 12 под ссылку
    int i = 0, j; int type;
    type = getType(op);
    if(type == Label){
        char found = 0;
        for(i = 0; i < getsize(def.ptrLabelNames) && !found; i++)
            if(compareStrStack(op, def.ptrLabelNames, i)) found = 1;
        if(found){
            result[0] = Ptr_const_const_const;
            myMemCpy(&result[1], stack_r(def.ptrLabelValues, i - 1), REG_SIZE);
            for(i = 5; i < 13; i++)
                result[i] = 0;
            return result;
        }
        else
            ERROROP2(Undefined lable,op);
    }
    if(type == NotDefined){ //переменная
        char found = 0;
        for(i = 0; i < getsize(def.ptrVariableNames) && !found; i++)
            if(compareStrStack(op, def.ptrVariableNames, i)) found = 1;
        if(found){
            result[0] = Ptr_const_const_const;
            myMemCpy(&result[1], def.ptrVariableValues, REG_SIZE);
            for(i = 5; i < 13; i++)
                result[i] = 0;
            return result;
        }
        else
            ERROROP2(Undefined variable,op);
    }
    assert(type == Pointer);

    int type2, type3;
    int buff[MAXOP], buff2[MAXOP], buff3[MAXOP];
    j = 0;
    //достаю 1й операнд
    for(i = 1; op[i] == '\t' || op[i] == ' '; i++);
    for (; op[i] != ';' && op[i] != ')' && op[i] != ' ' && op[i] != '\t'; i++, j++)
        buff[j] = op[i];
    buff[j] = '\0';
    type = getType(buff);

    //достаю 2й операнд
    for(; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] == ';')
        for(i++; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] != ')'){
        j = 0;
        for (; op[i] != ';' && op[i] != ')'; i++, j++)
            buff2[j] = op[i];
        buff2[j] = '\0';
        type2 = getType(buff2);
    }
    else
        type2 = Nothing;

    //достаю 3й операнд
    for(; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] == ';')
        for(i++; op[i] == '\t' || op[i] == ' '; i++);
    if(op[i] != ')'){
        j = 0;
        for (; op[i] != ';' && op[i] != ')'; i++, j++)
            buff3[j] = op[i];
        if(op[i] == ';') ERROROP2(Excess separator,op);
        buff3[j] = '\0';
        type3 = getType(buff3);
    }
    else
        type3 = Nothing;

    char code = 0;
    switch(type){
    case Register: case Register + 1: case Register + 2: case Register + 3:
    case Register + 4:case Register + 5: case Register + 6: case Register + 7:
        code += 0b000;
        result[1] = (char)(type - Register);
        result[2] = result[3] = result[4] = 0;
        break;
    case Label:
        code += 0b100;
        i = searchForLabel(def, buff);
        if(i != NONE)
            myMemCpy(&result[1], stack_r(def.ptrLabelValues, i), REG_SIZE);
        else ERROROP2(Undefined label,buff);
        break;
    case NotDefined:
        code += 0b100;
        i = searchForVariable(def, buff);
        if(i != NONE)
            myMemCpy(&result[1], stack_r(def.ptrVariableValues, i), REG_SIZE);
        else ERROROP2(Undefined variable,buff);
        break;
    default:
        ERROROP2(What the fuck 1?, buff);
    }

    void* temp;
    switch(type2){
        case Register: case Register + 1: case Register + 2: case Register + 3:
        case Register + 4:case Register + 5: case Register + 6: case Register + 7:
            code += 0b000;
            result[5] = (char)(type2 - Register);
            result[6] = result[7] = result[8] = 0;
            break;
        case Const16:
            code +=0b010;
            temp = getConst16D(&buff2[1]);
            myMemCpy(&result[5], temp, REG_SIZE);
            break;
        case Const10:
            code +=0b010;
            temp = getConst10D(&buff2[1]);
            myMemCpy(&result[5], temp, REG_SIZE);
            break;
        case Nothing:
            code +=0b010;
            result[5] = result[6] = result[7] = result[8] = 0;
            break;

        default:
            ERROROP2(What the fuck 2?, buff2);
    }

    switch(type3){
        case Register: case Register + 1: case Register + 2: case Register + 3:
        case Register + 4:case Register + 5: case Register + 6: case Register + 7:
            code += 0b000;
            result[9] = (char)(type3 - Register);
            result[10] = result[11] = result[12] = 0;
            break;
        case Const16:
            code +=0b001;
            temp = getConst16D(&buff3[1]);
            myMemCpy(&result[9], temp, REG_SIZE);
            break;
        case Const10:
            code +=0b001;
            temp = getConst10D(&buff3[1]);
            myMemCpy(&result[9], temp, REG_SIZE);
            break;
        case Nothing:
            code +=0b001;
            result[9] = 1;
            result[10] = result[11] = result[12] = 0; //те тут по дефолту единица в качестве шага
            break;

        default:
            ERROROP2(What the fuck 3?, buff3);
    }

    result[0] = code;
    return result;
}

///Конструктор
void definesInit(struct Defines_ *def){
    def->ptrVariableNames = stackInit(MAXOP * sizeof(int));   ///<имена переменных
    def->ptrVariableValues = stackInit(REG_SIZE);         ///<величины ссылок на переменные
    def->ptrLabelNames = stackInit(MAXOP * sizeof(int));      ///<имена меток
    def->ptrLabelValues = stackInit(REG_SIZE);            ///<величины ссылок меток
}

///Деструктор
void definesFree(struct Defines_ *def){
    stackFree(def->ptrVariableNames);
    stackFree(def->ptrVariableValues);
    stackFree(def->ptrLabelNames);
    stackFree(def->ptrLabelValues);
}

///Основная функция, вызывает все остальные
int compileFile(FILE* input, Stack* ptrProgram){
    int op[MAXOP];
    int type;
    unsigned int lineNum = 0;

    struct Defines_ def;
    definesInit(&def);
    u_int32_t varCounter = 0; ///<содержит кол-во уже занятых битов переменными (= указатель на след свободный бит)

    //Часто используемые переменные:
    int op2[MAXOP];     ///< Содержит второй оператор
    int type2;         ///< Содержит тип второго оператора
    unsigned char code;    ///< Обычно содержит кодировку оператора/операнда
    u_int32_t value;       ///< Обычно содержит значение метки или переменной (место в памяти)
    int search;          ///< Индекс найденного элемента через функции seach..
    int i;          ///< Индекс для циклов for
    char *temp;     ///<Какой то указатель (обычно для ссылок на строки)

    while(getOp(input, op, &lineNum) > 0) {
        type = getType(op);
        switch (type) {
            case MOV_reg_reg: //операнды с переменной длиной 3-21 байт

                if (getOp(input, op, &lineNum) > 0) {
                    type = getType(op);
                } else
                    ERROR(There are no operand);

                if (getOp(input, op2, &lineNum) > 0) {
                    type2 = getType(op2);
                } else
                    ERROR(There are no operand);

                if (type >= Register && type < Register + CountOfRegs) {
                    if (type2 >= Register && type2 < Register + CountOfRegs)
                        code = MOV_reg_reg;
                    else if (type2 == Const16 || type2 == Const10 || type2 == Label)
                        code = MOV_reg_const;
                    else if (type2 == Pointer || type2 == NotDefined)
                        code = MOV_reg_mem;
                    else
                        ERROROP(Invalid 2 argument to MOV, op2);
                } else if (type == Pointer || type == NotDefined) {
                    if (type2 >= Register && type2 < Register + CountOfRegs)
                        code = MOV_mem_reg;
                    else if (type2 == Const16 || type2 == Const10 || type2 == Label)
                        code = MOV_mem_const;
                    else
                        ERROROP(Invalid 2 argument to MOV, op2);
                } else
                    ERROROP(Invalid 1 argument to MOV, op2);

                push(ptrProgram, &code);

                switch(type){
                    case Register: case Register + 1: case Register + 2: case Register + 3:
                    case Register + 4: case Register + 5: case Register + 6: case Register + 7:
                        code = (char) (type - Register);
                        push(ptrProgram, &code);
                        break;
                    case Pointer: case NotDefined:                      //FIXME
                        temp = getPointer(op, def, lineNum);
                        if (temp == NULL) ERROROP(Error pointer, op);
                        for (i = 0; i < POINTER_SIZE; i++)
                            push(ptrProgram, temp + i);
                        break;
                    default:
                        assert(0);
                }

                switch(type2){
                case Register: case Register + 1: case Register + 2: case Register + 3:
                case Register + 4: case Register + 5: case Register + 6: case Register + 7:
                    code = (char) (type - Register);
                    push(ptrProgram, &code);
                    break;
                case Const16:
                    temp = (char *) getConst16D(&op2[1]);
                    for (i = 0; i < REG_SIZE; i++)
                        push(ptrProgram, temp + i);
                    break;
                case Const10:
                    temp = (char *) getConst10D(&op2[1]);
                    for (i = 0; i < REG_SIZE; i++)
                        push(ptrProgram, temp + i);
                    break;
                case Pointer:                                               //FIXME
                    temp = getPointer(op2, def, lineNum);
                    if (temp == NULL) ERROROP(Error pointer, op2);
                    for (i = 0; i < POINTER_SIZE; i++)
                        push(ptrProgram, temp + i);
                    break;
                case Label:
                    search = searchForLabel(def, op2);
                    if (search == NONE) ERROROP(Undefined label, op2);
                    temp = stack_r(def.ptrLabelValues, search);
                    for (i = 0; i < REG_SIZE; i++)
                        push(ptrProgram, temp + i);
                    break;
                default:
                    assert(0);
                }
                break;

            case PUSH_reg: case POP_reg: //операнды с фиксированной длиной, состоящие из 2-х байт

                code = (char) type;
                push(ptrProgram, &code);
                if (getOp(input, op, &lineNum) > 0) {
                    type = getType(op);
                    if (type >= Register && type < Register + CountOfRegs) {
                        code = (char) (type - Register); //num of reg
                        push(ptrProgram, &code);
                    } else
                        ERROROP(Invalid argument, op);
                } else
                    ERROROP(there are no arg, op);
                break;

            case ADD: case SUB: case INC: case IMUL: case IDIV: case AND: case OR: //однобайтные операнды
            case XOR: case NOT: case NEG: case SHL: case SHR: case SHRL: case CMP:
            case RET: case END:

                code = (char) type;
                push(ptrProgram, &code);
                break;

            case JMP_lbl: case JE_lbl: case JZ_lbl: case JG_lbl: //операнды с переменной длиной, весят по 5 байт в случае если переход по метке, 14 байт, если переход по ссылке
            case JGE_lbl: case JL_lbl: case JLE_lbl: case CALL_lbl:

                code = (char) type;
                if (getOp(input, op, &lineNum) > 0) {
                    type = getType(op);
                    if (type == Label) {
                        push(ptrProgram, &code);
                        search = searchForLabel(def, op);
                        if (search == NONE) ERROROP(Undefined label, op);
                        temp = stack_r(def.ptrLabelValues, search);
                        for (search = 0; search < REG_SIZE; search++)
                            push(ptrProgram, (void *) (temp + search));
                    } else if (type == Pointer) {                               //FIXME
                        code = code - JMP_lbl + JMP_ptr;
                        push(ptrProgram, &code);
                        temp = getPointer(op, def, lineNum);
                        if (temp == NULL) ERROROP(Error pointer, op);
                        for (i = 0; i < POINTER_SIZE; i++)
                            push(ptrProgram, (void *) (temp + i));
                    } else
                        ERROROP(This is not a lable or pointer, op);
                } else
                    ERROR(There are no operand);
                break;

            case DV: //инициализация переменных

                if (getOp(input, op, &lineNum) > 0) {
                    if (getType(op) != NotDefined) ERROROP(This is not name, op);
                    char found = 0;
                    for (i = 0; i < getsize(def.ptrVariableNames) && !found; i++)
                        if (compareStrStack(op, def.ptrVariableNames, i)) found = 1;
                    if (!found) {
                        value = varCounter | 0xFF000000;
                        push(def.ptrVariableNames, op);
                        push(def.ptrVariableValues, &value);
                        varCounter += sizeof(int32_t);
                    } else
                        ERROROP(Variable is still defined, op);
                } else
                    ERROR(there are no arg);
                break;

            case DA: //инициализация массивов

                if (getOp(input, op, &lineNum) > 0) {
                    if (getType(op) != NotDefined) ERROROP(This is not name, op);
                    char found = 0;
                    for (i = 0; i < getsize(def.ptrVariableNames) && !found; i++)
                        if (compareStrStack(op, def.ptrVariableNames, i)) found = 1;
                    if (!found) {
                        value = varCounter | 0xFF000000;
                        push(def.ptrVariableNames, op);
                        push(def.ptrVariableValues, &value);
                    } else
                        ERROROP(Variable is still defined, op);
                } else
                    ERROR(there are no arg:
                        name of variable);

                if (getOp(input, op, &lineNum) > 0) {
                    u_int32_t *tempConst;
                    if ((type = getType(op)) == Const10) {
                        tempConst = getConst10D(&op[1]);
                    } else if (type == Const16) {
                        tempConst = getConst16D(&op[1]);
                    } else
                        ERROROP(This is not count, op);
                    if (*tempConst == 0) ERROROP(It cant be zero value, op);
                    varCounter += (*tempConst) * sizeof(int32_t);
                    if (varCounter > 0x00FFFFFF) ERROR(Too many variables);
                } else
                    ERROR(there are no arg:
                        count of elements);
                break;

            case Label:

                search = searchForLabel(def, op);
                if (search != NONE) {
                    value = *((int32_t *) stack_r(def.ptrLabelValues, search));
                    for (i = 0; i < REG_SIZE; i++)
                        push(ptrProgram, (char *) &value + i);
                } else {
                    if (getsize(ptrProgram) < 0x00FFFFFF) { //FIXME: write error
                        push(def.ptrLabelNames, op);
                        value = getsize(ptrProgram);
                        push(def.ptrLabelValues, &value);
                    } else
                        ERROR(file is too large);
                }
                break;

            default:
                ERROROP(Unknown operator, op);
        }
    }
    definesFree(&def);
    return 0;
}








    /* //for QUAD command:
else if(type == Const16){   //const16
    char* temp = (char*)getConst16D(&op[1]);
    for(i = 0; i < REG_SIZE; i++)
        push(ptrProgram, temp + i);
}
else if(type == Const10){   //const10
    char* temp = (char*)getConst10D(&op[1]);
    for(i = 0; i < REG_SIZE; i++)
        push(ptrProgram, temp + i);
}
else if(type == NotDefined){    //may be variable
    int32_t temp;
    search = searchForVariable(def, op);
    if(search != NONE){
        temp = *((int32_t*)stack_r(def.ptrVariableValues, search));
        for(i = 0; i < REG_SIZE; i++)
            push(ptrProgram, (char*)&temp + i);
    }
    else
        ERROROP(Undefined variable or unknown operator, op);
}
else if(type == Pointer){   //pointer
    char* temp = getPointer(op, def, lineNum);
    if(temp == NULL) ERROROP(Error pointer,op);
    for(i = 0; i < POINTER_SIZE; i++)
        push(ptrProgram, temp + i);
}
else
    ERROROP(Unknown operator, op);
}*/