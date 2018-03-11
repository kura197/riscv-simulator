#include<iostream>
#include<queue>
#include<stdio.h>
using namespace std;

class tmp{
	public:
	queue<int8_t> q;
};

int push(tmp* t, int8_t ch){
	printf("ch:%d, empty:%d\n",ch,t->q.empty());
	t->q.push(ch);
}

int main(){
	tmp t;
	int8_t ch;
	for(int i = 1; i < 4; i++){
		ch = i * 10; 
		push(&t, ch);
	}

	return 0;
}

