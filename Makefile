CC = g++ -g
OUT = emu_risc-v

run:$(OUT) test.bin
	./$(OUT) test.bin

gdb:$(OUT) test.bin
	gdb $(OUT) 

$(OUT):main.o instruction.o emulator.hpp
	$(CC) *.o -o $(OUT)

main.o:main.cpp instruction.cpp
	$(CC) *.cpp  -c

clean:
	rm *.o
