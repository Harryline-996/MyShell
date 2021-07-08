// 程序名： myshell.c
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：主程序，定义了一些全局变量，并包含main程序，是myshell程序的核心

#include "myshell.h"


//该函数用来初始化全局变量
void Init_global_variable();
//该函数用来显示命令提示符，包含当前路径，用户名和主机名
void Display_prompt();
//该函数用来读取用户输入的命令以及参数并存储到全局变量inbuf中
void Read_command(FILE* script_name);
//该函数用来分割读取的输入并存储到全局变量commands中
int Split_command();
//该函数用来选择并执行myshell的内建命令
int Exec_builtin_command(int count, int index);
//该函数用来根据不同的输入执行相应的内部命令或者创建子进程来执行外部命令
void Execute_command();



//定义全局变量inbuf用来保存用户输入
char inbuf[MAX_INBUF_SIZE];
//定义全局变量commands用来保存用户输入拆分后的结果
char* commands[MAX_COMMAND_NUM];
//定义全局变量command_count用来记录用户输入拆分后的数量
int command_count;
//定义全局变量left_count用来记录左边命令的长度
int left_count;
//定义全局变量right_count用来记录右边命令的长度
int right_count;
//定义全局变量Is_pipe用来指示命令中是否包含管道操作
int Is_pipe;
//定义全局变量Is_in_redirect用来指示命令中是否包含输入重定向
int Is_in_redirect;
//定义全局变量Is_out_redirect_cov用来指示命令中是否包含输出重定向(覆盖写)
int Is_out_redirect_cov;
//定义全局变量Is_out_redirect_app用来指示命令中是否包含输出重定向(追加写)
int Is_out_redirect_app;
//定义全局变量Is_bg用来指示命令是否为后台命令
int Is_bg;
//定义全局变量infile_path和outfile_path的分别用来保存输入重定向和输出重定向的文件名
char infile_path[MAX_PATH_LENGTH];
char outfile_path[MAX_PATH_LENGTH];

//定义后台进程表相关的全局变量
job* jobs;
int* job_count;
int job_shmID;

//定义用于信号处理的结构
struct sigaction old_action;
struct sigaction new_action;




//main主程序
int main(int argc, char **argv)
{
  //script_flag用来指示脚本文件是否已经被打开
  int script_flag = 0; 
  //定义文件指针fp用来指向要打开的脚本文件 
  FILE* fp;
  //初始化后台进程表
  Init_joblist();
  //初始化信号处理
  Init_signal();
  //将环境变量SHELL的值设置为myshell的路径
  setenv("SHELL","/home/lx/myshell",1);
  
  //不断循环执行
  while(1){
    //初始化全局变量
    Init_global_variable();
    //没有参数传入，则调用Display_prompt()函数显示命令提示符
    if(argc == 1)
      Display_prompt();
    //没有参数传入时将NULL作为Read_command()函数的参数，否则将fp作为Read_command()函数的参数
    if(argc == 1)
      Read_command(NULL);
    else{
      //script_flag为0，即脚本文件还未打开
      if(!script_flag){
        //设置script_flag为1，并打开脚本文件将指针保存到fp中
        script_flag = 1;
        //如果fp为NULL，说明打开失败，则输出错误信息并退出
        if(!(fp = fopen(argv[1], "r"))){
          fprintf(stderr, RED "[myshell] Error: Cannot open the script file named \"%s\"!\n", argv[1]);
          exit(1);
        }      
      }
      //script_flag为1，即脚本文件已经打开，调用Read_command(fp)函数读取命令
      else
        Read_command(fp);     
    }
      
    //调用Split_command()函数分割读取的输入并将函数返回值存到flag中
    int flag = Split_command();
    //如果flag的值为1，说明用户没有输入内容或者输入均为分隔符，直接continue
    if(flag == 1)
      continue;
    //如果flag的值为2，说明用户输入的参数个数过多，输出错误信息并continue
    else if(flag == 2){
      fprintf(stderr, RED "[myshell] Error: Parameters are too much!\n");
      continue;
    }
    //命令正常，则调用Execute_command()函数解析命令并相应执行
    else
    Execute_command();
  }
  return 0;
}

