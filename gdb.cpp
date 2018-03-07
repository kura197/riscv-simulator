#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <gflags/gflags.h>
#include "gdb.hpp"
#include "emulator.hpp"
using namespace std;

DEFINE_int32(port, 20000, "gdb remote port");
DEFINE_bool(p, false, "watch packets");


rsp::rsp(Emulator* emu){
	/* Clear out the central data structure */
	client_waiting =  0;		/* GDB client is not waiting for us */
	client_fd      = -1;		/* i.e. invalid */
	sigval         =  0;		/* No exception */
	stop = true;
	attach = true;
	tv.tv_sec = 0;
	tv.tv_usec = 20;

	gdb_emu = emu;
}

void rsp::handle_rsp (){
	/* If we have no RSP client, wait until we get one. */
	while (-1 == client_fd)
	{
		rsp_get_client ();
		client_waiting = 0;		/* No longer waiting */
	}

//	/* If we have an unacknowledged exception tell the GDB client. If this
//	   exception was a trap due to a memory breakpoint, then adjust the NPC. */
//	if (client_waiting){
//		if ((TARGET_SIGNAL_TRAP == sigval) && (NULL != mp_hash_lookup (BP_MEMORY, cpu_state.sprs[SPR_PPC]))){
//			set_npc (cpu_state.sprs[SPR_PPC]);
//		}
//		rsp_report_exception();
//		client_waiting = 0;		/* No longer waiting */
//	}

	/* Get a RSP client request */
	rsp_client_request ();

}	/* handle_rsp () */

void rsp::rsp_client_request (){
	string buf = get_packet ();	// Message sent to us 

	// Null packet means we hit EOF or the link was closed for some other
	// reason. Close the client and return
	if (buf.empty() == true){
		rsp_client_close ();
		return;
	}

	if(FLAGS_p){
		//printf ("Packet received %s: %d chars\n", buf, buf.length());
		cout << "Packet received " << buf << ": " << buf.length() << "chars" << endl;
		fflush (stdout);
	}

	switch (buf[0]){
		case 'q':
			rsp_query(buf);
			return;
		case '?':
			/* Return last signal ID */
			rsp_report_exception();
			return;
		case 'H':
			/* Set the thread number of subsequent operations. For now ignore
			   silently and just reply "OK" */
			put_str_packet ("OK");
			return;
		case 'v':
			/* Any one of a number of packets to control execution */
			rsp_vpkt (buf);
			return;
		case 'g':
			rsp_read_all_regs();
			return;
		case '!':
			put_str_packet ("OK");
			return;
		case 'p':
			rsp_read_reg (buf);
      		return;
		case 'm':
			rsp_read_mem (buf);
			return;
		case 'z':
			rsp_remove_matchpoint (buf);
			return;
		case 'Z':
			rsp_insert_matchpoint (buf);
			return;
		case 'c':
			rsp_continue (buf);
			return;
		case 's':
			rsp_step (buf);
			return;
		case 'D':
			put_str_packet ("OK");
			rsp_client_close ();
			attach = false;
			stop = false;
			//sigval = TARGET_SIGNAL_NONE;	/* No signal now */

			return;
	}
}

void rsp::rsp_query(string buf){
	if(buf.find("qSupported") != -1){
		string reply = "PacketSize=" + to_string(GDB_BUF_MAX);
		put_str_packet (reply);

	}else if(buf.find("qC") != -1){
		put_str_packet("");

	}else if(buf.find("qTStatus") != -1){
		put_str_packet("");

	}else if(buf.find("qOffsets") != -1){
		put_str_packet ("Text=0;Data=0;Bss=0");

	}else if(buf.find("qSymbol") != -1){
		put_str_packet ("OK");

	}else if(buf.find("qAttached") != -1){
		put_str_packet ("1");

	}else if(buf.find("qfThreadInfo") != -1){
		put_packet ("m1");

	}else if(buf.find("qsThreadInfo") != -1){
		put_packet ("l");
	}
}

