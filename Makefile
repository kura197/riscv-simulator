CC = g++ -g
OUT = emu_risc-v

run:$(OUT) test.bin
	./$(OUT) test.bin

gdb:$(OUT) test.bin
	gdb $(OUT) 

$(OUT):main.o instruction.o csr.o trap.o
	$(CC) *.o -o $(OUT)
	#rm *.o

main.o:main.cpp instruction.cpp csr.cpp trap.cpp
	$(CC) *.cpp  -c

clean:
	rm *.o
