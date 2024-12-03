.data
.dword 0xfffffffff1234567
.text
lui x3, 0x10
ld x4, 0(x3)
addi x5, x3, 20
sd x4, 0(x5)