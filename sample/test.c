
struct b{
	int t;
	int x;
};

struct a{
	int a,b,c;
	struct b* y;
};

int main(){
	struct a* z;
	z->y->x = 16;
	return 0;
}
