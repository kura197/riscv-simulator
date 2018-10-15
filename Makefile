
CC = g++
CFLAG = -O3 -g -std=c++11

OUTPUT = emu

OBJ = main.o \
	  emulator.o \
	  csr.o \
	  intr.o \
	  gdb.o \
	  instruction.o

HEADER = emulator.h \
		 csr.h \
		 gdb.h \
		 intr.h \
		 reg.h \
		 instruction.h


$(OUTPUT): $(OBJ)
	$(CC) $(CFLAG) -o $(OUTPUT) $(OBJ)


main.o: main.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o main.o main.cpp

emulator.o: emulator.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o emulator.o emulator.cpp

csr.o: csr.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o csr.o csr.cpp

gdb.o: gdb.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o gdb.o gdb.cpp

instruction.o: instruction.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o instruction.o instruction.cpp

intr.o: intr.cpp $(HEADER)
	$(CC) $(CFLAG) -c -o intr.o intr.cpp

clean:
	rm $(OBJ) $(OUTPUT)
