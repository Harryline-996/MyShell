// 程序名：signal_process.c
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：与信号处理有关函数的定义

#include "myshell.h"
#include "signal_process.h"

extern struct sigaction old_action;
extern struct sigaction new_action;
extern job* jobs;
extern int* job_count;
extern int job_shmID;

//该函数用来初始化信号处理
void Init_signal()
{
  //初始化new_action结构
  memset(&new_action, 0, sizeof(new_action));
  //设置new_action的信号处理函数为SIGCHLD_handler
  new_action.sa_sigaction = SIGCHLD_handler;
  //设置new_action的sa_flags
  new_action.sa_flags = SA_SIGINFO | SA_RESTART;
  //重置new_action的sa_mask
  sigemptyset(&new_action.sa_mask);
  //调用sigaction函数将接收到SIGCHLD信号后的处理方式设置为new_action
  sigaction(SIGCHLD, &new_action, &old_action);
  //调用signal()函数将接收到SIGTSTP和SIGSTOP信号后的处理函数设置为SIGTSTP_handler
  signal(SIGTSTP,SIGTSTP_handler);
  signal(SIGSTOP,SIGTSTP_handler);
}

//该函数用来响应捕获到的SIGCHLD信号
void SIGCHLD_handler(int sig_no, siginfo_t* info, void* vcontext)
{
  //定义pid保存返回SIGCHLD信号进程的pid
  pid_t pid = info->si_pid;
  int i;
  int count = *job_count;
  //在后台进程表中查找进程号为pid的记录
  for(i=0; i<count; i++){
    if(pid == jobs[i].pid)
      break;
  }
  
  //如果i<count，说明找到了对应的记录
  if(i < count){
    //如果jobs[i]是正在运行的后台进程，则将其状态调整为已完成
    if(jobs[i].type == BG && jobs[i].status == RUN)
      jobs[i].status = DONE;
    //如果jobs[i]是暂停中的后台进程，不做动作
    else if(jobs[i].type == BG && jobs[i].status == SUSPEND);
    //否则从后台进程表中删掉该记录
    else
      Del_job(pid);
  }  
}

//该函数用来响应捕获到的SIGTSTP信号
void SIGTSTP_handler(int sig_no)
{
  //printf("我收到了SIGTSTP信号!\n");
  //捕获到SIGTSTP信号即会输出^z，这时再输出换行符使之后的输出在下一行
  printf("\n");
  int i;
  int count = *job_count;
  //在后台进程表中查找类型为FG的记录(前台进程)
  for(i=count - 1; i>0; i--){
    if(jobs[i].type == FG)
      break;
  }
  
  //i>0说明找到了对应的记录
  if(i > 0){
    //将该进程挂起，即将其状态设置为SUSPEND，类型设置为BG(已经暂停的后台进程)
    jobs[i].status = SUSPEND;
    jobs[i].type = BG;
    //向该进程发送SIGSTOP信号来使其挂起
    kill(jobs[i].pid, SIGSTOP);
  }   

}
