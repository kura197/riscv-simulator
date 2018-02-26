CC = riscv32-unknown-linux-gnu-gcc
AS = riscv32-unknown-linux-gnu-as
LD = riscv32-unknown-linux-gnu-ld
OBJCOPY = riscv32-unknown-linux-gnu-objcopy
OBJDUMP = riscv32-unknown-linux-gnu-objdump
CCFLAGS = -nostdlib -nostartfiles -Ttest.ls 

test.bin:test.s
	$(CC) $(CCFLAGS) test.s  -o test.o
	$(LD) test.o -o test.bin -Ttest.ls --oformat binary

test.s:test.c
	$(CC) -O0 test.c -S -o test.s

dump:test.o
	$(OBJDUMP) test.o -S

clean:
	rm test.o test.bin

#test.bin:test.s
#	$(AS) test.s -o test.o
#	#$(LD) --oformat binary test.o -o test.bin
#	$(OBJCOPY) test.o test.bin -O binary
#
#test.s:test.c
#	$(CC) test.c -S -o test.s