void rsp::rsp_vpkt (string buf){
	if(buf.find("vMustReplyEmpty") != -1){
		put_str_packet("");
		return;
	}if(buf.find("vCont?") != -1){
		put_str_packet("");
		return;
	}if(buf.find("vCont") != -1){
		fprintf(stderr, "Warning: RSP vCont not supported: ignored\n" );
		return;
	}
}

int rsp::rsp_get_client(){
	int                 tmp_fd;		/* Temporary descriptor for socket */
	int                 optval;		/* Socket options */
	struct sockaddr_in  sock_addr;	/* Socket address */
	socklen_t           len;		/* Size of the socket address */

	/* Open a socket on which we'll listen for clients */
	tmp_fd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tmp_fd < 0)
	{
		fprintf (stderr, "ERROR: Cannot open RSP socket\n");
		//sim_done ();
		return 1;
	}

	/* Allow rapid reuse of the port on this socket */
	optval = 1;
	setsockopt (tmp_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
			sizeof (optval));

	/* Bind the port to the socket */
	sock_addr.sin_family      = PF_INET;
	sock_addr.sin_port        = htons (FLAGS_port);
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind (tmp_fd, (struct sockaddr *) &sock_addr, sizeof (sock_addr)))
	{
		fprintf (stderr, "ERROR: Cannot bind to RSP socket\n");
		//sim_done ();
		return 1;
	}

	/* Listen for (at most one) client */
	if (listen (tmp_fd, 1))
	{
		fprintf (stderr, "ERROR: Cannot listen on RSP socket\n");
		//sim_done ();
		return 1;
	}

	printf ("Listening for RSP on port %d\n", FLAGS_port);
	fflush (stdout);

	/* Accept a client which connects */
	len = sizeof (socklen_t);		/* Bug fix by Julius Baxter */
	client_fd = accept (tmp_fd, (struct sockaddr *)&sock_addr, &len);

	if (-1 == client_fd)
	{
		fprintf (stderr, "Warning: Failed to accept RSP client\n");
		return 1;
	}

	/* Enable TCP keep alive process */
	optval = 1;
	setsockopt (client_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval,
			sizeof (optval));

	/* Don't delay small packets, for better interactive response (disable
	   Nagel's algorithm) */
	optval = 1;
	setsockopt (client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&optval,
			sizeof (optval));

	/* Socket is no longer needed */
	close (tmp_fd);			/* No longer need this */
	signal (SIGPIPE, SIG_IGN);		/* So we don't exit if client dies */

	printf ("Remote debugging from host %s\n", inet_ntoa (sock_addr.sin_addr));

	FD_ZERO(&readfd);
	FD_SET(client_fd, &readfd);

	return 0;

}	/* rsp_get_client () */

void rsp::rsp_client_close (){
	if (-1 != client_fd){
		close (client_fd);
		client_fd = -1;
	}
}	/* rsp_client_close () */

