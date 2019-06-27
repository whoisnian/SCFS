//用于控制交互
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "definition.h"
#include "scfs.h"
#include "superblock.h"
#include "inode.h"
#include "block.h"
#include "screen.h"
#define max_username_size 128
#define max_password_size 128
#define max_path_size 256
char path[max_path_size],username[max_username_size],password[max_password_size];
char real_path[max_path_size];
char buf[SC_BLOCK_SIZE];//read buffer
char* vi(char* prech)//文本输入器
{
  int key=0,x=0,y=0;
  int limitx=16,limity=32;
  char ch[limitx+1][limity+1];
  char cl[limity+1];
  memset(ch,0,sizeof(ch));
  if(prech!=NULL){
    int i,ii,jj,len;
    len=strlen(prech);
    ii=0;jj=0;
    for(i=0;i<len;i++){
      ch[ii][jj]=prech[i];
      if(prech[i]=='\n'){
        ii++;
        jj=0;
      }else{
        jj++;
      }
    }
  }
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
          ch[x-1][y]='\n';
          for(int i=y+1;i<limity;i++)ch[x-1][i]=0;
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
          int maxx=limitx;
          //while(maxx&&strlen(ch[maxx-1])==0)maxx--;
          for(int i=0;i<maxx;i++)
          {
            if(strlen(ch[i])>0)
            {
              memcpy(prech+now,ch[i],strlen(ch[i]));
              now+=strlen(ch[i]);
              //prech[now-1]='\n';
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

void scfsviewer()//查看各种信息
{
  char command[100];
  int id, ok = 1;
  char data[SC_BLOCK_SIZE];
  dir_st dir[15];

  while(ok)
  {
      printf("\n\033[1;32;40m--->>\033[0m ");
      scanf("%s", command);
      if(!strcmp(command, "q"))
      {
          // q: 退出 quit
          ok = 0;
      }
      else if(!strcmp(command, "s"))
      {
          // s: 查看超级块 superblock
          printf("超级块信息：\n");
          superblock_st *superblock = read_superblock();
          debug_superblock(superblock);
          free(superblock);
      }
      else if(!strcmp(command, "i"))
      {
          // i: 查看索引节点 inode
          scanf("%d", &id);
          printf("索引节点信息：\n");
          inode_st *inode = read_inode(id);
          debug_inode(inode);
          free(inode);
      }
      else if(!strcmp(command, "b"))
      {
          // b: 查看数据块 block
          scanf("%d", &id);
          printf("数据块信息：\n");
          read_block(id, data, SC_BLOCK_SIZE);
          for(int i = 0;i < SC_BLOCK_SIZE/16;i++)
          {
              printf("%04x: ", i*16);
              for(int j = 0;j < 8;j++)
                  printf("%02x%02x ", data[i*16+j*2]&255, data[i*16+j*2+1]&255);
              printf("  ");
              for(int j = 0;j < 8;j++)
                  printf("%c%c ", data[i*16+j*2]>=32?data[i*16+j*2]:46, data[i*16+j*2+1]>=32?data[i*16+j*2+1]:46);
              printf("\n");
          }
      }
      else if(!strcmp(command, "d"))
      {
          // d: 查看目录所在数据块 dir
          scanf("%d", &id);
          printf("目录块信息：\n");
          read_block(id, dir, sizeof(dir_st)*15);
          for(int i = 0;i < 15;i++)
              printf("%2u %s\n", dir[i].inodeid, dir[i].filename);
      }
      else if((!strcmp(command, "h"))||(!strcmp(command, "?")))
      {
          // h/?: 帮助 help
          printf(" s        查看超级块信息\n");
          printf(" i <id>   查看索引节点信息\n");
          printf(" b <id>   查看数据块信息\n");
          printf(" d <id>   查看目录块信息\n");
          printf(" q        退出\n");
          printf(" h/?      查看帮助信息\n");
      }
    }
}

int init(){
  memset(buf,0,sizeof(buf));
  memset(path,0,sizeof(path));
  memset(username,0,sizeof(username));
  memset(password,0,sizeof(password));
  system("reset");
  return 0;
}

int login()
{
  int max_fail_time=3;//max time of failed login attempt
  int fail_time=0;
  do
  {
    memset(buf,0,sizeof(buf));
    printf("username : ");scanf("%s",buf);
    if(strlen(buf)>max_username_size){//avoid over memory error
      printf("username overlength\n");
      return -1;
    }else{
      memcpy(username,buf,max_username_size);
    }
    printf("password : ");scanf("%s",buf);
    if(strlen(buf)>max_username_size){
      printf("password overlength\n");
      return -1;
    }else{
      memcpy(password,buf,max_password_size);
    }
    if(command_login(username,password)==0){//modify later to use relevant interface
      system("reset");
      break;
    }else
    {
      fail_time++;
      if(fail_time>=max_fail_time){
        printf("too much failed login attempts\n");
        return -1;
      }
      system("reset");
      printf("permission failed, please try again\n");
    }
  }while(1);
  return 0;
}

//check return value and print warning for get_real_path 
int check_return_get_real_path(bool check_path){
  if(!check_path){
    return 0;
  }else{  
    if(sc_access(real_path,SC_F_OK)==0){
      return 0;
    }else{
      printf("invalid path or path not exist!\n");
      return -1;
    }
  }
  return -1;
}

//transform string in buff into real path and save inti real_path
//return -1:path not valid
int get_real_path(bool check_path){
  memset(real_path,0,sizeof(real_path));
  if(buf[0]=='/'){
    if(strlen(buf)>max_path_size){
      printf("path argument is too long\n");
      return -1;
    }
    memcpy(real_path,buf,max_path_size);
    if(check_path)
    return check_return_get_real_path(check_path);
  }else if(buf[0]=='.'&&strlen(buf)>=2&&buf[1]=='.'){
    memcpy(real_path,path,max_path_size);
    int i=strlen(real_path);
    real_path[i-1]=0;
    i--;
    while(i&&real_path[i-1]!='/'){
      real_path[i-1]=0;
      i--;
    }
    if(i)real_path[i-1]=0;
    memcpy(real_path+strlen(real_path),buf+2,strlen(buf)-2);
    if(strlen(real_path)==0)real_path[0]='/';
    return check_return_get_real_path(check_path);
  }else if(buf[0]=='.'&&strlen(buf)>=2&&buf[1]=='/'){
    if(strlen(buf)+strlen(path)-2>max_path_size){
      printf("path argument is too long\n");
      return -1;
    }
    memcpy(real_path,path,strlen(path));
    memcpy(real_path+strlen(path),buf+2,strlen(buf)-2);
    return check_return_get_real_path(check_path);
  }else{
    if(strlen(buf)+strlen(path)>max_path_size){
      printf("path argument is too long\n");
      return -1;
    }
    memcpy(real_path,path,strlen(path));
    memcpy(real_path+strlen(path),buf,strlen(buf));
    return check_return_get_real_path(check_path);
  }
  printf("invalid path or path not exist!\n");
  return -1;
}

int terminal()
{
  int ret;
  path[0]='/';
  do
  {
    if(path[strlen(path)-1]!='/'){
      path[strlen(path)]='/';
    }
    printf("\033[1;32;40m%s@cool_SCFS\033[0m:\033[1;34;40m~%s\033[0m$ ",username,path+1);
    scanf("%s",buf);
    if(strlen(buf)>max_path_size){
      printf("too long argument\n");
      continue;
    }
    if(strcmp(buf,"clear")==0||strcmp(buf,"reset")==0){
      system(buf);
    }else if(strcmp(buf,"shutdown")==0||strcmp(buf,"exit")==0){
      return 0;
    }else if(strcmp(buf,"ls")==0){//ok
      char* ls_buf=malloc(sizeof(char)*SC_BLOCK_SIZE);
      memset(ls_buf,0,sizeof(char)*SC_BLOCK_SIZE);
      ret=sc_readdir(path,ls_buf,sc_filler,0,NULL,0);
      if(ret!=0){
        printf("ls failed\n");
        continue;
      }
      int len=strlen(ls_buf),cnt=0,num=0;
      for(int i=1;i<len;i++){
        if(ls_buf[i]=='/'){
          for(int j=cnt;j<12;j++)
            printf(" ");
          num++;
          if(num%4==0)printf("\n");
          cnt=0;
        }else{
          if(cnt<8){
            printf("%c",ls_buf[i]);
            cnt++;
          }else{
            if(cnt==8){
              printf("..");
              cnt=10;
            }
          }
        }
        
      }
      printf("\n"); 
      free(ls_buf);
    }else if(strcmp(buf,"cd")==0){//ok
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret==0){//success!
        ret=sc_access(real_path,SC_X_OK);
        if(ret!=0){//failed;
          printf("permission denied\n");
          continue;
        }
        memset(path,0,sizeof(path));
        memcpy(path,real_path,max_path_size);
      }else{//failed
      }
    }else if(strcmp(buf,"mkdir")==0){//ok
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(false);
      if(ret!=0){
        continue;//failed
      }
      ret=sc_access(path,SC_W_OK);
      if(ret!=0){
        printf("write permission denied\n");
        continue;
      }
      ret=sc_mkdir(real_path,SC_DEFAULT_DIR);
      if(ret==0){//success!
      }else{
        printf("mkdir failed\n");
      }

    }else if(strcmp(buf,"cat")==0){//ok
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret!=0){//failed
        continue;
      }
      ret=sc_access(real_path,SC_R_OK);
      if(ret!=0){
        printf("read permission denied\n");
        continue;
      }
      char* cat_buf=(char*)malloc(sizeof(char)*SC_BLOCK_SIZE);
      memset(cat_buf,0,sizeof(char)*SC_BLOCK_SIZE);
      ret=sc_read(real_path,cat_buf,SC_BLOCK_SIZE,0,NULL);
      if(ret<0){//failed
        printf("cat failed\n");
        continue;
      }
      printf("%s\n",cat_buf);
      if(ret>1024)
        printf("\nfile too long, the rest part is hidden\n");
      free(cat_buf);
    }else if(strcmp(buf,"touch")==0){//ok
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(false);
      //printf("(%s)",real_path);//debug
      if(ret!=0){
        continue;//failed
      }
      ret=sc_access(path,SC_W_OK);
      if(ret!=0){
        printf("%s %d write permission denied\n", path, ret);
        continue;
      }
      ret=sc_create(real_path,SC_DEFAULT_FILE,NULL);
      if(ret==0){//success!
      }else{
        printf("touch failed\n");
      }
    }else if(strcmp(buf,"rm")==0){//ok, but may need further check
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret!=0){
        continue;//failed
      }
      ret=sc_access(real_path,SC_W_OK);
      if(ret!=0){
        printf("write permission denied\n");
        continue;
      }
      ret=sc_rmdir(real_path);
      if(ret==0){//success
      }else{
        printf("rm failed\n");
      }
    }else if(strcmp(buf,"cp")==0){//todo
      char* cp_buf=malloc(sizeof(char)*SC_BLOCK_SIZE);
      char* path_src=malloc(sizeof(char)*max_path_size);
      char* path_dest=malloc(sizeof(char)*max_path_size);
      memset(cp_buf,0,sizeof(char)*SC_BLOCK_SIZE);
      memset(path_src,0,sizeof(char)*max_path_size);
      memset(path_dest,0,sizeof(char)*max_path_size);
      int now;
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret!=0){
        continue;//failed
      }
      memcpy(path_src,real_path,strlen(real_path));
      ret=sc_access(path_src,SC_R_OK);
      if(ret!=0){
        printf("read permission denied\n");
        continue;
      }
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret!=0){
        continue;//failed
      }
      memcpy(path_dest,real_path,strlen(real_path));
      ret=sc_access(path_dest,SC_W_OK);
      if(ret!=0){
        printf("write permission denied\n");
        continue;
      }
      if(strcmp(path_src,path_dest)==0){
        sc_rename(path_src,path_dest,0);
      }else{
        now=0;
        ret=sc_create(path_src,SC_DEFAULT_FILE,NULL);
        if(ret==0){//success!
        }else{
          printf("touch failed\n");
        }
        while((ret=sc_read(path_src,cp_buf,SC_BLOCK_SIZE,now,NULL))){
          sc_write(path_dest,cp_buf,SC_BLOCK_SIZE,now,NULL);
          now+=SC_BLOCK_SIZE;
        }
      }
      free(path_dest);
      free(path_src);
      free(cp_buf);
    }else if(strcmp(buf,"vi")==0){//ok, with ignorable tiny problem
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=get_real_path(true);
      if(ret!=0){
        continue;//failed
      }
      ret=sc_access(real_path,SC_R_OK);
      if(ret!=0){
        printf("read permission denied\n");
        continue;
      }
      ret=sc_access(real_path,SC_W_OK);
      if(ret!=0){
        printf("write permission denied\n");
        continue;
      }
      char* vi_buf=(char*)malloc(SC_BLOCK_SIZE*sizeof(char));
      memset(vi_buf,0,SC_BLOCK_SIZE*sizeof(char));
      ret=sc_read(real_path,vi_buf,SC_BLOCK_SIZE,0,NULL);
      if(ret<0){//failed
        printf("read failed\n");
        continue;
      }
      //printf("%s\n",vi_buf);getchar();getchar();//debug
      vi_buf=vi(vi_buf);//have problem here
      system("reset");
      //printf("(%s)(%s)(%d)\n",vi_buf,real_path,strlen(vi_buf));//debug
      ret=sc_write(real_path,vi_buf,strlen(vi_buf),0,NULL);
      free(vi_buf);
      if(ret<0){//failed
        printf("write failed\n");
        continue;
      }
    }else if(strcmp(buf,"chmod")==0){//checking
      unsigned int new_mode;
      memset(buf,0,sizeof(buf));
      scanf("%o%s",&new_mode,buf);
      ret=get_real_path(true);
      if(ret!=0){//failed
        continue;
      }
      ret=sc_access(real_path,SC_W_OK);
      if(ret!=0){//failed
        printf("write permission denied\n");
        continue;
      }
      ret=sc_chmod(real_path,new_mode,NULL);
      if(ret!=0){//failed
        printf("chmod failed\n");
        continue;
      }
    }else if(strcmp(buf,"passwd")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      char* buf2=malloc(sizeof(char)*SC_BLOCK_SIZE);
      memset(buf2,0,sizeof(char)*SC_BLOCK_SIZE);
      scanf("%s",buf2);
      ret=command_passwd(buf,buf2);
      if(ret!=0){
        printf("passwd failed\n");
        continue;
      }
    }else if(strcmp(buf,"useradd")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=command_useradd(buf);
      if(ret!=0){
        printf("useradd failed\n");
        continue;
      }
    }else if(strcmp(buf,"userdel")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=command_userdel(buf);
      if(ret!=0){
        printf("userdel failed\n");
        continue;
      }
    }else if(strcmp(buf,"groupadd")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=command_groupadd(buf);
      if(ret!=0){
        printf("groupadd failed\n");
        continue;
      }
    }else if(strcmp(buf,"groupdel")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=command_groupdel(buf);
      if(ret!=0){
        printf("groupdel failed\n");
        continue;
      }
    }else if(strcmp(buf,"gpasswd")==0){
      memset(buf,0,sizeof(buf));
      scanf("%s",buf);
      ret=command_gpasswd(username,buf);
      if(ret!=0){
        printf("gpasswd failed\n");
        continue;
      }
    }else if(strcmp(buf,"scfsviewer")==0){
      scfsviewer();
      system("reset");
    }else{
      printf("%s: command not found\n",buf);
    }
  }while(1);
}

int main()
{
  int ret;
  if(access("test.img", F_OK) != 0)
  {
    ret = init_scfs("test.img");
    if(ret!=0)
    {
      printf("init scfs failed in function main,line__LINE__\n");
      return 0;
    }
  }
  ret = open_scfs("test.img");
  if(ret!=0){
    printf("open scfs failed in function main,line__LINE__\n");
    return 0;
  }
  init();
  if(login()!=0){
    printf("login failed, system exit");
    return 0;
  }
  terminal();
  ret=close_scfs();
  if(ret!=0){
    printf("close scfs failed in function main,line__LINE__\n");
    return 0;
  }
  return 0;
}
