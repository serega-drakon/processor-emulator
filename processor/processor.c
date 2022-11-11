#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "../encoding/encodings.h"
#include "../memory/memory.h"

#define ERROR 1
#define ERROREND (END + 1)

#define ERROR_EXIT(message) do { \
printf(#message);          \
return ERROR;                    \
}while(0)

struct Processor_ {
    Stack *ptrProgram;
    Stack *ptrVariable;
    Stack *ptrStack;
    int32_t ax;
    int32_t bx;
    int32_t cx;
    int32_t dx;
    int32_t si;
    int32_t di;
    int32_t sp;
    int32_t dp;
    u_int32_t pos; ///< текущая позиция в программе
    char cf;    ///<carry flag
    char zf;    ///<zero flag
    char sf;    ///<sign flag
    char of;    ///<overflow flag
};

typedef struct Processor_ Processor;

///Конструктор процессора
int processor_init(Processor *ptrProc){        //FIXME: new registers
    assert(ptrProc != NULL);
    ptrProc->ptrStack = stackInit(REG_SIZE);
    if(ptrProc->ptrStack == NULL) return ERROR;
    ptrProc->ptrVariable = stackInit(REG_SIZE);
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
    ptrProc->cf = 0;
    ptrProc->zf = 0;
    ptrProc->sf = 0;
    ptrProc->of = 0;
    return 0;
}

void mem_alloc(Stack *ptrVariable, u_int32_t bytesForVar){ //делает так, чтобы стек не писал ошибок и не возвращал
    u_int32_t buffValue = 0;                               //бессмысленных пойзонов
    for(; bytesForVar > 0; bytesForVar--)
        push(ptrVariable, &buffValue);
}

void processor_end(Processor *ptrProc){
    free(ptrProc->ptrStack);
    free(ptrProc->ptrVariable);
}

///Записывает 4 байта в ptrRes из ptrStack по позиции pos (без проверки)
void getValue32(int32_t *ptrRes, Stack *ptrStack, u_int32_t pos){
    for(int i = 0; i < 4; i++)
        *((char*) ptrRes + i) = *(char*) stack_r(ptrStack, pos + i);
}

///Получает байт с позицией pos из стека с программой (с проверкой)
unsigned char getByte(Stack *ptrStack, u_int32_t pos){
    assert(ptrStack != NULL);
    unsigned char byte;
    if(pos < getsize(ptrStack))
        byte = *(unsigned char*)stack_r(ptrStack, pos);
    else
        byte = ERROREND;
    return byte;
}

///Получает значение из регистра
int32_t getFromReg(const Processor *proc, char codeOfReg){
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
void pushToReg(Processor *ptrProc, char codeOfReg, int32_t value){
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

void doMov_reg_reg(Processor *ptrProc){
    const char reg1 = *(char*) stack_r(ptrProc->ptrProgram, ptrProc->pos++);
    const char reg2 = *(char*) stack_r(ptrProc->ptrProgram, ptrProc->pos++);
    const int32_t value = getFromReg(ptrProc, reg2);
    pushToReg(ptrProc, reg1, value);
}

void doMov_reg_const(Processor *ptrProc){
    const char reg1 = *(char*) stack_r(ptrProc->ptrProgram, ptrProc->pos++);
    int32_t value;
    getValue32(&value, ptrProc->ptrProgram, ptrProc->pos);
    ptrProc->pos += 4;
    pushToReg(ptrProc, reg1, value);
}

int doMov_reg_mem(){

}

int doMov_mem_reg(){

}

int doMov_mem_const(){

}

int doPush_reg(){

}

int doPop_reg(){

}

int doAdd(){

}

int doSub(){

}

int doInc(){

}

int doImul(){

}

int doIdiv(){

}

int doAnd(){

}

int doOr(){

}

int doXor(){

}

int doNot(){

}

int doNeg(){

}

int doShl(){

}

int doShr(){

}

int doShrl(){

}

int doCmp(){

}

int doJmp_lbl(){

}

int doJe_lbl(){

}

int doJz_lbl(){

}

int doJg_lbl(){

}

int doJge_lbl(){

}

int doJl_lbl(){

}

int doJle_lbl(){

}

int doCall_lbl(){

}

int doRet(){

}

int doPrint_reg(){

}

int doJmp_ptr(){

}

int doJe_ptr(){

}

int doJz_ptr(){

}

int doJg_ptr(){

}

int doJge_ptr(){

}

int doJl_ptr(){

}

int doJle_ptr(){

}

int doCall_ptr() {
}

int processor_main(Stack *ptrProgram, u_int32_t bytesForVar) {
    Processor proc;
    proc.ptrProgram = ptrProgram;
    if (processor_init(&proc) == ERROR)
        return ERROR;
    mem_alloc(proc.ptrVariable, bytesForVar);

    unsigned command; //код команды весит 1 байт
    while ((command = getByte(proc.ptrProgram, proc.pos++)) != ERROREND) {
        switch (command) {
            case MOV_reg_reg:
                doMov_reg_reg(&proc);
                break;
            case MOV_reg_const:
                doMov_reg_const(&proc);
                break;
            case MOV_reg_mem:
            case MOV_mem_reg:
            case MOV_mem_const:
            case PUSH_reg:
            case POP_reg:
            case ADD:
            case SUB:
            case INC:
            case IMUL:
            case IDIV:
            case AND:
            case OR:
            case XOR:
            case NOT:
            case NEG:
            case SHL:
            case SHR:
            case SHRL:
            case CMP:
            case JMP_lbl:
            case JE_lbl:
            case JZ_lbl:
            case JG_lbl:
            case JGE_lbl:
            case JL_lbl:
            case JLE_lbl:
            case CALL_lbl:
            case RET:
            case PRINT_reg:
            case JMP_ptr:
            case JE_ptr:
            case JZ_ptr:
            case JG_ptr:
            case JGE_ptr:
            case JL_ptr:
            case JLE_ptr:
            case CALL_ptr:
            default:
                ERROR_EXIT(unknown byte);
            case END:
                processor_end(&proc);
                return 0;
        }
    }
    ERROR_EXIT(unexpected end of program);
}