string rsp::get_packet (){
	string buf;		/* Survives the return */

	/* Keep getting packets, until one is found with a valid checksum */
	while(1){
		unsigned char  checksum;		/* The checksum we have computed */
		int            count;		/* Index into the buffer */
		int 	     ch;		/* Current character */

		/* Wait around for the start character ('$'). Ignore all other
		   characters */
		ch = get_rsp_char ();
		while(ch != '$'){
			if (-1 == ch){
				return  NULL;		/* Connection failed */
			}
			ch = get_rsp_char ();
		}

		/* Read until a '#' or end of buffer is found */
		checksum =  0;
		count    =  0;
		while(count < GDB_BUF_MAX - 1){
			ch = get_rsp_char ();

			/* Check for connection failure */
			if (-1 == ch){
				return  NULL;
			}

			/* If we hit a start of line char begin all over again */
			if ('$' == ch){
				checksum =  0;
				count    =  0;
				continue;
			}

			/* Break out if we get the end of line char */
			if ('#' == ch){
				break;
			}

			/* Update the checksum and add the char to the buffer */

			checksum        = checksum + (unsigned char)ch;
			buf	 += (char)ch;
			count           = count + 1;
		}

		/* Mark the end of the buffer with EOS - it's convenient for non-binary
		   data to be valid strings. */
		//buf[count] = 0;
		//buf.len         = count;

		/* If we have a valid end of packet char, validate the checksum */
		if ('#' == ch){
			unsigned char  xmitcsum;	/* The checksum in the packet */

			ch = get_rsp_char ();
			if (-1 == ch){
				return  NULL;		/* Connection failed */
			}
			xmitcsum = convert_hex(ch) << 4;

			ch = get_rsp_char ();
			if (-1 == ch){
				return  NULL;		/* Connection failed */
			}

			xmitcsum += convert_hex(ch);

			/* If the checksums don't match print a warning, and put the
			   negative ack back to the client. Otherwise put a positive ack. */
			if (checksum != xmitcsum){
				fprintf (stderr, "Warning: Bad RSP checksum: Computed "
						"0x%02x, received 0x%02x\n", checksum, xmitcsum);

				put_rsp_char('-');	/* Failed checksum */
			}
			else{
				put_rsp_char('+');	/* successful transfer */
				break;
			}
		}
		else{
			fprintf (stderr, "Warning: RSP packet overran buffer\n");
		}
	}

	return buf;				/* Success */

}	/* get_packet () */

int rsp::get_rsp_char (){
	if (-1 == client_fd){
		fprintf (stderr, "Warning: Attempt to read from unopened RSP "
				"client: Ignored\n");
		return  -1;
	}

	/* Non-blocking read until successful (we retry after interrupts) or
	   catastrophic failure. */
	while(1){
		unsigned char  c;
		switch (read (client_fd, &c, sizeof(c))){
			case -1:
				/* Error: only allow interrupts */
				if ((EAGAIN != errno) && (EINTR != errno))
				{
					fprintf (stderr, "Warning: Failed to read from RSP client: "
							"Closing client connection: %s\n",
							strerror (errno));
					rsp_client_close ();
					return  -1;
				}
				break;

			case 0:
				// EOF
				rsp_client_close ();
				return  -1;

			default:
				return  c & 0xff; /* Success, we can return (no sign extend!) */
		}
	}
}	/* get_rsp_char () */

void rsp::put_rsp_char(char c){
	if (-1 == client_fd){
		fprintf (stderr, "Warning: Attempt to write '%c' to unopened RSP "
				"client: Ignored\n", c);
		return;
	}

	/* Write until successful (we retry after interrupts) or catastrophic
	   failure. */
	while (1){
		switch (write (client_fd, &c, sizeof (c))){
			case -1:
				/* Error: only allow interrupts or would block */
				if ((EAGAIN != errno) && (EINTR != errno)){
					fprintf (stderr, "Warning: Failed to write to RSP client: "
							"Closing client connection: %s\n",
							strerror (errno));
					rsp_client_close ();
					return;
				}
				break;

			case 0:
				break;		/* Nothing written! Try again */

			default:
				return;		/* Success, we can return */
		}
	}
}	/* put_rsp_char () */

int rsp::convert_hex(int c){
	return  ((c >= 'a') && (c <= 'f')) ? c - 'a' + 10 :
		((c >= '0') && (c <= '9')) ? c - '0' :
		((c >= 'A') && (c <= 'F')) ? c - 'A' + 10 : -1;
}	/* hex () */


void rsp::put_str_packet (const string str){
	//string  buf;
	int len = str.length();

	/* Construct the packet to send, so long as string is not too big,
	   otherwise truncate. Add EOS at the end for convenient debug printout */

	if (len >= GDB_BUF_MAX)
	{
		fprintf (stderr, "Warning: String %s too large for RSP packet: "
				"truncated\n", str);
		len = GDB_BUF_MAX - 1;
	}

	//strncpy (buf.data, str, len);
	//buf.data[len] = 0;
	//buf.len       = len;

	put_packet (str);

}	/* put_str_packet () */

