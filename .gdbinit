set architecture riscv:rv32
target remote localhost:20000
symbol-file ./xv6-riscv/kernelmemfs
#symbol-file ./xv6-riscv/bootblock.o
#tb *0x100050
