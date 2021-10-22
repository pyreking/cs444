extern void ioinit(void);
extern void main(void);

void init_kernel()
{
  ioinit();
  (void)main();
}