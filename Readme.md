# Эмулятор процессора
## Структура проекта:

### Процессор я решил сделать подобием x86.

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
   1. ZF - zero flag
   2. SF – sign flag

## Семантика моего ассемблера:
##### Условно договорюсь большими буквами обозначать операторы, маленькими - операнды и названия

#### Операторы:
Регистры обозначаются так: `%reg` где `reg` - имя соответствующего регистра.

Полная адресация выглядит так:\
`(POINTER,INDEX,STEP)` – similar [D + POINTER + STEP * INDEX]\
For an example:
`(%reg)` - is a pointed by `reg` memory.

Константы:`$CONST`

##### Переменные:
1. **DV <name>**
2. **DA <name> <count>**

### Data Movement Instructions:

1. #### MOV \<op1> \<op2> – op2 = op1
###### Потом тут всякие варики типа movq накидать
2. #### PUSH \<reg>
###### а сюда описание (но пока думаю понятно что это делает)
3. #### POP \<reg>

### Stack arithmetic and logic instructions (Справа приписал Си-аналоги выражений):

Операции берут операнды из стека для арифметики и результат возвращают обратно в стек. 
Его можно будет достать командой POP.

1. **ADD**  – op1 = op1 + op2.
2. **SUB** – op1 = op1 - op2.
3. **INC** – op++.
4. **DEC** - op--.
4. **IMUL** – op1 *= op2, op0 = op1 * op2.
5. **IDIV** – op1 /= op2, op0 = op1 / op2.
6. **AND, OR, XOR** – op1 = op1 &|^ op2. //bitwise operator
7. **NOT** – op = ~op.
8. **NEG** – op = -op.
9. **SHL, SHR, SHRI** – op1 << op2, (unsigned) op1 >> op2, (signed) op1 >> op2.
10. **CMP** - как вычисление a - b без возвращения значения
    1. ZF = 1, если a == b
    2. SF = 1, если (a - b) < 0

### Control Flow instructions:

Labels encode by: `.<name>`

#### J* – условный переход
* JMP \<label> (<pointer>) (jump)
* JE \<label> (<pointer>) (jump when equal)
* JNE \<label> (<pointer>) (jump when not equal)
* JZ \<label> (<pointer>) (jump when last result was zero)
* JG \<label> (<pointer>) (jump when greater than)
* JGE \<label> (<pointer>) (jump when greater than or equal to)
* JL \<label> (<pointer>) (jump when less than)
* JLE \<label> (<pointer>) (jump when less than or equal to)

#### CALL_lbl, RET — Subroutine call and return (жесть этож имба)
CALL <label> – безусловный переход с сохранением текущего положения в стеке\
RET – возврат по вершине стека

#### End of execution:
END – closes program

