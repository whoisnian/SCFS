//用于控制交互
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include<string.h>
int min(int x,int y){return x<y?x:y;}
int max(int x,int y){return x>y?x:y;}
int vi(char* prech)//文本输入器
{
  int key=0,x=0,y=0;
  int limitx=16,limity=32;
  char ch[limitx+1][limity+1];
  memset(ch,0,sizeof(ch));
  int mode=0;//0:view;1:insert
  initscr();
  crmode();
  keypad(stdscr, TRUE);
  noecho();
  while (key != ERR && key != 'q')
  {
    move(x, y);
    if(mode==0)
    {
      if(key==KEY_LEFT)
      {
        y=max(0,y-1);
      }else if(key==KEY_RIGHT)
      {
        y=min(strlen(ch[x]),y+1);
      }else if(key==KEY_UP)
      {
        x=max(0,x-1);
        y=min(y,strlen(ch[x]));
      }else if(key==KEY_DOWN)
      {
        if(strlen(ch[x+1])>0)
        {
          x=min(limitx,x+1);
          y=min(y,strlen(ch[x]));
        }
      }else if(key==':')
      {
        
        mode=2;
      }else if(key=='i')
      {
        mode=1;
      }
      
    }else if(mode==1)
    {
      if(key==KEY_LEFT)
      {
        y=max(0,y-1);
      }else if(key==KEY_RIGHT)
      {
        y=min(strlen(ch[x]),y+1);
      }else if(key==KEY_UP)
      {
        x=max(0,x-1);
        y=min(y,strlen(ch[x]));
      }else if(key==KEY_DOWN)
      {
        if(strlen(ch[x+1])>0)
        {
          x=min(limitx,x+1);
          y=min(y,strlen(ch[x]));
        }
      }else if(key==27)
      {
        mode=0;
      }else
      {
        if(key==10)
        {
          x++;
          for(int i=limitx-1;i>x;i--){
            memcpy(ch[i],ch[i-1],limity);
          }
          memset(ch[x],0,sizeof(ch[x]));
          memcpy(ch[x],ch[x-1]+y,limity-y);
          for(int i=y;i<limity;i++)ch[x-1][i]=0;
          y=0;
        }else if(y<limity)
        {
          for(int i=strlen(ch[x]);i>y;i--){
            ch[x][i]=ch[x][i-1];
          }
          ch[x][y]=(char)key; 
          y++;
        }
      }      
    }else if(mode==2)
    {
      move(limitx,0);
      printw(":");
      move(limitx,1);
      printf("%c",(char)key);//todo
    }
    clear();
    for(int i=0;i<limitx;i++){
      move(i,0);
      printw("%s",ch[i]);
    }
    move(limitx,limity-9);
    printw("(%d,%d)%d\n",x,y,mode);
    move(limitx,0);
    if(mode==1)
      printw("--insert--");
    move(x,y);
    refresh();
    key = getch();
  }
  endwin();
  exit(EXIT_SUCCESS);
}
int main()
{
  vi(NULL);
  return 0;
}