void rsp::put_packet (string buf){
	int  ch;				/* Ack char */

	/* Construct $<packet info>#<checksum>. Repeat until the GDB client
	   acknowledges satisfactory receipt. */
	do{
		unsigned char checksum = 0;	/* Computed checksum */
		int           count    = 0;	/* Index into the buffer */

		if(FLAGS_p){
			//printf ("Putting %s\n", buf);
			cout << "Putting " << buf << endl;
			fflush (stdout);
		}
		put_rsp_char ('$');		/* Start char */
		/* Body of the packet */
		for (count = 0; count < buf.length(); count++){
			unsigned char  ch = buf[count];

			/* Check for escaped chars */
			if (('$' == ch) || ('#' == ch) || ('*' == ch) || ('}' == ch))
			{
				ch       ^= 0x20;
				checksum += (unsigned char)'}';
				put_rsp_char ('}');
			}

			checksum += ch;
			put_rsp_char (ch);
		}

		put_rsp_char ('#');		/* End char */

		/* Computed checksum */
		put_rsp_char (hexchars[checksum >> 4]);
		put_rsp_char (hexchars[checksum % 16]);

		/* Check for ack of connection failure */
		ch = get_rsp_char ();
		if (-1 == ch){
			return;			/* Fail the put silently. */
		}
	}while ('+' != ch);

}	/* put_packet () */

void rsp::rsp_report_exception(){
  string  buf = "";
  /* Construct a signal received packet */
  buf += 'S';
  buf += hexchars[sigval >> 4];
  buf += hexchars[sigval % 16];
  //buf += "0";

  put_packet (buf);
}	/* rsp_report_exception () */


void rsp::rsp_read_all_regs(){
	string  buf = "";			/* Buffer for the reply */
	int     r;			/* Register index */

	/* The GPRs */
	for (r = 0; r < USER_REG_CNT; r++){
		reg2hex (gdb_emu->x[r], &buf);
	}
	//reg2hex (gdb_emu->PC-4, &buf);
	//reg2hex (gdb_emu->PC, &buf);
	//reg2hex (gdb_emu->x[r], &buf);

	put_packet (buf);

}	/* rsp_read_all_regs () */

void rsp::reg2hex (unsigned long int val, string* buf){
	int  n;			/* Counter for digits */
	int nyb_shift;
	for (n = 0; n < 4; n++){
		nyb_shift = 8 * n + 4;
		*buf += hexchars[(val >> nyb_shift) & 0xf];
		nyb_shift -= 4;
		*buf += hexchars[(val >> nyb_shift) & 0xf];
	}

	//buf[8] = 0;			/* Useful to terminate as string */

}	/* reg2hex () */

