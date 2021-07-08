// 程序名： myshell.h
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：程序执行需要的头文件，宏定义以及结构定义

//需要用到的头文件
#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<signal.h>
#include<pwd.h>
#include<time.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/shm.h>

//定义一些需要用到的常量
#define MAX_PATH_LENGTH 1024
#define MAX_INBUF_SIZE 1024
#define MAX_USRNAME_LENGTH 128
#define MAX_HOSTNAME_LENGTH 128
#define MAX_NAME_LENGTH 256
#define MAX_COMMAND_NUM 128
#define MAX_JOB_NUM 128
#define FG 0
#define BG 1
#define RUN 0
#define SUSPEND 1
#define DONE 2
#define YELLOW "\e[1;33m"
#define CYAN "\e[0;36m"
#define WHITE "\e[1;37m"
#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define CLEAR "\e[1;1H\e[2J"

//定义结构job，用来存储任务信息
typedef struct job
{
  pid_t pid;
  char jobname[MAX_NAME_LENGTH];
  int type;
  int status;
} job;
