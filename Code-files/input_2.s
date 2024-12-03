.data
.dword 3, 12, 3, 125, 50, 32, 16
.text
    lui x3, 0x10
    addi x10, x3, 200
    ld x4, 0(x3)
    addi x3, x3, 8
loop: beq x4, x0, Exit
    ld x5, 0(x3)
    ld x6, 8(x3)
    add x9, x0, x0
    beq x5, x0, store
    beq x6, x0, store
find_gcd: bge x5, x6, swap
    add x9, x0, x5
    add x7, x0, x5
    add x8, x0, x6
div_of_a: blt x7, x0, change_gcd
    beq x7, x0, div_of_b
    sub x7, x7, x9
    beq x0, x0, div_of_a
div_of_b: blt x8, x0, change_gcd
    beq x8, x0, store
    sub x8, x8, x9
    beq x0, x0, div_of_b
change_gcd: addi x9, x9, -1
    add x7, x0, x5
    add x8, x6, x0
    beq x0, x0, div_of_a
swap: add x13, x5, x0
    add x5, x6, x0
    add x6, x13, x0
    beq x0, x0, find_gcd
store: sd x9, 0(x10)
    addi x10, x10, 8
    addi x3, x3, 16
    addi x4, x4, -1
    beq x0, x0, loop
Exit: add x2, x0, x0