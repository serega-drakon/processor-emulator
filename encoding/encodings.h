#ifndef PROCESSOR_EMULATOR_ENCODINGS_H
#define PROCESSOR_EMULATOR_ENCODINGS_H

#define REG_SIZE 4          //Размер регистра
#define POINTER_SIZE 13     //Размер указателя

///Это не кодировка, а просто перечисление количества соответствущих типов команд
enum count_{
    CountOfRegs = 8,
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
    ADD, SUB, INC, IMUL,
    IDIV, AND, OR, XOR,
    NOT, NEG, SHL, SHR,
    SHRL, CMP,
    JMP_lbl, JE_lbl, JNE_lbl, JZ_lbl, JG_lbl,
    JGE_lbl, JL_lbl, JLE_lbl, CALL_lbl,
    RET, END, PRINT_reg,
    JMP_ptr, JE_ptr, JNE_ptr, JZ_ptr, JG_ptr,
    JGE_ptr, JL_ptr, JLE_ptr, CALL_ptr, //варианты когда нужно перейти по значению, лежащему по данной ссылке

};

///Это костыль для getType, чтобы на основе одного вывода можно было бы сделать кодировку всего что может быть \n
///У таких типов как pointer или label слишком сложная структура, чтобы доставать их из одной функции вместе с остальными,
enum others_{
    NotDefined = -2, //It may be name of variable
    Error = -1, //it exactly is error
    Nothing = 0,
    QUAD = CALL_ptr, DV, DA, //не кодируемые команды (стоило бы переместить в others)
    Register, //регистры по задумке не должны кодироваться, записал их для удобства вывода из getType()
    Const16 = Register + CountOfRegs,
    Const10,
    Pointer,
    Label,
    RegAX = Register,
    RegBX,
    RegCX,
    RegDX,
    RegSI,
    RegDI,
    RegSP,
    RegDP,
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

#endif //PROCESSOR_EMULATOR_ENCODINGS_H