//该函数用来初始化全局变量
void Init_global_variable(){
  command_count = 0;
  left_count = 0;
  right_count = 0;
  Is_pipe = 0;
  Is_in_redirect = 0;
  Is_out_redirect_cov = 0;
  Is_out_redirect_app = 0;
  Is_bg = 0;
  strcpy(infile_path,"\0");
  strcpy(outfile_path,"\0");
}



//该函数用来显示命令提示符，包含当前路径，用户名和主机名
void Display_prompt()
{
  char path[MAX_PATH_LENGTH];
  char usrname[MAX_USRNAME_LENGTH];
  char hostname[MAX_HOSTNAME_LENGTH];
  //获得当前路径存储在path中
  getcwd(path,MAX_PATH_LENGTH);
  //声明结构变量pwd来保存含有用户名信息的passwd结构
  struct passwd *pwd;
  //getuid()函数获取用户id，getpwuid()函数根据用户id获取用户信息，保存到pwd中
  pwd = getpwuid(getuid());
  //将用户名信息保存到usrname中
  strcpy(usrname,pwd->pw_name);
  //获得当前主机名保存到hostname中
  gethostname(hostname,MAX_HOSTNAME_LENGTH);
  //控制颜色，输出命令提示符到屏幕
  printf(YELLOW "[myshell]%s@%s", usrname,hostname);
  printf(WHITE ":");
  printf(CYAN "%s", path);
  printf(WHITE "$ ");
}

//该函数用来读取用户输入的命令以及参数并存储到全局变量inbuf中
void Read_command(FILE* script_fp)
{
  //如果传入的参数script_fp为NULL,则从标准输入流读取
  if(!script_fp)
    //直接使用fgets()函数从标准输入流读取MAX_INBUF_SIZE-1个字符到inbuf中
    fgets(inbuf,MAX_INBUF_SIZE-1,stdin);
  //否则从参数所指的脚本文件中读取
  else{
    if(fgets(inbuf,MAX_INBUF_SIZE-1,script_fp) == NULL){
      fclose(script_fp);
      exit(0);
    }      
  }
}

//该函数用来分割读取的输入并存储到全局变量commands中
int Split_command()
{ 
  //初始化commands的每一项为NULL
  for(int i=0; i<MAX_COMMAND_NUM; i++)
    commands[i] = NULL;
  command_count = 0;
  
  //needcpy_infile和needcpy_outfile分别指示是否需要拷贝需要输入重定向和输出重定向文件的路径
  int needcpy_infile = 0;  
  int needcpy_outfile = 0;  
  //定义temp保存第一次分隔的结果
  char* temp;
  //定义分隔符为空格，换行符和制表符
  char* delim = " \n\t";
  //利用strtok()函数将inbuf以分隔符delim进行分割，第一次分割的结果存放在temp中
  temp = strtok(inbuf,delim);
  //如果temp为NULL，说明用户没有输入或者输入均为分隔符，返回1
  if(!temp)
    return 1;
  //temp不为NULL，则将temp赋值给commands[0]
  commands[0] = temp;
  command_count++;
 
  //在command_count不超过MAX_COMMAND_NUM，且继续分割的结果字符串不为NULL时，将command_count每次递增1
  while(command_count < MAX_COMMAND_NUM && (commands[command_count] = strtok(NULL,delim)) ){
    
    //如果Is_pipe为1，则之后的分隔均为右边的命令，command_count和right_count均递增
    if(Is_pipe){
      command_count++;
      right_count++;
      continue;
    }
    
    //如果needcpy_infile为1，则将当前分割结果复制到infile_path中并重新设置needcpy_infile为0
    if(needcpy_infile){
      strcpy(infile_path,commands[command_count]);
      needcpy_infile = 0;
      continue;
    }
    
    //如果needcpy_outfile为1，则将当前分割结果复制到outfile_path中并重新设置needcpy_outfile为0    
    if(needcpy_outfile){
      strcpy(outfile_path,commands[command_count]);
      needcpy_outfile = 0;
      continue;
    }   
    
    //如果当前分割结果为"<"，说明接下来将要读入输入重定向的文件，设置needcpy_infile和Is_in_redirect为1
    if(strcmp(commands[command_count],"<") == 0){
      needcpy_infile = 1;
      Is_in_redirect = 1;
      continue;
    }
    
    //如果当前分割结果为">"，说明接下来将要读入输出重定向(覆盖写)的文件，设置needcpy_outfile和Is_out_redirect_cov为1    
    if(strcmp(commands[command_count],">") == 0){
      needcpy_outfile = 1;
      Is_out_redirect_cov = 1;
      continue;
    }
    
    //如果当前分割结果为">>"，说明接下来将要读入输出重定向(追加写)的文件，设置needcpy_outfile和Is_out_redirect_app为1      
    if(strcmp(commands[command_count],">>") == 0){
      needcpy_outfile = 1;
      Is_out_redirect_app = 1;
      continue;
    }

    //如果当前分割结果为"|"，说明接下来将要读入管道右边的命令，设置Is_pipe为1，左边命令的长度为command_count         
    if(strcmp(commands[command_count],"|") == 0){
      Is_pipe = 1;
      left_count = command_count;
      continue;
    }
    
    //正常情况下command_count递增1
    command_count++;  
  }
    
  //如果command_count >= MAX_COMMAND_NUM则说明输入的参数过多，返回2
  if(command_count >= MAX_COMMAND_NUM)
    return 2;
    
  //如果输入命令以"&"结尾，则说明该命令需要后台执行，设置Is_bg为1 
  if(strcmp(commands[command_count - 1],"&") == 0){
    Is_bg = 1;
    //最后一位设置为NULL方便execvp()函数的调用
    commands[command_count - 1] = NULL;
    command_count--;
  }
    
  //正常结束函数则返回0
  return 0;
  
}

