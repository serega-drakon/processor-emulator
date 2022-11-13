#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "../encoding/encodings.h"
#include "../memory/memory.h"

#define ERROR 1
#define ERROREND (-1)

#define ERROR_EXIT(message) do { \
printf(message);          \
return ERROR;                    \
}while(0)

#define VAR_MEM_MIN 0xFF000000
#define PROGRAM_MEM_MAX 0x01000000

struct Processor_ {
    Stack *ptrProgram; ///< Код программы
    Stack *ptrVariable; ///< Переменные
    Stack *ptrAStack; ///<Стек для арифметики
    Stack *ptrPStack; ///<Стек для процедур
    int32_t ax;
    int32_t bx;
    int32_t cx;
    int32_t dx;
    int32_t si;
    int32_t di;
    int32_t sp;
    int32_t dp;
    u_int32_t pos; ///< текущая позиция в программе
    char zf;    ///<zero flag
    char sf;    ///<sign flag
};

typedef struct Processor_ Processor;

///Конструктор процессора
int processor_init(Processor *ptrProc){
    assert(ptrProc != NULL);
    ptrProc->ptrAStack = stackInit(sizeof(int32_t));
    if(ptrProc->ptrAStack == NULL) return ERROR;
    ptrProc->ptrPStack = stackInit(sizeof(int32_t));
    if(ptrProc->ptrPStack == NULL) return ERROR;
    ptrProc->ptrVariable = stackInit(sizeof(int32_t));
    if(ptrProc->ptrVariable == NULL) return ERROR;
    ptrProc->ax = 0;
    ptrProc->bx = 0;
    ptrProc->cx = 0;
    ptrProc->dx = 0;
    ptrProc->si = 0;
    ptrProc->di = 0;
    ptrProc->sp = 0;
    ptrProc->dp = 0;
    ptrProc->pos = 0;
    ptrProc->zf = 0;
    ptrProc->sf = 0;
    return 0;
}
///делает так, чтобы стек не писал ошибок и не возвращал
///бессмысленных пойзонов
void mem_alloc(Stack *ptrVariable, u_int32_t bytesForVar){
    u_int32_t buffValue = 0;
    for(; bytesForVar > 0; bytesForVar--)
        push(ptrVariable, &buffValue);
}

///Деструктор процессора
void processor_end(Processor *ptrProc){
    free(ptrProc->ptrAStack);
    free(ptrProc->ptrVariable);
}

///Получает байт с позицией pos из стека с программой (с проверкой)
int getByte(Stack *ptrStack, u_int32_t pos){
    assert(ptrStack != NULL);
    unsigned char byte;
    if(pos < getsize(ptrStack))
        byte = *(unsigned char*)stack_r(ptrStack, pos);
    else
        byte = ERROREND;
    return byte;
}

///Получает значение из регистра
int32_t getFromReg(const Processor *proc, unsigned char codeOfReg){
    switch(codeOfReg){
        case AX:
            return proc->ax;
        case BX:
            return proc->bx;
        case CX:
            return proc->cx;
        case DX:
            return proc->dx;
        case SI:
            return proc->si;
        case DI:
            return proc->di;
        case SP:
            return proc->sp;
        case DP:
            return proc->dp;
        default:
            assert(0);
    }
}

///Присваивает регистру данное значение
void writeToReg(Processor *ptrProc, unsigned char codeOfReg, int32_t value){
    switch(codeOfReg){
        case AX:
            ptrProc->ax = value;
            break;
        case BX:
            ptrProc->bx = value;
            break;
        case CX:
            ptrProc->cx = value;
            break;
        case DX:
            ptrProc->dx = value;
            break;
        case SI:
            ptrProc->si = value;
            break;
        case DI:
            ptrProc->di = value;
            break;
        case SP:
            ptrProc->sp = value;
            break;
        case DP:
            ptrProc->dp = value;
            break;
        default:
            assert(0);
    }
}

///Возвращает 4 байта из ptrAStack по позиции pos (без проверки) \n
///\warning ptrAStack должен быть размера 1 байт!
int32_t getValue32(Stack *ptrStack, u_int32_t pos){
    int32_t result;
    for(int i = 0; i < 4; i++)
        *((char*) &result + i) = *(char*) stack_r(ptrStack, pos + i);
    return result;
}