void rsp::rsp_read_reg(string buf){
	unsigned int regnum;	
	string reply = "";

	if(sscanf(buf.c_str(), "p%x", &regnum) == -1){
		fprintf (stderr, "Warning: Failed to recognize RSP read register "
				"command: %s\n", buf);
		cout << "Warning: Failed to recognize RSP read register command:" << buf << endl; 
		put_str_packet ("E01");
		return;
	}
	if(regnum < USER_REG_CNT)
		reg2hex(gdb_emu->x[regnum], &reply);

	else if(regnum == 0x20)
		reg2hex(gdb_emu->PC, &reply);

	else if(regnum == 0x1041)
		reg2hex(gdb_emu->PC, &reply);
//mstatus
	else if(regnum == 0x341)
		reg2hex(gdb_emu->csr[mstatus], &reply);
//misa
	else if(regnum == 0x342)
		reg2hex(gdb_emu->csr[misa], &reply);
//mtvec
	else if(regnum == 0x345)
		reg2hex(gdb_emu->csr[mie], &reply);
//mtvec
	else if(regnum == 0x346)
		reg2hex(gdb_emu->csr[mtvec], &reply);
//mscratch
	else if(regnum == 0x381)
		reg2hex(gdb_emu->csr[mscratch], &reply);
//mepc
	else if(regnum == 0x382)
		reg2hex(gdb_emu->csr[mepc], &reply);
//mcause
	else if(regnum == 0x383)
		reg2hex(gdb_emu->csr[mcause], &reply);
//mip
	else if(regnum == 0x385)
		reg2hex(gdb_emu->csr[mip], &reply);
//satp
	else if(regnum == 0x1c1)
		reg2hex(gdb_emu->csr[satp], &reply);

//??
	else if(regnum == 0xf51)
		reg2hex(gdb_emu->PC, &reply);

	else{
		/* Error response if we don't know the register */
		fprintf (stderr, "Warning: Attempt to read unknown register 0x%x: "
				"ignored\n", regnum);
		put_str_packet ("E01");
      return;
	}
	put_packet(reply);
}

void rsp::rsp_read_mem (string buf){
	string reply = "";
	unsigned int    addr;			/* Where to read the memory */
	int             len;			/* Number of bytes to read */
	int             off;			/* Offset into the memory */

	if (2 != sscanf (buf.c_str(), "m%x,%x:", &addr, &len)){
		cout << "Warning: Failed to recognize RSP read memory. command: " << buf << endl;
		put_str_packet ("E01");
		return;
	}

	/* Make sure we won't overflow the buffer (2 chars per byte) */
	if ((len * 2) >= GDB_BUF_MAX){
		cout << "Warning: Memory read " << buf << "too large for RSP packet: truncated" << endl;
		len = (GDB_BUF_MAX - 1) / 2;
	}

	/* Refill the buffer with the reply */
	for (off = 0; off < len; off++){
		unsigned char  ch;		/* The byte at the address */

		/* Check memory area is valid */
		if (gdb_emu->V2P(addr + off, -1) == -1){
			/* The error number doesn't matter. The GDB client will substitute
			   its own */
			put_str_packet ("E01");
			return;
		}

		// Get the memory direct - no translation.
		ch = gdb_emu->get_mem8(addr + off);

		//buf->data[off * 2]     = hexchars[ch >>   4];
		//buf->data[off * 2 + 1] = hexchars[ch &  0xf];
		reply += hexchars[ch >> 4];
		reply += hexchars[ch & 0xf];
	}

	//buf->data[off * 2] = 0;			/* End of string */
	//buf->len           = strlen (buf->data);
	put_packet (reply);

}	/* rsp_read_mem () */

void rsp::rsp_insert_matchpoint (string buf){
	int                type;	/* To avoid old GCC limitations */
	unsigned long int  addr;		/* Address specified */
	int                len;		/* Matchpoint length (not used) */

	/* Break out the instruction. We have to use an intermediary for the type,
	   since older GCCs do not like taking the address of an enum
	   (dereferencing type-punned pointer). */
	if (3 != sscanf (buf.c_str(), "Z%1d,%lx,%1d", &type, &addr, &len)){
		fprintf (stderr, "Warning: RSP matchpoint insertion request not "
				"recognized: ignored\n");
		put_str_packet ("E01");
		return;
	}

	/* Sanity check that the length is 4 */
	if (4 != len){
		fprintf (stderr, "Warning: RSP matchpoint insertion length %d not "
				"valid: 4 assumed\n", len);
		len = 4;
	}

	/* Sort out the type of matchpoint */
	switch (type){
		case BP_MEMORY:
			put_str_packet ("OK");
			bp.push_back(addr);
			return;

		case BP_HARDWARE:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_WRITE:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_READ:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_ACCESS:
			put_str_packet ("");		/* Not supported */
			return;

		default:
			fprintf (stderr, "Warning: RSP matchpoint type %d not "
					"recognized: ignored\n", type);
			put_str_packet ("E01");
			return;

	}

}	/* rsp_insert_matchpoint () */

