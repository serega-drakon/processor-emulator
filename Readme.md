### Процессор я решил сделать 32 битным подобием x86.

## Описания всех регистров:
#### Все регистры - 32 битные.
1. AX - accumulator
2. BX - base
3. CX - counter
4. DX - data
5. SI - stack index
6. DI - data index
7. SP - stack pointer
8. DP - data pointer
9. EFLAGS
   1. CF - carry flag
   2. ZF - zero flag
   3. SF – sign flag
   4. OF – overflow flag

## Семантика моего ассемблера:
##### Условно договорюсь большими буквами обозначать операторы, маленькими - операнды и названия

#### operators:
Регистры обозначаются так: `%reg` где `reg` - имя соответствующего регистра.

Полная адресация выглядит так:\
`D[POINTER,INDEX,STEP]` – similar [D + POINTER + STEP * INDEX]\
`[%reg]` - is a pointed by `reg` memory.

Константы:`$CONST`

##### Переменные.
Allocating Storage Space for Initialized Data:
1. **\<name> DB \<const>** – 1 byte
2. **\<name> DW \<const>** – 2 bytes
3. **\<name> DD \<const>** – 4 bytes
 
Allocating Storage Space for Uninitialized Data
1. **\<name> RESB** – 1 byte
2. **\<name> RESW** – 2 bytes
3. **\<name> RESD** – 4 bytes

###### мб добавить поддержку 8- и 16-ых СС

### Data Movement Instructions:

1. #### MOV \<op1> \<op2> – op2 = op1
###### Потом тут всякие варики типа movq накидать
2. #### PUSH \<op>
###### а сюда описание (но пока думаю понятно что это делает)
3. #### POP \<op>

4. #### LEA \<reg> \<mem> – reg = *[mem] 

### Arithmetic and logic instructions (Справа приписал Си-аналоги выражений):

1. **ADD \<op1> \<op2>**  – op1 = op1 + op2.
2. **SUB \<op1> \<op2>** – op1 = op1 - op2.
3. **INC \<op>, DEC \<op>** – op++, op--.
4. **IMUL \<op1> \<op2>, IMUL \<op0> \<op1> \<op2>** – op1 *= op2, op0 = op1 * op2.
5. **IDIV \<op1> \<op2>, IDIV \<op0> \<op1> \<op2>** – op1 /= op2, op0 = op1 / op2.
6. **AND, OR, XOR \<op1> \<op2>** – op1 = op1 &|^ op2. //bitwize operator
7. **NOT <op>** – op = ~op.
8. **NEG <op>** – op = -op.
9. **SHL, SHR, SHRI \<op1> \<op2>** – op1 << op2, (unsgined) op1 >> op2, (signed) op1 >> op2.

### Control Flow instructions:

#### J* – условный переход
* JMP \<label> (jump)
* JE \<label> (jump when equal)
* JNE \<label> (jump when not equal)
* JZ \<label> (jump when last result was zero)
* JG \<label> (jump when greater than)
* JGE \<label> (jump when greater than or equal to)
* JL \<label> (jump when less than)
* JLE \<label> (jump when less than or equal to)

#### CALL, RET — Subroutine call and return (жесть этож имба)
CALL <label> – безусловный переход с сохранением текущего положения в стеке\
RET – возврат по вершине стека
###### Потом тут всякие варики типа retq накидать

###### Заметка: после отладки этого добавить поддержку строк

