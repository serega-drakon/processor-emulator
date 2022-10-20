#include "compilator.h"
#include "../memory/memory.h"
#include <stdio.h>
#include <ctype.h>

#define MAXOP 100

///Получает следующий операнд или оператор из файла\n
///Возвращает длину найденной строки\n
///\warning Подразумеваем что размер op[] достаточный
int getOp(FILE* input, int op[]){ //works!
    int c; int i = 0;
    while((c = getc(input)) == ' ' || c == '\t' || c == '\n');
    if(c != EOF)
        op[i++] = c;
    while((c = getc(input)) != ' ' && c != '\t' && c != '\n' && c != EOF) {
        op[i] = c;
        i++;
    }
    op[i] = '\0';
    return i;
}

int compareStr(const int a[], const char b[]){
    int j;
    int flag = 1;

    for(j = 0; flag && a[j] != '\0' && b[j] != '\0'; j++) {
        if (a[j] != b[j]) flag = 0;
        printf("||%c –– %c\n", a[j], b[j]);
    }
    if(flag && a[j] == '\0' && b[j] == '\0')
        return 1;
    return 0;
}

enum countOfOps_{
    CountOfRegs = 8,
    CountOfQuadOps = 2,//
};

enum ops_{
    Error = -1,
    Nothing = 0,
    Register,
    Const = Register + CountOfRegs,
    Pointer,
    QuadOperator,
    Smth = QuadOperator + CountOfQuadOps,

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

const char *quadOperators_[] = {
    "MOVQ",
    "ADDQ",
};

///Отображает множество строк на множество целых чисел *три крутых смайлика*\n
///Ну или это словарь операторов и операндов, если по-человечески
//решил делать не деревом, а менее эффективным методом прямой проверки с каждым операндом/оператором,
// зато дебажить проще простого
int getType(int op[]){
    if(op[0] == '$') {
        if(op[1] == '\0')
            return Error;
        for (int i = 1; op[i] != '\0'; i++)
            if (!isdigit(op[i])) return Error;
        return Const;
    }
    int i,j;
    char flag = 1;
    if(op[0] == '%'){
        for(i = 0; i < CountOfRegs; i++){
            if(compareStr(&op[1], registers_[i]))
                return Register + i;
        }
        return Error;
    }
    //суда ссылки потом

    //самые последние пойдут операторы
    for(i = 0; i < CountOfQuadOps; i++){
        if(compareStr(op, quadOperators_[i]))
            return QuadOperator + i;
    }
    return Error;
}

unsigned int translateConst(int op[]){}

int compileFile(FILE* input, Stack* ptrOutput){
    int op[ MAXOP ];
    int type;
    while(getOp(input, op) > 0){
        type = getType(op);
        switch(type){  //FIXME: debug
            case Const:
                printf("Const\n");
                for(int i = 0; op[i] != '\0'; i++)
                    printf("%c", op[i]);
                printf("\n");
                break;
            case Register:
            case Register + 1:
            case Register + 2:
                //...
                printf("Register\n");
                for(int i = 0; op[i] != '\0'; i++)
                    printf("%c", op[i]);
                printf("\n");
                break;
            case QuadOperator:
                printf("QOP 0\n");
                for(int i = 0; op[i] != '\0'; i++)
                    printf("%c", op[i]);
                printf("\n");
                break;
            case QuadOperator + 1:
                printf("QOP 1\n");
                for(int i = 0; op[i] != '\0'; i++)
                    printf("%c", op[i]);
                printf("\n");
                break;
            default:
                printf("Error\n");
                for(int i = 0; op[i] != '\0'; i++)
                    printf("%c", op[i]);
                printf("\n");
                break;
        }
    }

}