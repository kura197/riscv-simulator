struct trapframe {
  int ra;
  int sp;
  int gp;
  int tp;
  int t0;
  int t1;
  int t2;
  int s0;
  int s1;
  /*
  int a0;
  int a1;
  int a2;
  int a3;
  int a4;
  int a5;
  int a6;
  int a7;
  int s2;
  int s3;
  int s4;
  int s5;
  int s6;
  int s7;
  int s8;
  int s9;
  int s10;
  int s11;
  int t3;
  int t4;
  int t5;
  int t6;

  int mepc;
  int mcause;
  */
  int mstatus;
};

int sum(struct trapframe);

int main(void)
{
	struct trapframe tf;
	int ans;
	tf.ra = 16;
	tf.mstatus = 48;
	ans = sum(tf);
	return 0;
}

int sum(struct trapframe tf){
	int ans;
	ans = tf.mstatus ;
	return ans;
}



