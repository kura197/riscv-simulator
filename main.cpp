#include <string>
#include <iostream>
#include <fstream>
#include <termios.h>
#include "emulator.h"
#include "instruction.h"
#include "intr.h"
#include "gdb.h"

#define SECTSIZE 512

void ioport(Emulator* emu, ifstream *binary);

using namespace std;

bool FLAGS_d;
int main(int argc, char* argv[]){
    if(argc == 1){
        cout<< "usage: sim [ROM] [-d] [-g] [-t] [-w] [-p gdb_port]" << endl;
        return -1;
    }
    ifstream binary(argv[1],ios::in|ios::binary);
    if(!binary){
        cout << "can not open binary file" << endl;
        return -1;
    }
    int opt;
    int port = 20000;
    bool watch = false;
    bool FLAGS_t = false;
    bool FLAGS_g = false;
    while((opt = getopt(argc, argv, "dgtp:w")) != -1){
        switch(opt){
            case 'd':
                FLAGS_d = true;
                break;
            case 't':
                FLAGS_t = true;
                break;
            case 'g':
                FLAGS_g = true;
                break;
            case 'w':
                watch = true;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                cout<< "usage: sim [ROM] [-d] [-g] [-t] [-w] [-p gdb_port]" << endl;
                return -1;
                break;
        }
    }
    Emulator emu;
    if(!FLAGS_t)
        emu.load_memory(&binary, 0x0, STARTPC, 0x200);
    else{
        emu.load_memory(&binary, 0x0, 0x0, 0x1000);
        emu.PC = 0x0;
    }
    rsp gdb(&emu, port, watch);
    init_instruction();
    uint32_t instr;
    struct termios term;
    struct termios save;
    tcgetattr(STDIN_FILENO, &term);
    save = term;
    term.c_lflag &= ~ECHO;
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    while(1){
        interrupt(&emu);
        if(FLAGS_g && gdb.attach){
            if(gdb.step){
                gdb.stop = true;
                gdb.step = false;
                gdb.sigval = 5;
                gdb.rsp_report_exception();
            }else{
                for(int x : gdb.bp){
                    if(emu.PC == x){
                        gdb.stop = true;
                        gdb.sigval = 5;
                        gdb.rsp_report_exception();
                        break;
                    }
                }
            }
            if(gdb.handle_interrupt_rsp() == 0){
                gdb.sigval = 5;
                //cout << "interrupt signal" << endl;
                gdb.stop = 1;
            }
            while(gdb.stop){
                gdb.handle_rsp();
            }
        }
        instr = emu.fetch();
        //finish
        if(instr == 0x00000000)
            break;
        if(FLAGS_d){
            cout << endl;
            emu.dump_registers(0);
            printf("PC = %08x, Code = %08x, runlevel = %d\n",emu.PC-4,instr,emu.runlevel);
        }
        //instruction
        decoder_t d;
        d = decode(instr);
        instruction[d.opcode](&emu,d);
        ioport(&emu, &binary);
    }
    emu.dump_registers(0);
    printf("program ended successfully at PC = %08x\n", emu.PC-4);
    emu.dump_memory(0xF00,12);

    binary.close();
    tcsetattr(STDIN_FILENO, TCSANOW, &save);
    return 0;
}

void ioport(Emulator* emu, ifstream *binary){
    /*  READ DISK DATA  */
    //0x1F0:	read data
    //0x1F2:	num of read sector??
    //0x1F3 - 0x1F6:	offset
    //0x1F7:	== 0x20 -> read
    //			&0xC0 == 0x40 -> ready to read 

    if(emu->get_mem8(IO_BASE+0x1F7) == 0x20){
        int32_t offset = emu->get_mem32(IO_BASE+0x1F3) * SECTSIZE;
        int32_t size = emu->get_mem8(IO_BASE+0x1F2) * SECTSIZE;
        if(FLAGS_d){
            emu->dump_memory(IO_BASE+0x1F0,7);
        }
        emu->read_sector(binary, offset, size);
    }
    emu-> store_mem8(IO_BASE+0x1F7, 0x40);

    /*  READ DISK DATA test  */
    //0x1E3:	read op
    //0x1E4 - 0x1E7:	phisical address
    //0x1E8 - 0x1EB:	read address(offset)
    //0x1EC - 0x1EF:	read size
    if(emu->get_mem8(IO_BASE+0x1E3) == 1){
        emu-> store_mem8(IO_BASE+0x1E3, 0);
        uint32_t pa = emu->get_mem32(IO_BASE+0x1E4);
        uint32_t offset = emu->get_mem32(IO_BASE+0x1E8) * SECTSIZE;
        uint32_t size = emu->get_mem32(IO_BASE+0x1EC);
        if(FLAGS_d){
            emu->dump_memory(IO_BASE+0x1E4,12);
        }
        emu->load_memory(binary, offset, pa, size);
    }

    /* UART */
    //0x3F8	Write/Read
    //0x3F8+5:buffer empty flag
    //0x3F9 - 0x3FF	Reserved
    if(!emu->uart_tx.empty()){
        char ch = emu->uart_tx.front();
        emu->uart_tx.pop();
        printf("%c",ch);
        fflush(stdout);
    }
}


