#ifndef SCREEN_H
#define SCREEN_H

//文本输入器界面
char* vi(char* prech);

//查看各种信息的界面
//s        查看超级块信息
//i <id>   查看索引节点信息
//b <id>   查看数据块信息
//d <id>   查看目录块信息
//q        退出
//h/?      查看帮助信息
void scfsviewer();

//初始化
int init();

//登录界面
int login();

//提供准确返回值，根据check_path决定是否检查路径存在性
int check_return_get_real_path(bool check_path);

//把buff内存储的本次读入路径和path内存储的当前路径合并获取命令需要的路径
//return -1:路径不合法
int get_real_path(bool check_path);

//终端命令界面
int terminal();

#endif