Процессор я решил сделать 32 битным подобием x86.

## Описания всех регистров:
1. AX - accumulator, предназначен для сохранения результата исполнения
2. BX - base, 
3. CX - counter,
4. DX - data, 
5. SI - 
6. DI - 
7. SP - 
8. DP - 
9. EFLAGS
   1. CF - carry flag
   2. ZF - zero flag
   3. SF – sign flag
   4. OF – overflow flag
    

## Семантика моего ассемблера:

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

#### call, ret — Subroutine call and return
call <label> – безусловный переход с сохранением текущего положения в регистре\
ret – возврат по значению регистра и еще чото



