//用于控制交互
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include<string.h>
int min(int x,int y){return x<y?x:y;}
int max(int x,int y){return x>y?x:y;}
char* vi(char* prech)//文本输入器
{
  int key=0,x=0,y=0;
  int limitx=16,limity=32;
  char ch[limitx+1][limity+1];
  char cl[limity+1];
  memset(ch,0,sizeof(ch));
  memset(cl,0,sizeof(cl));
  int mode=0;//0:view;1:insert
  initscr();
  crmode();/* code */
  keypad(stdscr, TRUE);
  noecho();
  while (key != ERR)
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
        x=limitx;
        y=0;
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
      }else if(key==263){
        y--;
        if(y<0){
          x--;
          y=strlen(ch[x]);
          memcpy(ch[x]+strlen(ch[x]),ch[x+1],min(strlen(ch[x+1]),limitx-strlen(ch[x])));
          for(int i=x+1;i<limitx;i++)
          {
            memset(ch[i],0,sizeof(ch[i]));
            memcpy(ch[i],ch[i+1],limity);
          }
        }else{
          int len=strlen(ch[x]);
          for(int i=y+1;i<len;i++)
          {
            ch[x][i-1]=ch[x][i];
          }
          ch[x][len-1]=0;
        }
        
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
      if(key>='a'&&key<='z')
      {
        if(y<limity/2)
        {
          y++;
          cl[y-1]=(char)key;
        }
      }else if(key==263)
      {
        if(y>0)
        {
          y--;
          cl[y]=0;
        }
      }else if(key==10)
      {
        if(strlen(cl)==2&&cl[0]=='w'&&cl[1]=='q')
        {
          free(prech);
          int len=0,now=0;
          for(int i=0;i<limitx;i++)
          {
            if(strlen(ch[i])>0)
            {
              len+=strlen(ch[i])+1;
            }
          }
          prech=(char*)malloc(len*sizeof(char));
          memset(prech,0,len*sizeof(char));
          for(int i=0;i<limitx;i++)
          {
            if(strlen(ch[i])>0)
            {
              memcpy(prech+now,ch[i],strlen(ch[i]));
              now+=strlen(ch[i])+1;
              prech[now-1]='\n';
            }
          }
          return prech;
        }else if(strlen(cl)==1&&cl[0]=='q')
        {
          return prech;
        }else
        {
          mode=0;
          memset(cl,0,sizeof(cl));
        }
          
      }
    }
    clear();
    for(int i=0;i<limitx;i++){
      move(i,0);
      printw("%s",ch[i]);
    }
    move(limitx,0);
    if(mode==0)
      printw("                  (%d,%d)%d %d",x,y,mode,key);
    if(mode==1)
      printw("--insert--        (%d,%d)%d %d",x,y,mode,key);
    else if(mode==2)
      printw(":%-16s(%d,%d)%d %d",cl,x,y,mode,key);
    printw("\n",x,y,mode,key);
    move(x,y);
    refresh();
    key = getch();
    
  }
  move(0,0);
  refresh();
  clear();
  endwin();
  exit(EXIT_SUCCESS);
}
int main()
{
  char* ch=NULL;
  ch=vi(ch);
  printf("(%d)%s\n",strlen(ch),ch);
  return 0;
}