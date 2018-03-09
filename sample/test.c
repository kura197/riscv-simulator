
int main(){
	char *s, *ep;
	char str[] = "init";
	ep = str + 100;
	for(s = str; s < ep; s++){
		if(*s == 0)
			return (s - str);
	}
	return -1;
}
