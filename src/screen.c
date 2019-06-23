//用于控制交互
#include <curses.h>
#include <stdio.h>
char password[100];
int vim()
{
  char ch;
  int count = 0;
  initscr(); // 开始curses模式
  noecho();  // 当执行getch()函数的时候关闭键盘回显
  while ((ch = getch()) != '\n')
  {
    password[count++] = ch;
  }
  password[count] = '\0';
  endwin(); // 结束curses模式
  printf("%s\n", password);
  return 0;
}
int main()
{
  vim();
  return 0;
}