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
#### operators:
Регистры обозначаются так: `%reg` где `reg` - имя соответствующего регистра.\
`[%reg]` - is a pointed by `reg` memory.\
Полная адресация выглядит так:\
`D[POINTER,INDEX,STEP]` – similar [D + POINTER + STEP * INDEX]

### Data Movement Instructions:

1. #### mov \<op1> \<op2> – op2 = op1
###### Потом тут всякие варики типа movq накидать
2. #### push \<op>
###### а сюда описание (но пока думаю понятно что это делает)
3. #### pop \<op>

4. #### lea \<reg> \<mem> – reg = [mem] 

### Arithmetic and logic instructions (Справа приписал Си-аналоги выражений):

1. **add \<op1> \<op2>**  – op1 = op1 + op2.
2. **sub \<op1> \<op2>** – op1 = op1 - op2.
3. **inc \<op>, dec \<op>** – op++, op--.
4. **imul \<op1> \<op2>, imul \<op0> \<op1> \<op2>** – op1 *= op2, op0 = op1 * op2.
5. **idiv \<op1> \<op2>, idiv \<op0> \<op1> \<op2>** – op1 /= op2, op0 = op1 / op2.
6. **and, or, xor \<op1> \<op2>** – op1 = op1 &|^ op2. //bitwize operator
7. **not <op>** – op = ~op.
8. **neg <op>** – op = -op.
9. **shl, shr, shrl \<op1> \<op2>** – op1 << op2, (unsgined) op1 >> op2, (signed) op1 >> op2.

### Control Flow instructions:

#### j* – условный переход
* jmp \<label> (jump)
* je \<label> (jump when equal)
* jne \<label> (jump when not equal)
* jz \<label> (jump when last result was zero)
* jg \<label> (jump when greater than)
* jge \<label> (jump when greater than or equal to)
* jl \<label> (jump when less than)
* jle \<label> (jump when less than or equal to)

#### call, ret — Subroutine call and return (жесть этож имба)
call <label> – безусловный переход с сохранением текущего положения в стеке\
ret – возврат по вершине стека

###### Заметка: после отладки этого добавить поддержку строк