unsigned char readReg(Processor *ptrProc){
    const unsigned char reg = *(char*) stack_r(ptrProc->ptrProgram, ptrProc->pos++);
    return reg;
}

///Читает код регистра из потока ввода ptrProgram, обновляет позицию ptrPos и возвращает значение регистра
u_int32_t readRegValue(Processor *ptrProc){
    unsigned char codeOfReg = (unsigned char) getByte(ptrProc->ptrProgram, ptrProc->pos++);
    u_int32_t value = getFromReg(ptrProc, codeOfReg);
    return value;
}

///Читает константу из потока ввода ptrProgram и обновляет позицию ptrPos
u_int32_t readConst32(Processor *ptrProc){
    u_int32_t value = getValue32(ptrProc->ptrProgram, ptrProc->pos);
    ptrProc->pos += sizeof(int32_t);
    return value;
}

u_int32_t readAddress_reg_reg_reg(Processor *ptrProc){
    const u_int32_t firstArg = readRegValue(ptrProc);
    const u_int32_t secondArg = readRegValue(ptrProc);
    const u_int32_t thirdArg = readRegValue(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_reg_reg_const(Processor *ptrProc){
    const u_int32_t firstArg = readRegValue(ptrProc);
    const u_int32_t secondArg = readRegValue(ptrProc);
    const u_int32_t thirdArg = readConst32(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_reg_const_reg(Processor *ptrProc){
    const u_int32_t firstArg = readRegValue(ptrProc);
    const u_int32_t secondArg = readConst32(ptrProc);
    const u_int32_t thirdArg = readRegValue(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_reg_const_const(Processor *ptrProc){
    const u_int32_t firstArg = readRegValue(ptrProc);
    const u_int32_t secondArg = readConst32(ptrProc);
    const u_int32_t thirdArg = readConst32(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_const_reg_reg(Processor *ptrProc){
    const u_int32_t firstArg = readConst32(ptrProc);
    const u_int32_t secondArg = readRegValue(ptrProc);
    const u_int32_t thirdArg = readRegValue(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_const_reg_const(Processor *ptrProc){
    const u_int32_t firstArg = readConst32(ptrProc);
    const u_int32_t secondArg = readRegValue(ptrProc);
    const u_int32_t thirdArg = readConst32(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_const_const_reg(Processor *ptrProc){
    const u_int32_t firstArg = readConst32(ptrProc);
    const u_int32_t secondArg = readConst32(ptrProc);
    const u_int32_t thirdArg = readRegValue(ptrProc);
    return firstArg + secondArg * thirdArg;
}

u_int32_t readAddress_const_const_const(Processor *ptrProc){
    const u_int32_t firstArg = readConst32(ptrProc);
    const u_int32_t secondArg = readConst32(ptrProc);
    const u_int32_t thirdArg = readConst32(ptrProc);
    return firstArg + secondArg * thirdArg;
}

///Получает адрес из сложной адресации
u_int32_t readAddress(Processor *ptrProc){
    const char type = *(char*) stack_r(ptrProc->ptrProgram, ptrProc->pos++);
    switch(type){
        case Ptr_reg_reg_reg:
            return readAddress_reg_reg_reg(ptrProc);
        case Ptr_reg_reg_const:
            return readAddress_reg_reg_const(ptrProc);
        case Ptr_reg_const_reg:
            return readAddress_reg_const_reg(ptrProc);
        case Ptr_reg_const_const:
            return readAddress_reg_const_const(ptrProc);
        case Ptr_const_reg_reg:
            return readAddress_const_reg_reg(ptrProc);
        case Ptr_const_reg_const:
            return readAddress_const_reg_const(ptrProc);
        case Ptr_const_const_reg:
            return readAddress_const_const_reg(ptrProc);
        case Ptr_const_const_const:
            return readAddress_const_const_const(ptrProc);
        default:
            assert(0);
    }
}

///Обрабатывает указатель из потока ввода и возвращает значение с него
int32_t readFromMem32(Processor *ptrProc){
    u_int32_t address = readAddress(ptrProc);
    if(address >= VAR_MEM_MIN){ //Variable
        address = (address- VAR_MEM_MIN) / sizeof(int32_t);
        if(address < getsize(ptrProc->ptrVariable))
            return *(int32_t*) stack_r(ptrProc->ptrVariable, address);
        else
            printf("readFromMem32: error reading from not allocated memory (variable)\n");
    }
    else if(address < PROGRAM_MEM_MAX){ //Program
        if(address < getsize(ptrProc->ptrProgram))
            return getValue32(ptrProc->ptrProgram, address);
        else
            printf("readFromMem32: error reading from not allocated memory (program)\n");
    }
    else
        printf("readFromMem32: error reading from unknown memory (overflow maybe?)\n");
    return 0;
}

///Записывает значение по его адресу
int writeToMem32(Processor *ptrProc, u_int32_t address, u_int32_t value){
    if(address >= VAR_MEM_MIN){ //Variable
        address = (address - VAR_MEM_MIN) / sizeof(int32_t);
        if(address < getsize(ptrProc->ptrVariable)) {
            stack_w(ptrProc->ptrVariable, address, &value);
            return 0;
        }
        else
            printf("readFromMem32: error reading from not allocated memory (variable)\n");
    }
    else if(address < PROGRAM_MEM_MAX){ //Program
        if(address < getsize(ptrProc->ptrProgram))
            return 0; //мб сделать запись в программу?..
        else
            printf("readFromMem32: error reading from not allocated memory (program)");
    }
    else
        printf("readFromMem32: error reading from unknown memory (overflow maybe?)");
    return 1;
}

void doMov_reg_reg(Processor *ptrProc){
    const unsigned char reg1 = readReg(ptrProc);
    const unsigned char reg2 = readReg(ptrProc);
    const int32_t value = getFromReg(ptrProc, reg2);
    writeToReg(ptrProc, reg1, value);
}

void doMov_reg_const(Processor *ptrProc){
    const unsigned char reg1 = readReg(ptrProc);
    int32_t value = (int32_t) readConst32(ptrProc);
    writeToReg(ptrProc, reg1, value);
}

void doMov_reg_mem(Processor *ptrProc){
    const unsigned char reg1 = readReg(ptrProc);
    int32_t valueFromMem = readFromMem32(ptrProc);
    writeToReg(ptrProc, reg1, valueFromMem);
}

int doMov_mem_reg(Processor *ptrProc){
    const u_int32_t address = readAddress(ptrProc);
    const u_int32_t valueFromReg2 = readRegValue(ptrProc);
    return writeToMem32(ptrProc, address, valueFromReg2);
}

int doMov_mem_const(Processor *ptrProc){
    const u_int32_t address = readAddress(ptrProc);
    const u_int32_t valueConst = readConst32(ptrProc);
    return writeToMem32(ptrProc, address, valueConst);
}

void doPush_reg(Processor *ptrProc){
    const u_int32_t valueFromReg = readRegValue(ptrProc);
    push(ptrProc->ptrAStack, &valueFromReg);
}

void doPop_reg(Processor *ptrProc){
    const unsigned char reg = readReg(ptrProc);
    int32_t valueFromStack = *(int32_t*) pop(ptrProc->ptrAStack);
    writeToReg(ptrProc, reg, valueFromStack);
}

void flagCheck(Processor *ptrProc, int32_t result){
    if(result < 0) ptrProc->sf = 1;
    else ptrProc->sf = 0;
    if(result == 0) ptrProc->zf = 1;
    else ptrProc->zf = 0;
}

void doAdd(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value1 + value2;
    push(ptrProc->ptrAStack, &result);
}

void doSub(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value1 - value2;
    push(ptrProc->ptrAStack, &result);
}

void doInc(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 1)
        printf("Stack is empty\n");
    const int32_t value = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value + 1;
    push(ptrProc->ptrAStack, &result);
}

void doDec(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 1)
        printf("Stack is empty\n");
    const int32_t value = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value - 1;
    push(ptrProc->ptrAStack, &result);
}

void doImul(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value1 * value2;
    push(ptrProc->ptrAStack, &result);
}

void doIdiv(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    int32_t result;
    if(value2 != 0)
        result = value1 / value2;
    else{
        result = 0;
        printf("doIdiv: division by zero, returning zero value...\n");
    }
    push(ptrProc->ptrAStack, &result);
}

void doAnd(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const u_int32_t value2 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t value1 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t result = value1 & value2;
    push(ptrProc->ptrAStack, &result);
}

void doOr(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const u_int32_t value2 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t value1 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t result = value1 | value2;
    push(ptrProc->ptrAStack, &result);
}

void doXor(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const u_int32_t value2 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t value1 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t result = value1 ^ value2;
    push(ptrProc->ptrAStack, &result);
}

void doNot(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 1)
        printf("Stack is empty\n");
    const u_int32_t value = *(int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t result = ~value;
    push(ptrProc->ptrAStack, &result);
}

void doNeg(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 1)
        printf("Stack is empty\n");
    const int32_t value = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = -value;
    push(ptrProc->ptrAStack, &result);
}

void doShl(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value1 << value2;
    push(ptrProc->ptrAStack, &result);
}

void doShr(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const int32_t value2 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t value1 = *(int32_t*) pop(ptrProc->ptrAStack);
    const int32_t result = value1 >> value2;
    push(ptrProc->ptrAStack, &result);
}

void doShrl(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) < 2)
        printf("Stack is empty\n");
    const u_int32_t value2 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t value1 = *(u_int32_t*) pop(ptrProc->ptrAStack);
    const u_int32_t result = value1 >> value2;
    push(ptrProc->ptrAStack, &result);
}

void doCmp(Processor *ptrProc){
    if(getsize(ptrProc->ptrAStack) > 1) {
        const int32_t value2 = *(int32_t *) pop(ptrProc->ptrAStack);
        const int32_t value1 = *(int32_t *) pop(ptrProc->ptrAStack);
        const int32_t result = value1 - value2;
        flagCheck(ptrProc, result);
    }
    else
        printf("Stack is empty\n");
}

void doJmp_lbl(Processor *ptrProc){
    const u_int32_t valueLabel = readConst32(ptrProc);
    ptrProc->pos = valueLabel;
}

void doJe_lbl(Processor *ptrProc){
    if(ptrProc->zf == 1)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJne_lbl(Processor *ptrProc){
    if(ptrProc->zf == 0)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJz_lbl(Processor *ptrProc){
    if(ptrProc->zf == 1)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJg_lbl(Processor *ptrProc){
    if(ptrProc->sf == 0 && ptrProc->zf == 0)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJge_lbl(Processor *ptrProc){
    if(ptrProc->sf == 0)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJl_lbl(Processor *ptrProc){
    if(ptrProc->sf == 1)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doJle_lbl(Processor *ptrProc){
    if(ptrProc->sf == 1 || ptrProc->zf == 1)
        doJmp_lbl(ptrProc);
    else
        readConst32(ptrProc);
}

void doCall_lbl(Processor *ptrProc){
    const u_int32_t nextPos = ptrProc->pos + sizeof(int32_t);
    push(ptrProc->ptrPStack, &nextPos);
    doJmp_lbl(ptrProc);
}

void doRet(Processor *ptrProc){
    const u_int32_t valueRet = *(u_int32_t*) pop(ptrProc->ptrPStack);
    ptrProc->pos = valueRet;
}

void doPrint_reg(Processor *ptrProc){
    const u_int32_t valueFromReg = readRegValue(ptrProc);
    printf("output: %d\n", valueFromReg);
}

void doJmp_ptr(Processor *ptrProc){
    const u_int32_t valueFromPtr = readFromMem32(ptrProc);
    ptrProc->pos = valueFromPtr;
}

void doJe_ptr(Processor *ptrProc){
    if(ptrProc->zf == 1)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJne_ptr(Processor *ptrProc){
    if(ptrProc->zf == 0)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJz_ptr(Processor *ptrProc){
    if(ptrProc->zf == 1)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJg_ptr(Processor *ptrProc){
    if(ptrProc->sf == 0 && ptrProc->zf == 0)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJge_ptr(Processor *ptrProc){
    if(ptrProc->sf == 0)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJl_ptr(Processor *ptrProc){
    if(ptrProc->sf == 1)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doJle_ptr(Processor *ptrProc){
    if(ptrProc->sf == 1 || ptrProc->zf == 1)
        doJmp_ptr(ptrProc);
    else
        readAddress(ptrProc);
}

void doCall_ptr(Processor *ptrProc){
    const u_int32_t nextPosFromPointer = readAddress(ptrProc);
    push(ptrProc->ptrPStack, &ptrProc->pos);
    ptrProc->pos = nextPosFromPointer;
}

int processor_main(Stack *ptrProgram, u_int32_t bytesForVar) {
    Processor proc;
    proc.ptrProgram = ptrProgram;
    if (processor_init(&proc) == ERROR)
        return ERROR;
    mem_alloc(proc.ptrVariable, bytesForVar);

    int command; //код команды весит 1 байт
    while ((command = getByte(proc.ptrProgram, proc.pos++)) != ERROREND) {
        switch (command) {
            case MOV_reg_reg:
                doMov_reg_reg(&proc);
                break;
            case MOV_reg_const:
                doMov_reg_const(&proc);
                break;
            case MOV_reg_mem:
                doMov_reg_mem(&proc);
                break;
            case MOV_mem_reg:
                doMov_mem_reg(&proc);
                break;
            case MOV_mem_const:
                doMov_mem_const(&proc);
                break;
            case PUSH_reg:
                doPush_reg(&proc);
                break;
            case POP_reg:
                doPop_reg(&proc);
                break;
            case ADD:
                doAdd(&proc);
                break;
            case SUB:
                doSub(&proc);
                break;
            case INC:
                doInc(&proc);
                break;
            case DEC:
                doDec(&proc);
                break;
            case IMUL:
                doImul(&proc);
                break;
            case IDIV:
                doIdiv(&proc);
                break;
            case AND:
                doAnd(&proc);
                break;
            case OR:
                doOr(&proc);
                break;
            case XOR:
                doXor(&proc);
                break;
            case NOT:
                doNot(&proc);
                break;
            case NEG:
                doNeg(&proc);
                break;
            case SHL:
                doShl(&proc);
                break;
            case SHR:
                doShr(&proc);
                break;
            case SHRL:
                doShrl(&proc);
                break;
            case CMP:
                doCmp(&proc);
                break;
            case JMP_lbl:
                doJmp_lbl(&proc);
                break;
            case JE_lbl:
                doJe_lbl(&proc);
                break;
            case JNE_lbl:
                doJne_lbl(&proc);
                break;
            case JZ_lbl:
                doJz_lbl(&proc);
                break;
            case JG_lbl:
                doJg_lbl(&proc);
                break;
            case JGE_lbl:
                doJge_lbl(&proc);
                break;
            case JL_lbl:
                doJl_lbl(&proc);
                break;
            case JLE_lbl:
                doJle_lbl(&proc);
                break;
            case CALL_lbl:
                doCall_lbl(&proc);
                break;
            case RET:
                doRet(&proc);
                break;
            case PRINT_reg:
                doPrint_reg(&proc);
                break;
            case JMP_ptr:
                doJmp_ptr(&proc);
                break;
            case JE_ptr:
                doJe_ptr(&proc);
                break;
            case JNE_ptr:
                doJne_ptr(&proc);
                break;
            case JZ_ptr:
                doJz_ptr(&proc);
                break;
            case JG_ptr:
                doJg_ptr(&proc);
                break;
            case JGE_ptr:
                doJge_ptr(&proc);
                break;
            case JL_ptr:
                doJl_ptr(&proc);
                break;
            case JLE_ptr:
                doJle_ptr(&proc);
                break;
            case CALL_ptr:
                doCall_ptr(&proc);
                break;
            case END:
                processor_end(&proc);
                return 0;
            default:
                ERROR_EXIT("unknown byte");
        }
    }
    ERROR_EXIT("unexpected end of program");
}