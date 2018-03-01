
#BIN = xv6memfs.img
BIN = xv6.img


CC = g++ -g
OUT = emu_risc-v

run:$(OUT) $(BIN)
	./$(OUT) $(BIN)

gdb:$(OUT) $(BIN)
	gdb $(OUT) 

$(OUT):main.o instruction.o csr.o trap.o
	$(CC) *.o -o $(OUT)
	#rm *.o

main.o:main.cpp instruction.cpp csr.cpp trap.cpp
	$(CC) *.cpp  -c

clean:
	rm *.o
