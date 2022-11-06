#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define READ 0
#define WRITE 1
#define HAS_USED 1
#define RESET 2

#define KAN_NUM 1
#define KAN_VALUE 255    //1111.1111
#define POISON_VALUE 255 //1111.1111

#define EXIT_MAIN do{ \
if (error_main(ptrStack, READ, BuffForErrNull) == 0)\
    return ptrStack->buffForErr;                    \
else                                                \
    return NULL;                                    \
}while(0)

#define EXIT do{ \
if (ptrStack != NULL && error_main(ptrStack, READ, BuffForErrNull) == 0)\
    return ptrStack->buffForErr;                    \
else                                                \
    return NULL;                                    \
}while(0)

enum Errors { //Не больше 8 ошибок! иначе надо расширять переменную error
    PtrStackNull = 0, //number of right bit in error
    DataArrayNull,
    BuffForErrNull,
    BuffForResNull,
    MetaNull,
    KanareiykePizda,
};

/// Structure of stack
struct Stack_ {
    void *data; ///< Pointer to data
    void *buffForErr;///< buffForErr returns if error occurred
    void *buffForRes;///< buffForRes points to result of stack_main() func
    u_int32_t size; ///< Size of one element of data in bytes
    u_int32_t num; ///< Number of elements of data (malloced memory)
    u_int32_t pos; ///< Next free position of stack (pop/push/getlast)
    unsigned char *meta; ///< "Poison" check of data
    int metaNum; ///< Number of elements of meta (malloced memory)
    unsigned char error;///< is an array of bools
};

int error_main(Stack *ptrStack, int flag, int numOfError);

int meta_main(Stack *ptrStack, int flag, u_int32_t x);

void stack_extend(Stack *ptrStack, u_int32_t x);

int kanareiyka_check(Stack *ptrStack);


///Копирует участок помяти по байтам с fromPtr на toPtr
void myMemCpy(const void *toPtr, const void *fromPtr, u_int32_t sizeInBytes){
    assert(toPtr != NULL);
    assert(sizeInBytes >= 0);
    assert(fromPtr != NULL || sizeInBytes == 0);

    for (int i = 0; i < sizeInBytes; i++)
        ((char *) toPtr)[i] = ((char *) fromPtr)[i];
}

void saveResToBuff(Stack *ptrStack, u_int32_t x){
    const u_int32_t shift_in_stack = (x + KAN_NUM) * ptrStack->size;
    const void* from_buf = &((char *) ptrStack->data)[shift_in_stack];
    myMemCpy(ptrStack->buffForRes, from_buf, ptrStack->size);
}

/// Main function of stack array
void *stack_main(Stack *ptrStack, int flag, u_int32_t x, const void *ptrValue) { //FIXME: ООП
    assert(ptrStack != NULL);
    assert(flag != WRITE || ptrValue != NULL);
    assert(flag == READ || flag == WRITE);
    assert(x >= 0);

    if(kanareiyka_check(ptrStack))
        error_main(ptrStack, WRITE, KanareiykePizda);
    if(stackErrorCheck(ptrStack)) {
        stackErrorPrint(ptrStack);
        EXIT_MAIN;
    }
    //extend check
    stack_extend(ptrStack, x);
    if (stackErrorCheck(ptrStack)) {
        printf("stack_main: error\n");
        stackErrorPrint(ptrStack);
        EXIT_MAIN;
    }

    switch (flag) {
        case READ:
            //check here needed
            if (!meta_main(ptrStack, READ, x))
                printf("stack_main: using before undefined value X = %d\n", x);
            saveResToBuff(ptrStack, x);
            return ptrStack->buffForRes;
        case WRITE:
            myMemCpy(&(((char *) ptrStack->data)[(x + KAN_NUM) * ptrStack->size]), ptrValue, ptrStack->size);
            meta_main(ptrStack, HAS_USED, x);
            saveResToBuff(ptrStack, x);
            return ptrStack->buffForRes;
        default:
            assert(0);
    }
}

