#ifndef GDB_H_
#define GDB_H_

#include <iostream>
#include <string>
#include <vector>
#include "emulator.hpp"

/*! Size of the matchpoint hash table. Largest prime < 2^10 */
#define MP_HASH_SIZE  1021
#define GDB_BUF_MAX  ((NUM_REGS) * 8 + 1)
#define RSP_TRACE 1

/*! Enumeration of different types of matchpoint. These have explicit values
  matching the second digit of 'z' and 'Z' packets. */
enum mp_type {
	BP_MEMORY   = 0,
	BP_HARDWARE = 1,
	WP_WRITE    = 2,
	WP_READ     = 3,
	WP_ACCESS   = 4
};

static const char hexchars[]="0123456789abcdef";

// vectorからindex番目の要素を削除する
template<typename T>
void remove(std::vector<T>& vector, unsigned int index)
{
    vector.erase(vector.begin() + index);
}

class rsp{
	int                client_waiting;	/*!< Is client waiting a response? */
	int                proto_num;		/*!< Number of the protocol used */
	int                client_fd;		/*!< FD for talking to GDB */
	int                sigval;		/*!< GDB signal for any exception */
	unsigned long int  start_addr;	/*!< Start of last run */
	Emulator *gdb_emu;

public:
	bool stop = false;
	bool step = false;
	vector<unsigned int> bp;

	rsp(Emulator*);
	~rsp(){ rsp_client_close(); };
	void handle_rsp();
	//return 0 if success
	int rsp_get_client();
	int get_rsp_char();
	void put_rsp_char(char c);
	void rsp_client_request ();
	void rsp_client_close ();
	string get_packet();
	int convert_hex(int c);
	void rsp_query(string buf);
	void put_str_packet (const string str);
	void put_packet (string buf);
	void rsp_report_exception();
	void rsp_vpkt (string buf);
	void rsp_read_all_regs();
	void reg2hex (unsigned long int val, string* buf);
	void rsp_read_reg(string buf);
	void rsp_read_mem (string buf);
	void rsp_insert_matchpoint (string buf);
	void rsp_remove_matchpoint (string buf);
	void rsp_continue(string buf);
	void rsp_step (string buf);
};

#endif