//该函数用来选择并执行myshell的内建命令
int Exec_builtin_command(int count, int index)
{
  //根据不同的命令选择相应的函数  
  if(strcmp(commands[index],"clr") == 0)
    Clr_func();
    
  else if(strcmp(commands[index],"environ") == 0)
    Environ_func();
    
  else if(strcmp(commands[index],"pwd") == 0)
    Pwd_func();
    
  else if(strcmp(commands[index],"exit") == 0)
    Exit_func();
    
  else if(strcmp(commands[index],"time") == 0)
    Time_func(); 
       
  else if(strcmp(commands[index],"umask") == 0)
    Umask_func(count,index); 
    
  else if(strcmp(commands[index],"cd") == 0)
    Cd_func(count,index); 
      
  else if(strcmp(commands[index],"dir") == 0)
    Dir_func(count,index); 
    
  else if(strcmp(commands[index],"echo") == 0)    
    Echo_func(count,index); 
    
  else if(strcmp(commands[index],"exec") == 0)   
    Exec_func(count,index); 
    
  else if(strcmp(commands[index],"set") == 0)     
    Set_func(count,index); 
    
  else if(strcmp(commands[index],"unset") == 0)    
    Unset_func(count,index); 
    
  else if(strcmp(commands[index],"test") == 0)  
    Test_func(count,index); 
    
  else if(strcmp(commands[index],"shift") == 0)   
    Shift_func(count,index); 
    
  else if(strcmp(commands[index],"jobs") == 0)     
    Jobs_func();
    
  else if(strcmp(commands[index],"fg") == 0)    
    Fg_func();
    
  else if(strcmp(commands[index],"bg") == 0)      
    Bg_func();
  
  else if(strcmp(commands[index],"help") == 0)
    Help_func();
  
  //与上面任何一个命令都不匹配，返回1
  else
    return 1;
  //否则返回0  
  return 0;
}

