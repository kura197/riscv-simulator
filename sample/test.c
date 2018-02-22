int main(void)
{
  /* 変数の宣言 */
  int n;
  int f0, f1, f2;
  int* addr = (int*)0xf0;

  f0 = 0;
  f1 = 1;

  /* フィボナッチ数の計算 */
  while(f1<100000000) {
    // フィボナッチ数の計算
    f2 = f1 + f0;
    // 変数の代入
    f0 = f1;
    f1 = f2;
  }

  *addr = f1;

  return 0;
}
