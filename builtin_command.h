// 程序名： builtin_command.h
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：执行内建命令需要用到的函数的声明


//该函数是clr命令的具体实现，作用是清屏
void Clr_func();
//该函数是environ命令的具体实现，作用是列出所有环境变量
void Environ_func(); 
//该函数用来添加新的环境变量
void Add_environ(char* str);
//该函数是cd命令的具体实现，作用是改变当前目录
void Cd_func(int count, int index);
//该函数是pwd命令的具体实现，作用是显示当前目录
void Pwd_func();
//该函数是exit命令的具体实现，作用是正常退出当前进程
void Exit_func();
//该函数是time命令的具体实现，作用是显示当前时间
void Time_func();
//该函数是umask命令的具体实现，作用是显示当前掩码或修改掩码
void Umask_func(int count, int index);
//该函数是dir命令的具体实现，作用是列出目录内容
void Dir_func(int count, int index);
//该函数是echo命令的具体实现，作用是在屏幕上显示内容并换行
void Echo_func(int count, int index);
//该函数是exec命令的具体实现，作用是使用指定命令替换当前的shell
void Exec_func(int count, int index);
//该函数是set命令的具体实现，作用是设置环境变量的值，没有参数则列出所有环境变量
void Set_func(int count, int index);
//该函数是unset命令的具体实现，作用是删除环境变量
void Unset_func(int count, int index);
//该函数用于测试输入的参数是否全部由阿拉伯数字组成
int Isnum(int index);
//该函数是test命令的具体实现，作用是进行一些字符串、数字的比较
void Test_func(int count, int index);
//该函数是shift命令的具体实现，作用是从标准输入读入参数(以空格分隔)，左移后输出，左移的位数由shift命令后跟的参数决定，无参数则默认左移一位
void Shift_func(int count, int index);
//该函数是help命令的具体实现，作用是显示用户手册
void Help_func();
//该函数用来初始化后台进程表
void Init_joblist();
//该函数用来向后台进程表中添加进程
job* Add_job(pid_t pid, char* jobname, int type, int status);
//该函数用来删除后台进程表中的进程
void Del_job(pid_t pid);
//该函数用来删除储存后台进程表的共享内存段
void Free_joblist();
//该函数是jobs命令的具体实现，作用是显示所有后台进程的信息
void Jobs_func();
//该函数是fg命令的具体实现，作用是将后台进程转入前台执行
void Fg_func();
//该函数是bg命令的具体实现，作用是将挂起的进程转为后台执行
void Bg_func();