void rsp::rsp_remove_matchpoint (string buf){
	int                type;	/* To avoid old GCC limitations */
	unsigned long int  addr;		/* Address specified */
	int                len;		/* Matchpoint length (not used) */
	struct mp_entry   *mpe;		/* Info about the replaced instr */

	/* Break out the instruction. We have to use an intermediary for the type,
	   since older GCCs do not like taking the address of an enum
	   (dereferencing type-punned pointer). */
	if (3 != sscanf (buf.c_str(), "z%1d,%lx,%1d", &type, &addr, &len)){
		fprintf (stderr, "Warning: RSP matchpoint deletion request not "
				"recognized: ignored\n");
		put_str_packet ("E01");
		return;
	}

	/* Sanity check that the length is 4 */
	if (4 != len){
		fprintf (stderr, "Warning: RSP matchpoint deletion length %d not "
				"valid: 4 assumed\n", len);
		len = 4;
	}

	/* Sort out the type of matchpoint */
	switch (type){
		case BP_MEMORY:
			for(int x = 0; x < bp.size(); x++){
				if(bp[x] == addr){
					remove(bp,x);
					x--;
				}
			}
			put_str_packet ("OK");
			return;

		case BP_HARDWARE:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_WRITE:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_READ:
			put_str_packet ("");		/* Not supported */
			return;

		case WP_ACCESS:
			put_str_packet ("");		/* Not supported */
			return;

		default:
			fprintf (stderr, "Warning: RSP matchpoint type %d not "
					"recognized: ignored\n", type);
			put_str_packet ("E01");
			return;

	}
}	/* rsp_remove_matchpoint () */

void rsp::rsp_continue (string buf){
	unsigned long int  addr;		/* Address to continue from, if any */

	if (0 == strcmp ("c", buf.c_str())){
		addr = gdb_emu->PC;	/* Default uses current NPC */
	}
	else if (1 != sscanf (buf.c_str(), "c%lx", &addr)){
		cout << "Warning: RSP continue address " << buf << "not recognized: ignored" << endl;
		addr = gdb_emu->PC;	/* Default uses current NPC */
	}

	//rsp_continue_generic (addr, EXCEPT_NONE);
	stop = false;

}	/* rsp_continue () */

void rsp::rsp_step (string buf){
	unsigned long int  addr;		/* The address to step from, if any */

	if (0 == strcmp ("s", buf.c_str()))
	{
		addr = gdb_emu->PC;	/* Default uses current NPC */
	}
	else if (1 != sscanf (buf.c_str(), "s%lx", &addr))
	{
		cout << "Warning: RSP step address " << buf << "not recognized: ignored" << endl;
		addr = gdb_emu->PC;	/* Default uses current NPC */
	}

	//rsp_step_generic (addr, EXCEPT_NONE);
	stop = false;
	step = true;

}	/* rsp_step () */

int rsp::handle_interrupt_rsp(){
	static int instr_num = 0;
	instr_num++;
	if(instr_num != 1000)
		return -1;
	instr_num = 0;

	if (-1 == client_fd){
		fprintf (stderr, "Warning: Attempt to read from unopened RSP "
				"client: Ignored\n");
		return -1;
	}
	int ch;		
	FD_ZERO(&readfd);
	FD_SET(client_fd, &readfd);

	if((select(client_fd+1, &readfd, NULL, NULL, &tv)) <= 0){
		//fprintf(stderr,"\nTimeout\n");
		return -1;
	}

	if(FD_ISSET(client_fd, &readfd)){
		ch = get_rsp_char ();
		if(ch == 0x03){
			rsp_report_exception();
			return 0;
		}
	}
	return -1;
	
}
