
int main(void)
{
	int x = 5;
	int y = 0;
	int bit = 0x80000000;
	int* addr = (int*)0xf0;
	int* va = (int*)0x00001000;
	*va = x;

	*addr = y;
	return 0;
}