//该函数用来根据不同的输入执行相应的内部命令或者创建子进程来执行外部命令
void Execute_command()
{
  //分别备份stdin和stdout的文件描述符到stdinFd和stdoutFd中
  int stdinFd = dup(fileno(stdin));
  int stdoutFd = dup(fileno(stdout));
  
  //单独处理quit命令
  if(strcmp(commands[0],"quit") == 0){
      //用户输入了命令“quit”，则输出提示信息并退出myshell
      printf(CYAN "[myshell] Thanks for your using!\n");
      Free_joblist();
      exit(0);  
  }
  
  //单独处理help命令
  if(strcmp(commands[0],"help") == 0){
    //只有help而没有输出重定向时，将help命令替换成"help | more"即可
    if(!Is_out_redirect_cov && !Is_out_redirect_app){
      //设置管道操作标志为1
      Is_pipe = 1;
      //左边和右边命令长度都设置为1
      left_count = 1;
      right_count = 1;
      //右边命令名替换为more
      commands[1] = "more";    
    }
    //有输出重定向时不需要额外处理
    else;
  }
  
  //命令中包含了管道操作
  if(Is_pipe){
    //定义变量pipeFd用来保存管道的文件描述符
    int pipeFd[2];
    int status;
    //创建管道，将其文件描述符保存到pipeFd中，若失败输出错误信息并退出
    if(pipe(pipeFd) == -1){
      fprintf(stderr, RED "[myshell] Error: pipe() is failed!\n");
      exit(1);
    }
    
    //调用fork()函数创建子进程，返回值保存到pid1中
    pid_t pid1 = fork();
    
    //在pid1子进程中时
    if(pid1 == 0){
      //重定向标准输出到管道的读入端，向管道输出数据
      dup2(pipeFd[1], 1);
      //关闭管道的输出端
      close(pipeFd[0]);
      //调用Exec_builtin_command()函数执行命令，若该函数返回值为1，说明为外部命令，需要继续处理
      if(Exec_builtin_command(left_count,0) == 1){
      //定义left_commands来保存左边的命令
      char* left_commands[left_count + 1];
      for(int i=0; i<left_count; i++)
        left_commands[i] = commands[i]; 
      //最后一位赋值为NULL，便于调用execvp()函数
      left_commands[left_count] = NULL;
        //调用execvp()函数执行外部命令，如果执行失败则输出错误信息
        if(execvp(left_commands[0],left_commands)!=0)
          fprintf(stderr, RED "[myshell] Error: Can not find the command named \"%s\"!\n", commands[0]);      
      }
      //正常退出子进程
      exit(EXIT_SUCCESS);        
    }
    
    //在主进程中时
    else if(pid1 > 0){
      //fprintf(stderr,"Waiting pid1 = %d ...\n",pid1);
      //调用waitpid()函数阻塞主进程，等待pid1进程结束
      waitpid(pid1,&status,0);
      
      //调用fork()函数创建子进程，返回值保存到pid2中
      pid_t pid2 = fork();
      
      //如果pid2 == -1说明创建失败，输出错误信息
      if(pid2 == -1)
        fprintf(stderr, RED "[myshell] Error: fork() is failed!\n"); 
        
      //在pid2子进程中时
      else if(pid2 == 0){
        //关闭管道的输入端
        close(pipeFd[1]);
        //重定向标准输入到管道的输出端，从管道读取数据
        dup2(pipeFd[0], 0);
        //调用Exec_builtin_command()函数执行命令，若该函数返回值为1，说明为外部命令，需要继续处理        
        if(Exec_builtin_command(right_count,left_count) == 1){
          //定义right_commands来保存右边的命令
          char* right_commands[right_count + 1];
          for(int i=0; i<right_count; i++)
            right_commands[i] = commands[left_count + i]; 
          //最后一位赋值为NULL，便于调用execvp()函数  
          right_commands[right_count] = NULL;
          //调用execvp()函数执行外部命令，如果执行失败则输出错误信息
          if(execvp(right_commands[0],right_commands)!=0)
            fprintf(stderr, RED "[myshell] Error: Can not find the command named \"%s\"!\n", right_commands[0]);      
        }
        //正常退出子进程
        exit(EXIT_SUCCESS);       
      }
      
      //在主进程中时
      else{
        //关闭管道两端
        close(pipeFd[0]);
        close(pipeFd[1]);
        //fprintf(stderr,"Waiting pid2 = %d ...\n",pid2);
        //等待pid2子进程结束
        waitpid(pid2,&status,0);
        //fprintf(stderr,"pid2 = %d is done!\n",pid2);
      }  

    }
    
    //pid1 == -1，说明fork()创建失败，输出错误信息并退出
    else{
      fprintf(stderr, RED "[myshell] Error: fork() is failed!\n"); 
      exit(1);
    }

    return;
  }
  
  //命令中包含了输入重定向
  if(Is_in_redirect){
    int fileFd = 0;
    //以只读方式打开infile_path
    fileFd = open(infile_path, O_RDONLY, 0666);
    //将标准输入重定向到infile_path，如果失败则输出错误信息
    if(dup2(fileFd, fileno(stdin)) == -1)
      fprintf(stderr,RED "[myshell] Error: dup2() is failed!\n");
    //关闭文件
    close(fileFd);
  }
  
  //命令中包含了输出重定向(覆盖写)
  if(Is_out_redirect_cov){
    int fileFd = 0;
    //以读写，覆盖写方式打开outfile_path
    fileFd = open(outfile_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    //将标准输出重定向到outfile_path，如果失败则输出错误信息
    if(dup2(fileFd, fileno(stdout)) == -1)
      fprintf(stderr,RED "[myshell] Error: dup2() is failed!\n");
    //关闭文件  
    close(fileFd);
  }
  
  //命令中包含了输出重定向(追加写)
  if(Is_out_redirect_app){
    int fileFd = 0;
    //以读写，追加写方式打开outfile_path
    fileFd = open(outfile_path, O_RDWR | O_CREAT | O_APPEND, 0666);
    //将标准输出重定向到outfile_path，如果失败则输出错误信息
    if(dup2(fileFd, fileno(stdout)) == -1)
      fprintf(stderr,RED "[myshell] Error: dup2() is failed!\n");
    //关闭文件    
    close(fileFd);
  }
  
  //命令中不含管道操作，调用Exec_builtin_command()函数执行命令，若该函数返回值为1，说明为外部命令，需要继续处理
  if(Exec_builtin_command(command_count,0) == 1){
      
      //调用fork()函数创建子进程，返回值保存到pid中
      pid_t pid = fork();
      
      //pid == 0，说明在子进程中
      if(!pid){
        //进入子进程则添加环境变量PARENT来指示父进程
        Add_environ("PARENT=/home/lx/myshell");
        //调用execvp()执行外部命令，如果失败则输出错误信息       
        if(execvp(commands[0],commands)!=0)
          fprintf(stderr, RED "[myshell] Error: Can not find the command named \"%s\"!\n", commands[0]);
        //正常退出子进程
        exit(EXIT_SUCCESS);
      }
      
      //pid > 0，说明在主进程中
      else if(pid > 0){
        int status;
        //如果Is_bg为0，即该命令不为后台命令
        if(!Is_bg){
          //调用Add_job()函数将该进程相关信息加进后台进程表
          Add_job(pid, commands[0], FG, RUN);   
          //WUNTRACED选项下阻塞主进程，在进程暂停时即返回
          waitpid(pid,&status,WUNTRACED);         
        }
        
        //为后台命令时
        else{
          //调用Add_job()函数将该进程相关信息加进后台进程表
          Add_job(pid, commands[0], BG, RUN);
          //WNOHANG选项下不阻塞主进程
          waitpid(pid,&status,WNOHANG);
          int count = *job_count;
          int BG_count = 0;
          for(int i=0;i<count;i++)
            if(jobs[i].type == BG)
              BG_count++;
          //输出该条后台命令的序号和进程号
          printf("[%d] %d\n",BG_count,pid);             
        }

      }
      
      //pid < 0说明fork()创建进程失败，输出错误信息
      else
        fprintf(stderr, RED "[myshell] Error: fork() is failed!\n");     
  }
  
  //如果Is_in_redirect为1，说明之前进行了输入重定向，需要复原
  if(Is_in_redirect){
    //复原标准输入，失败则输出错误信息
    if(dup2(stdinFd, fileno(stdin)) == -1)
      fprintf(stderr,RED "[myshell] Error: dup2() is failed!\n");
    //关闭文件
    close(stdinFd);
  }  
  
  //如果Is_out_redirect_cov或Is_out_redirect_app为1，说明之前进行了输出重定向，需要复原
  if(Is_out_redirect_cov || Is_out_redirect_app){
   //复原标准输出，失败则输出错误信息
    if(dup2(stdoutFd, fileno(stdout)) == -1)
      fprintf(stderr,RED "[myshell] Error: dup2() is failed!\n");
    //关闭文件
    close(stdoutFd);
  }  

}


