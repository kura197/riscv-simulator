
CC = g++
CFLAGS = -O3 -g -std=c++14

OUTPUT = riscv_simulator

OBJS = main.o \
	  emulator.o \
	  csr.o \
	  intr.o \
	  gdb.o \
	  instruction.o

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o $(OUTPUT)
