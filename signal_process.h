// 程序名：signal_process.h
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：与信号处理有关函数的声明

//该函数用来初始化信号处理
void Init_signal();
//该函数用来响应捕获到的SIGCHLD信号
void SIGCHLD_handler(int sig_no, siginfo_t* info, void* vcontext);
//该函数用来响应捕获到的SIGTSTP信号
void SIGTSTP_handler(int sig_no);
