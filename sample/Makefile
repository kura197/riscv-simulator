CC = riscv32-unknown-elf-gcc
AS = riscv32-unknown-elf-as
LD = riscv32-unknown-elf-ld
OBJCOPY = riscv32-unknown-elf-objcopy
OBJDUMP = riscv32-unknown-elf-objdump

test.bin:test.s
	$(AS) test.s -o test.o
	#$(LD) --oformat binary test.o -o test.bin
	$(OBJCOPY) test.o test.bin -O binary

test.s:test.c
	$(CC) test.c -S -o test.s

dump:test.o
	$(OBJDUMP) test.o -S

clean:
	rm test.o test.bin
