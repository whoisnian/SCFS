/*************************************************************************
    > File Name: shel.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年05月31日 星期五 23时57分24秒
    > Compile: gcc shel.c -o shel -I../include
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "definition.h"

char input_cmd[SC_PATH_MAX*2];          // 输入的完整命令
char path_cmd[SC_PATH_MAX];             // 调用程序的完整路径
char argv_cmd[32][SC_PATH_MAX];         // 输入命令的完整参数

int exe_num;                            // 可执行文件数量
char exe_path[SC_PATH_MAX];             // 可执行文件所在文件夹（相当于环境变量PATH）
char exe[64][2][SC_PATH_MAX];           // 可执行文件名称与其完整路径

char cur_dir[SC_PATH_MAX];              // 当前工作目录

void init(void)
{
    memset(input_cmd, 0, sizeof(input_cmd));
    memset(path_cmd, 0, sizeof(path_cmd));
    
    exe_num = 0;
    strcpy(cur_dir, "/");
}

int load_exe(void)
{
    // 打开可执行文件所在文件夹
    DIR *dir_p = opendir(exe_path);
    if(dir_p == NULL)
    {
        return -1;
    }

    // 读取文件夹下所有文件
    struct dirent *dirent_p;
    for(dirent_p = readdir(dir_p);dirent_p != NULL;dirent_p = readdir(dir_p))
    {
        // 忽略隐藏文件
        if(dirent_p->d_name[0] == '.')
            continue;

        // 存储可执行文件名称与其完整路径
        strcpy(exe[exe_num][0], dirent_p->d_name);
        sprintf(exe[exe_num][1], "%s/%s", exe_path, dirent_p->d_name);
        exe_num++;
        if(exe_num >= 64) break;
    }
    closedir(dir_p);
    return 0;
}

int main(int argc, char *argv[])
{
    realpath("./bin", exe_path);
    for(int i = 1;i < argc;i++)
    {
        if((!strcmp(argv[i], "--path")||!strcmp(argv[i], "-p"))&&i+1<argc)
        {
            realpath(argv[i+1], exe_path);
            i++;
        }
    }

    init();
    if(load_exe() == -1)
    {
        printf("Can not load exe from '%s'.\n", exe_path);
        return -1;
    }

    int pid, wstatus;
    while(1)
    {
        // 命令提示符
        printf("%s $ ", cur_dir);

        // 读取命令
        fgets(input_cmd, SC_PATH_MAX*2-1, stdin);
        // 将命令末尾的\n替换为\0
        input_cmd[strlen(input_cmd)-1] = '\0';

        if(!strncmp(input_cmd, "exit", 4))
        {
            // exit: 内建命令
            return 0;
        }
        else if(!strncmp(input_cmd, "cd", 2))
        {
            // cd: 内建命令
            strcpy(cur_dir, input_cmd+3);
        }
        else if(!strncmp(input_cmd, "help", 4))
        {
            // help: 内建命令
            printf("exit (built-in)\n");
            printf("cd (built-in)\n");
            printf("help (built-in)\n");
            for(int i = 0;i < exe_num;i++)
                printf("%s\n", exe[i][0]);
        }
        else
        {
            // 分隔命令参数
            int i = 0;
            argv[0] = strtok(input_cmd, " ");
            for(i = 1;(argv[i] = strtok(NULL, " ")) != NULL;i++);
            argv[i] = NULL;

            // 查找命令完整路径
            path_cmd[0] = '\0';
            for(i = 0;i < exe_num;i++)
            {
                if(!strcmp(argv[0], exe[i][0]))
                {
                    strcpy(path_cmd, exe[i][1]);
                    break;
                }
            }
            if(path_cmd[0] == '\0')
            {
                printf("Command not found.\n");
                continue;
            }

            // fork()创建子进程执行命令，主进程阻塞
            pid = fork();
            if(pid == -1)
            {
                printf("Can not create child process.\n");
                break;
            }
            else if(pid == 0)
            {
                execv(path_cmd, argv);
                exit(0);
            }
            else
            {
                wait(&wstatus);
            }
        }
    }
    return 0;
}
