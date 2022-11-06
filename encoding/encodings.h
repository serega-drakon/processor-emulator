#ifndef PROCESSOR_EMULATOR_ENCODINGS_H
#define PROCESSOR_EMULATOR_ENCODINGS_H

#define REG_SIZE 4          //Размер регистра
#define POINTER_SIZE 13     //Размер указателя

///Это не кодировка, а просто перечисление количества соответствущих типов команд
enum count_{
    CountOfRegs = 8,
    CountOfOperators2arg = 1,
    CountOfStackOperators = 2,
    CountOfStackArithmetics = 14,
    CountOfDefineVars = 2,
    CountOfProgramControlArg = 8,
    CountOfProgramControlNoArg = 2
};

///Кодировка всех команд
enum encodingOps_{
    MOV_reg_reg = 1,
    MOV_reg_const,
    MOV_reg_mem,
    MOV_mem_reg,
    MOV_mem_const,
    PUSH_reg,
    POP_reg,
    ADD, SUB, INC, IMUL, IDIV, AND, OR, XOR, NOT, NEG, SHL, SHR, SHRL, CMP,
    DV, DA,
    JMP_lbl, JE_lbl, JZ_lbl, JG_lbl, JGE_lbl, JL_lbl, JLE_lbl, CALL_lbl,
    JMP_ptr, JE_ptr, JZ_ptr, JG_ptr, JGE_ptr, JL_ptr, JLE_ptr, CALL_ptr, //варианты когда нужно перейти по значению, лежащему по данной ссылке
    RET, QUAD, END
};

///Это костыль для getType, чтобы на основе одного вывода можно было бы сделать кодировку всего что может быть \n
///У таких типов как pointer или label слишком сложная структура, чтобы доставать их из одной функции вместе с остальными,
enum others_{
    NotDefined = -2, //It may be name of variable
    Error = -1, //it exactly is error
    Nothing = 0,
    Register = END + 1, //регистры по задумке не должны кодироваться, записал их для удобства вывода из getType()
    Const16 = Register + CountOfRegs,
    Const10,
    Pointer,
    Label
};

///Кодировка всех регистров
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

///Кодировка указателей
enum ptrTypes_{  //сделал специально удобную кодировку(см. ф. getpointer)
    Ptr_reg_reg_reg,      //000
    Ptr_reg_reg_const,    //001
    Ptr_reg_const_reg,    //010
    Ptr_reg_const_const,  //011
    Ptr_const_reg_reg,    //100
    Ptr_const_reg_const,  //101
    Ptr_const_const_reg,  //110
    Ptr_const_const_const //111
};

///Представления команд
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
        "QUAD",
        "END"
};

///Представления регистров %..
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


#endif //PROCESSOR_EMULATOR_ENCODINGS_H