/// Extends given Stack_ by STEP const
void stack_extend(Stack *ptrStack, u_int32_t x) {
    assert(ptrStack != NULL);
    assert(x >= 0);

    void *buffPtr;

    //Сначала выделяем память под data
    if (x >= ptrStack->num) {
        buffPtr = ptrStack->data;
        x = x * 2; //new number of elements
        ptrStack->data = malloc((x + 2 * KAN_NUM) * ptrStack->size); //+канарейка слева и справа
        if (ptrStack->data != NULL) {
            //возвращаем элементы (включил канарейку слева)
            //указатель на новую канарейку справа
            const void *ptrKanRightNew = &((char*)ptrStack->data)[(KAN_NUM + x) * ptrStack->size];
            //указатель на старую канарейку справа
            const void *ptrKanRightOld = &((char*)buffPtr)[(KAN_NUM + ptrStack->num) * ptrStack->size];

            myMemCpy(ptrStack->data, buffPtr, (KAN_NUM + ptrStack->num) * ptrStack->size);
            myMemCpy(ptrKanRightNew,ptrKanRightOld, KAN_NUM * ptrStack->size);

            if(!error_main(ptrStack, READ, BuffForErrNull))
                //заполняю пустоты пойзонами
                for(int i = 0; i < (x - ptrStack->num) * ptrStack->size ; i++){
                    ((unsigned char*)ptrStack->data)[(KAN_NUM + ptrStack->num) * ptrStack->size + i] = POISON_VALUE;
                }
            ptrStack->num = x;
            free(buffPtr);
        } else {
            error_main(ptrStack, WRITE, DataArrayNull);
            ptrStack->data = buffPtr;
            return;
        }
    }
    //Потом выделяем память для meta
    if (x > ptrStack->metaNum * 8) {
        buffPtr = ptrStack->meta;
        int y;
        for(y = ptrStack->metaNum; y * 8 < x; y *= 2)
            ;
        ptrStack->meta = calloc(y, sizeof(char));
        if (ptrStack->meta != NULL) {
            //возвращаем элементы
            myMemCpy(ptrStack->meta, buffPtr, ptrStack->metaNum);
            ptrStack->metaNum = y;
            free(buffPtr);
        } else {
            error_main(ptrStack, WRITE, MetaNull);
            ptrStack->meta = buffPtr;
            return;
        }
    }

}

///Битовый массив, который содержит информацию об использовании каждого из элементов массива
int meta_main(Stack *ptrStack, int flag, u_int32_t x) {
    assert(ptrStack != NULL);
    assert(flag == READ || flag == HAS_USED || flag == RESET);

    //нумерация справа налево
    const int numOfBit = 7 - (char) (x % 8);

    switch (flag) {
        case READ:
            break;
        case HAS_USED:
            ptrStack->meta[x / 8] = ptrStack->meta[x / 8] | (1 << numOfBit);
            break;
        case RESET:
            ptrStack->meta[x / 8] = ptrStack->meta[x / 8] & ~(1 << numOfBit);
            break;
        default:
            assert(0);
    }
    return (ptrStack->meta[x / 8] >> numOfBit) & 1; //достаю нужный бит
}

///Максимум вариантов ошибок - 8 с таким размером error.
int error_main(Stack *ptrStack, int flag, int numOfError) {
    assert(ptrStack != NULL);
    assert(flag == READ || flag == WRITE);
    assert(numOfError >= 0);

    switch (flag) {
        case READ:
            break;
        case WRITE:
            ptrStack->error = ptrStack->error | (1 << numOfError);
            break;
        default:
            assert(0);
    }
    return ptrStack->error >> numOfError & 1; //достаю нужный бит
}

///проверяет канарейки массива и возвращает 1 если они повреждены, 0 если нет.
int kanareiyka_check(Stack *ptrStack){
    int check = 0;
    const u_int32_t shift = (KAN_NUM + ptrStack->num) * ptrStack->size;
    for(int i = 0; i < KAN_NUM * ptrStack->size; i++)
        if(((unsigned char*)ptrStack->data)[i] != KAN_VALUE
           || ((unsigned char*)ptrStack->data)[shift + i] != KAN_VALUE)
            check = 1;
    return check;
}

///сбрасывает текущую позицию массива до пойзона.
void stack_reset_pos(Stack *ptrStack, u_int32_t x){
    u_int32_t i;
    for(i = 0; i < ptrStack->size; i++)
        ((unsigned char*)ptrStack->buffForErr)[i] = POISON_VALUE;
    stack_main(ptrStack, WRITE, x, ptrStack->buffForErr);
    meta_main(ptrStack, RESET, ptrStack->pos);
    for(i = 0; i < ptrStack->size; i++)
        ((char*)ptrStack->buffForErr)[i] = 0;
}

///returns !0 if error occurred
int stackErrorCheck(Stack *ptrStack) {
    if (ptrStack != NULL) {
        return ptrStack->error;
    } else
        return 1 << PtrStackNull;
    //ака так бы было если бы переменная error существовала
}

///Выводит инфу об ошибках в консоль.
void stackErrorPrint(Stack *ptrStack){
    if (ptrStack != NULL) {
        if (error_main(ptrStack, READ, DataArrayNull))
            printf("error DataArrayNull\n");
        if (error_main(ptrStack, READ, BuffForErrNull))
            printf("error BuffForErrNull\n");
        if (error_main(ptrStack, READ, BuffForResNull))
            printf("error BuffForResNull\n");
        if (error_main(ptrStack, READ, MetaNull))
            printf("error MetaNull\n");
        if(error_main(ptrStack, READ, KanareiykePizda)){
            printf("error KanareiykePizda\n");
        }
    } else
        printf("error PtrStackNull\n");
}

/// Constructor of stack.
void *stackInit(u_int32_t size) {
    if (size <= 0)
        return NULL;
    Stack *ptrStack;
    ptrStack = malloc(sizeof(Stack));
    if (ptrStack != NULL) {
        ptrStack->size = size;
        ptrStack->pos = 0;
        ptrStack->metaNum = 0;
        ptrStack->error = 0;

        ptrStack->buffForRes = malloc(ptrStack->size);
        if (ptrStack->buffForRes == NULL)
            error_main(ptrStack, WRITE, BuffForResNull);
        ptrStack->buffForErr = calloc(ptrStack->size, sizeof(char));
        if (ptrStack->buffForErr == NULL)
            error_main(ptrStack, WRITE, BuffForErrNull);

        ptrStack->data = malloc((2 * KAN_NUM + 1) * ptrStack->size);
        if(ptrStack->data != NULL) {    //заполняем канарейки
            ptrStack->num = 1;
            int i;
            const u_int32_t shift = (KAN_NUM + ptrStack->num) * ptrStack->size;

            for(i = 0; i < KAN_NUM * ptrStack->size; i++){
                ((unsigned char*)ptrStack->data)[i] = KAN_VALUE;
                ((unsigned char*)ptrStack->data)[shift + i] = KAN_VALUE;
            }
            for(i = 0; i < ptrStack->size; i++)
                ((unsigned char*)ptrStack->data)[KAN_NUM * ptrStack->size + i]= POISON_VALUE;
            ptrStack->meta = calloc(1, sizeof(char));
            ptrStack->metaNum = 1;
            if(ptrStack->meta == NULL)
                error_main(ptrStack, WRITE, MetaNull);
        }
        else
            error_main(ptrStack, WRITE, DataArrayNull);
    } else
        printf("stackInit: memory error\n");
    return ptrStack;
}

/// Деструктор стека
void stackFree(Stack *ptrStack){
    if(ptrStack != NULL){
        free(ptrStack->data);
        free(ptrStack->buffForErr);
        free(ptrStack->buffForRes);
        free(ptrStack->meta);
    }
    free(ptrStack);
}

/// Array READ function
void *stack_r(Stack *ptrStack, u_int32_t x) {
    if (!stackErrorCheck(ptrStack))
        return stack_main(ptrStack, READ, x, NULL);
    else
        EXIT;
}

/// Array WRITE function
void *stack_w(Stack *ptrStack, u_int32_t x, const void *ptrValue) {
    if (!stackErrorCheck(ptrStack) && ptrValue != NULL)
        return stack_main(ptrStack, WRITE, x, ptrValue);
    else
        EXIT;
}

/// Stack function: Push
void *push(Stack *ptrStack, const void *ptrValue) {
    if (!stackErrorCheck(ptrStack) && ptrValue != NULL)
        return stack_main(ptrStack, WRITE, ptrStack->pos++, ptrValue);
    else
        EXIT;
}

/// Stack function: Pop
void *pop(Stack *ptrStack) {
    if (!stackErrorCheck(ptrStack) && ptrStack->pos > 0) { //ебать костыль соорудил, зато работает!
        --ptrStack->pos;
        //void *buffPtr = malloc(ptrStack->size);
        //myMemCpy(buffPtr, stack_main(ptrStack, READ, ptrStack->pos, NULL), ptrStack->size);
        //stack_reset_pos(ptrStack, ptrStack->pos);
        return stack_main(ptrStack, READ, ptrStack->pos, NULL);         //FIXME!!! утечка памяти
    }
    else
        EXIT;
}

/// Stack function: GetLast
void *getLast(Stack *ptrStack) {
    if (!stackErrorCheck(ptrStack) && ptrStack->pos > 0)
        return stack_main(ptrStack, READ, ptrStack->pos - 1, NULL);
    else
        EXIT;
}

u_int32_t getsize(Stack *ptrStack){
    if(!stackErrorCheck(ptrStack))
        return ptrStack->pos;
    else
        return 0;
}
