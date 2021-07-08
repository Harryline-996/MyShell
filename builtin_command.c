// 程序名： builtin_command.c
// 作者： 李向   学号: 3180106148
// 最后修改日期： 2020/08/19
// 内容：执行内建命令需要用到的函数的定义

#include "myshell.h"
#include "builtin_command.h"

extern char* commands[];
extern int command_count;
extern job* jobs;
extern int* job_count;
extern int job_shmID;

//该函数是environ命令的具体实现，作用是列出所有环境变量
void Environ_func()
{
  //直接声明外部变量environ，其中存储了所有的环境变量
  extern char ** environ;
  //遍历environ并打印即可
  for(int i=0;environ[i]!=NULL;i++)
    printf("%s\n",environ[i]);  
}

//该函数用来添加新的环境变量
void Add_environ(char* str)
{
  //直接调用putenv()函数即可添加新的环境变量
  putenv(str);    
}

//该函数是cd命令的具体实现，作用是改变当前目录
void Cd_func(int count, int index)
{
  char old_dir[MAX_PATH_LENGTH];
  //通过getcwd()函数获取当前目录并存储到old_dir中
  getcwd(old_dir,MAX_PATH_LENGTH);
  //通过chdir()函数实现目录切换
  //用户只输入了cd命令，没有输入目录参数，则显示当前目录
  if(count == 1){
    //输出old_dir的内容到屏幕
    printf("%s\n",old_dir);
  }
  //用户输入参数过多，输出错误信息
  else if(count > 2)
    fprintf(stderr, RED "[myshell] Error: Too many parameters!\n");
  //用户正好输入一个参数
  else{
    //如果切换到该参数所指的目录成功，则继续修改PWD环境变量为新的当前目录
    if(!chdir(commands[index+1]))
      setenv("PWD",commands[index+1],1);
    //如果切换到该参数所指的目录失败，说明找不到该目录，输出错误信息
    else
      fprintf(stderr, RED "[myshell] Error: Cannot find directory named %s\n",commands[index+1]); 
  }
    
}

//该函数是clr命令的具体实现，作用是清屏
void Clr_func()
{
  //直接利用printf进行清屏控制(CLEAR为宏定义，见文件头)
  printf(CLEAR);
}

//该函数是pwd命令的具体实现，作用是显示当前目录
void Pwd_func()
{
  char now_dir[MAX_PATH_LENGTH];
  //通过getcwd()函数获取当前目录并存储到now_dir中
  getcwd(now_dir,MAX_PATH_LENGTH);
  //输出now_dir的内容到屏幕
  printf("%s\n",now_dir);
}

//该函数是exit命令的具体实现，作用是正常退出当前进程
void Exit_func()
{
  //直接调用stdlib.h库中的exit()函数即可
  exit(0);
}

//该函数是time命令的具体实现，作用是显示当前时间
void Time_func()
{
  //定义类型为time_t的变量nowtime
  time_t nowtime;
  //定义指向tm结构的指针t
  struct tm *t;
  //定义day字符指针数组，用来存放星期的信息
  char* day[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  //time()函数返回从公元1979年1月1日的UTC时间从0时0分0秒起到现在经过的秒数
  time(&nowtime);
  //locattime()函数将参数所指的time_t结构中的信息转换成真实世界所使用的时间日期表示方法，然后将此结果由结构tm返回
  t = localtime(&nowtime);
  //输出当前时间，格式为：年/月/日 星期X 时:分:秒
  printf("%d/%d/%d %s %02d:%02d:%02d\n",(1900+t->tm_year),(1+t->tm_mon),t->tm_mday,day[t->tm_wday],t->tm_hour,t->tm_min,t->tm_sec);
}

//该函数是umask命令的具体实现，作用是显示当前掩码或修改掩码
void Umask_func(int count, int index)
{ 
  //如果没有输入参数，则显示当前掩码    
  if(count == 1){
    //定义pre_mask保存当前掩码
    mode_t pre_mask;
    //umask()函数返回当前掩码，保存到pre_mask中
    pre_mask = umask(0);
    //重新设置掩码为pre_mask
    umask(pre_mask);
    //输出当前掩码
    printf("%04d\n",pre_mask);
  }
  
  //如果输入参数大于1个，输出错误信息
  else if(count > 2)
    fprintf(stderr, RED "[myshell] Error: umask only need 1 or 0 parameter!\n");
  
  //如果输入的参数长度大于4，输出错误信息
  else if(strlen(commands[index + 1]) > 4)
    fprintf(stderr, RED "[myshell] Error: parameter should only be 1 to 4 bit octal number!\n");
  
  //输入参数正常，继续处理
  else{
    //flag标识输入的新掩码是否合法
    int flag=0;
    //检查参数的每一位是否有8或9，如果有则flag置为1且退出循环
    for(int i = 0;i < strlen(commands[index + 1]);i++){
      if(commands[index + 1][i]=='8' || commands[index + 1][i]=='9'){
        flag = 1;
        break;
      }
    }
    
    //flag为0说明输入的新掩码合法，则调用umask()函数修改掩码
    if(flag == 0){
      unsigned int new_mask = atoi(commands[index + 1]) % 1000;
      umask(new_mask);
    }
    
    //否则说明输入的新掩码不合法，输出错误信息
    else
      fprintf(stderr, RED "[myshell] Error: the parameter should be octal number!\n");
  }
}

//该函数是dir命令的具体实现，作用是列出目录内容
void Dir_func(int count, int index)
{
  //定义DIR类型指针dir
  DIR* dir;
  //定义dirent结构指针ptr
  struct dirent * ptr;
  //定义path字符数组用来保存目录路径
  char path[MAX_PATH_LENGTH];
  
  //用户只输入了dir命令，没有输入目录参数，或只输入一个目录
  if(count <= 2){
    //没有输入目录参数，则显示当前目录的内容
    if(count == 1)
      //通过getcwd()函数获取当前目录路径并存储到path中
      getcwd(path,MAX_PATH_LENGTH);
    //只输入一个目录，则将其保存到path中
    else
      strcpy(path,commands[index + 1]);
    //通过opendir()函数打开path所指的目录，返回值保存到dir中
    dir = opendir(path);
    //将readdir(dir)的返回值赋值给ptr，并在其不为NULL的情况下循环读取
    while((ptr = readdir(dir)) != NULL){
      //除了.和..，将path目录下的其他内容输出
      if(strcmp(ptr->d_name,".") && strcmp(ptr->d_name,".."))
        printf("%s  ",ptr->d_name);
  }
    printf("\n");
    //关闭dir
    closedir(dir);    
  }
  
  //用户在dir后还输入了多个参数，则循环进行处理，方式与上面相同
  else{
    for(int i = 1;i < count;i++){
        strcpy(path,commands[index + i]);
        dir = opendir(path);
        //先输出当前读取的目录路径
        printf("%s:\n",path);
        while((ptr = readdir(dir)) != NULL){
          if(strcmp(ptr->d_name,".") && strcmp(ptr->d_name,".."))
            printf("%s  ",ptr->d_name);
      }
        printf("\n");
        closedir(dir);
    }
  } 
}

//该函数是echo命令的具体实现，作用是在屏幕上显示内容并换行
void Echo_func(int count, int index)
{
  //直接循环输出保存的参数，最后打印一个换行符即可
  for(int i = 1;i < count;i++)
    printf("%s ",commands[index + i]);
  printf("\n");
}

//该函数是exec命令的具体实现，作用是使用指定命令替换当前的shell
void Exec_func(int count, int index)
{
  if(count == 1)
    fprintf(stderr, RED "[myshell] Error: Exec need parameters!\n"); 
  else{
    //取commands[index + 1]的地址传给commands_for_exec，因为commands[index]一定为exec，所以需要后移一位
    char ** commands_for_exec = &(commands[index + 1]);
    //不创建子进程，直接调用execvp()函数执行命令替换当前shell
    //如果execvp()返回值不为0，说明没有找到命令，输出错误信息
    if(execvp(commands[index + 1],commands_for_exec)!=0)
      fprintf(stderr, RED "[myshell] Error: Can not find the command!\n"); 
  }
}

//该函数是set命令的具体实现，作用是设置环境变量的值，没有参数则列出所有环境变量
void Set_func(int count, int index)
{
  //没有输入参数，则执行Environ_func()函数列出所有环境变量的值
  if(count == 1)
    Environ_func();
  //输入了两个参数，第一个为变量名，第二个为变量值
  else if(count == 3){
    char* env = getenv(commands[index + 1]);
    //如果没有找到输入的环境变量，则输出错误信息
    if(!env)
      fprintf(stderr, RED "[myshell] Error: Can not find the environment variable!\n"); 
    //找到了则设置该环境变量的值  
    else
      setenv(commands[index + 1],commands[index + 2],1);
  }
  //参数数量不对，则输出错误信息
  else
    fprintf(stderr, RED "[myshell] Error: Please check your parameters!\n"); 
}

//该函数是unset命令的具体实现，作用是删除环境变量
void Unset_func(int count, int index)
{
  //输入1个参数为将要取消的环境变量
  if(count == 2){
    char* env = getenv(commands[index + 1]);
    //如果没有找到输入的环境变量，则输出错误信息
    if(!env)
      fprintf(stderr, RED "[myshell] Error: Can not find the environment variable!\n"); 
    //找到了则设置该环境变量的值为空字符串 
    else
      setenv(commands[index + 1],"",1);
  }
  //参数数量不对，则输出错误信息
  else
    fprintf(stderr, RED "[myshell] Error: Please check your parameters!\n"); 
}

//该函数用于测试输入的参数是否全部由阿拉伯数字组成
int Isnum(int index)
{
  //循环查看commands[index + 1]的每一位
  for(int i=0;i<strlen(commands[index + 1]);i++)
    //如果从第二位起有非数字出现，或者第一位不为数字或者负号，则返回1
    if((!isdigit(commands[index + 1][i]) && i != 0) || (i == 0 && commands[index + 1][i]  != '-' && !isdigit(commands[index + 1][i]))){
      return 1;
    }
    
  //循环查看commands[index + 3]的每一位
  for(int i=0;i<strlen(commands[index + 3]);i++)
    //如果从第二位起有非数字出现，或者第一位不为数字或者负号，则返回1  
    if((!isdigit(commands[index + 3][i]) && i != 0) || (i == 0 && commands[index + 3][i]  != '-' && !isdigit(commands[index + 3][i]))){
      return 1;
    }
  //返回0说明参数正确  
  return 0;       
}

//该函数是test命令的具体实现，作用是进行一些字符串、数字的比较
void Test_func(int count, int index)
{
  //定义变量flag标明输入的参数是否为数字，如果是则为0，不是则为1
  int flag = 0;
  //参数个数刚好为三个，根据选项进行相应的比较
  if(count == 4){
    //commands[index + 2]为“=”，判断两字符串是否相等
    if(strcmp(commands[index + 2],"=") == 0){
      if(strcmp(commands[index + 1],commands[index + 3]) == 0)
        printf("True\n");
      else
        printf("False\n");
    }
    //commands[2]为“!=”，判断两字符串是否不相等
    if(strcmp(commands[index + 2],"!=") == 0){
      if(strcmp(commands[index + 1],commands[index + 3]) == 0)
        printf("False\n");
      else
        printf("True\n");
    }
    //commands[2]为“-eq”，判断两数字是否相等
    if(strcmp(commands[index + 2],"-eq") == 0){
      //利用Isnum()函数判断要比较的参数是否完全由数字组成
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) == atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -eq, parameters should be number!\n");      
    }
    //commands[2]为“-ge”，判断第一个数字是否大于等于第二个数字
    if(strcmp(commands[index + 2],"-ge") == 0){
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) >= atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -gt, parameters should be number!\n"); 
    }
    //commands[2]为“-gt”，判断第一个数字是否大于第二个数字
    if(strcmp(commands[index + 2],"-gt") == 0){
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) > atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -gt, parameters should be number!\n"); 
    }
    //commands[2]为“-le”，判断第一个数字是否小于等于第二个数字        
    if(strcmp(commands[index + 2],"-le") == 0){
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) <= atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -le, parameters should be number!\n"); 
    }
    //commands[2]为“-lt”，判断第一个数字是否小于第二个数字
    if(strcmp(commands[index + 2],"-lt") == 0){
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) < atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -lt, parameters should be number!\n"); 
    }
    //commands[2]为“-ne”，判断两数字是否不相等
    if(strcmp(commands[index + 2],"-ne") == 0){
      flag = Isnum(index);
      if(!flag){
        if(atoi(commands[index + 1]) != atoi(commands[index + 3]))
          printf("True\n");
        else
          printf("False\n");
      }
      else
        fprintf(stderr, RED "[myshell] Error: When you use -ne, parameters should be number!\n"); 
    }           
  }
  //参数个数不为3，输出错误信息
  else
    fprintf(stderr, RED "[myshell] Error: The number of parameters should be exactly 3!\n");  
}

//该函数是shift命令的具体实现，作用是从标准输入读入参数(以空格分隔)，左移后输出，左移的位数由shift命令后跟的参数决定，无参数则默认左移一位
void Shift_func(int count, int index)
{
  //没有输入参数，则默认左移一位后输出
  if(count == 1){  
    //读取用户输入
    Read_command(NULL);
    //分割读取的输入并将函数返回值存到flag中
    int flag = Split_command();
    //如果flag的值为1，说明用户没有输入内容或者输入均为分隔符，直接continue
    if(flag == 1)
      ;    
    //如果flag的值为2，说明用户输入的参数个数过多，输出错误信息
    else if(flag == 2){
      fprintf(stderr, RED "[myshell] Error: Parameters are too much!\n");
    }
    //输入正常，则左移一位后依次输出
    else{
      for(int i=1;i<command_count;i++){
        printf("%s ",commands[i]);
      }
      printf("\n");
    }    
  }
  //输入1个参数
  else if(count == 2){
    int flag = 0;
    //循环查看commands[1]的每一位是否均为数字
    for(int i=0;i<strlen(commands[index + 1]);i++){
      if(!isdigit(commands[index + 1][i])){
        flag = 1;
        break;
      }
    }
    if(flag)
      fprintf(stderr, RED "[myshell] Error: Parameter should be number!\n");
    else{
      int shift_num = atoi(commands[index + 1]);
      //读取用户输入
      Read_command(NULL);
      //分割读取的输入并将函数返回值存到flag中
      int flag = Split_command();
      //如果flag的值为1，说明用户没有输入内容或者输入均为分隔符，直接continue
      if(flag == 1)
        ;          
      //如果flag的值为2，说明用户输入的参数个数过多，输出错误信息
      else if(flag == 2){
        fprintf(stderr, RED "[myshell] Error: Parameters are too much!\n");
      }
      //输入正常，则左移一位后依次输出
      else{
        for(int i=shift_num;i<command_count;i++){
          printf("%s ",commands[i]);
        }
        printf("\n");
      } 
    }       
  }
  //输入1个以上参数，输出错误信息
  else
    fprintf(stderr, RED "[myshell] Error: The number of parameters should be exactly 0 or 1!\n");
}

//该函数是help命令的具体实现，作用是显示用户手册
void Help_func()
{
  printf("欢迎查看myshell的用户手册！\n");
  printf("\n");
 printf("*******************************************************************************\n");
  printf("Chapter 1: 内建命令的使用\n\n");
  printf("1. bg\n");
  printf("命令作用：  将最近一个挂起的进程转为后台执行\n");
  printf("使用示例：  bg\n");
  printf("参数个数：  无参数\n");
  printf("\n");
  
  printf("2. cd\n");
  printf("命令作用：  无参数则显示当前目录，有参数则改变当前目录为参数内容\n");
  printf("使用示例：  cd\n");
  printf("使用示例：  cd /home\n");
  printf("参数个数：  无参数或1个参数\n");
  printf("\n");  

  printf("3. clr\n");
  printf("命令作用：  清空当前屏幕内容\n");
  printf("使用示例：  clr\n");
  printf("参数个数：  无参数\n");
  printf("\n");   

  printf("4. dir\n");
  printf("命令作用：  无参数则显示当前目录下的内容，有参数则显示参数所指目录下的内容\n");
  printf("使用示例：  dir\n");
  printf("使用示例：  dir /\n");
  printf("参数个数：  无参数或1个参数\n");
  printf("\n"); 
  
  printf("5. echo\n");
  printf("命令作用：  无参数则显示空内容，有参数则显示参数内容\n");
  printf("使用示例：  echo\n");
  printf("使用示例：  echo 1 22 oop\n");
  printf("参数个数：  无参数或任意多参数\n");
  printf("\n");    
  
  printf("6. exec\n");
  printf("命令作用：  使用参数代表的命令替换当前进程\n");
  printf("使用示例：  exec ls\n");
  printf("参数个数：  1个参数\n");
  printf("\n"); 

  printf("7. exit\n");
  printf("命令作用：  退出当前进程\n");
  printf("使用示例：  exit\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 
  
  printf("8. environ\n");
  printf("命令作用：  显示所有的环境变量\n");
  printf("使用示例：  environ\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 

  printf("9. fg\n");
  printf("命令作用：  将最近的一个后台任务转到前台执行\n");
  printf("使用示例：  fg\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 
  
  printf("10. help\n");
  printf("命令作用：  显示用户手册\n");
  printf("使用示例：  help\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 

  printf("11. jobs\n");
  printf("命令作用：  显示所有的后台进程\n");
  printf("使用示例：  jobs\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 
  
  printf("12. pwd\n");
  printf("命令作用：  显示当前路径\n");
  printf("使用示例：  pwd\n");
  printf("参数个数：  无参数\n");
  printf("\n"); 

  printf("13. quit\n");
  printf("命令作用：  退出myshell\n");
  printf("使用示例：  quit\n");
  printf("参数个数：  无参数\n");
  printf("\n");
  
  printf("14. set\n");
  printf("命令作用：  无参数时，显示所有环境变量；有2个参数时，设置第1个参数代表的环境变量的值为第2个参数\n");
  printf("使用示例：  set\n");
  printf("使用示例：  set USER Wang\n");
  printf("参数个数：  无参数或2个参数\n");
  printf("\n");
  
  printf("15. shift\n");
  printf("命令作用：  从标准输入读入参数(以空格分隔)，左移后输出，左移的位数由shift命令后跟的参数决定，无参数则默认左移一位，有1个参数则左移参数代表的位数\n");
  printf("使用示例：  shift\n");
  printf("使用示例：  shift 2\n");
  printf("参数个数：  无参数或1个参数\n");
  printf("\n");

  printf("16. test\n");
  printf("命令作用：  可以进行一些字符串、数字的比较，包括两字符串是否相等，两数字之间的大小关系是否成立(相等，不相等，大于，小于，大于等于，小于等于)\n");
  printf("使用示例：  test abc = abc\n");
  printf("使用示例：  test abc != abc\n");
  printf("使用示例：  test 2 -eq 2\n");
  printf("使用示例：  test 2 -ne 2\n");
  printf("使用示例：  test 2 -gt 2\n");
  printf("使用示例：  test 2 -ge 2\n");
  printf("使用示例：  test 2 -lt 2\n");
  printf("使用示例：  test 2 -le 2\n");
  printf("参数个数：  3个参数\n");
  printf("\n");
  
  printf("17. time\n");
  printf("命令作用：  显示当前时间\n");
  printf("使用示例：  time\n");
  printf("参数个数：  无参数\n");
  printf("\n");
  
  printf("18. umask\n");
  printf("命令作用：  无参数时，显示当前掩码；有1个参数时，将当前掩码修改为参数的值\n");
  printf("使用示例：  umask\n");
  printf("使用示例：  umask 0222\n");
  printf("参数个数：  无参数或1个参数\n");
  printf("\n");

  printf("19. unset\n");
  printf("命令作用：  将参数所指的环境变量的值取消\n");
  printf("使用示例：  unset USER\n");
  printf("参数个数：  1个参数\n");
  printf("\n");  
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 2: 外部命令的执行\n\n");
  printf("简单描述： 除了内建命令之外，myshell还能够自动查找并执行外部命令\n\n");
  printf("实现原理： 其他的命令行输入被解释为程序调用，myshell通过fork()创建子进程，然后在子进程中调用execvp()函数来查找并执行这个程序，如果没有找到则会输出相应的错误提示信息\n\n");
  printf("使用示例： ls -l\n");
  printf("使用示例： gedit test.txt\n");
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 3: 脚本文件的执行\n\n");
  printf("简单描述： myshell能够从脚本文件中提取命令行输入，在调用myshell时，如果不加参数则进入命令行输入模式，如果加上一个脚本文件的参数，则会从参数代表的文件中提取命令并执行\n\n");
  printf("实现原理： 在检查到命令行参数时，myshell将打开参数代表的文件，之后用Read_command()函数读取命令时，从文件流中读取内容到buf，而不是从标准输入流中读取，如果打开文件失败则输出相应的错误提示信息\n\n");
  printf("使用示例： myshell test.sh\n");
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 4: I/O重定向\n\n");
  printf("简单描述： myshell能够支持I/O重定向，在输入要执行的命令后，输入‘<’，再接输入重定向到的文件inputfile，myshell在执行命令时就会从inputfile中读取而非从标准输入中读取；输入‘>’或者‘>>’再接输出重定向到的文件outputfile，myshell就会将命令执行的结果输出到outputfile中而非输出到屏幕上，其中‘>’表示覆盖写，‘>>’表示追加写\n\n");
  printf("实现原理： 在命令读取的过程中检查到‘<’，‘>’或者‘>>’时，将预先定义的表示是否有重定向的标志置1。在标志为1的情况下，将重定向的目标文件名保存到infile_path或outfile_path中，之后用open()函数用相应的方式打开文件(只读、覆盖写、追加写)，再用dup2()函数用打开的文件流替换相应的标准输出或输入流，即完成了重定向操作，最后还需要将标准输入输出流复原\n\n");
  printf("使用示例： wc < test1.txt >> test2.txt\n");
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 5: 后台程序执行\n\n");
  printf("简单描述： myshell能够支持后台程序执行，在输入命令后空格并输入字符‘&’，即可使得该条命令在后台执行而不阻塞主进程\n\n");
  printf("实现原理： 在命令读取的过程中检查到‘&’时，将预先定义的表示是否为后台执行的标志置1。在标志为1的情况下，利用fork()函数创建子进程来执行命令，但在主进程中，使用waitpid()函数的WNOHANG选项，不阻塞主进程，这样就实现了命令在后台执行，主进程仍然可以显示命令提示符，进行其他操作\n\n");
  printf("使用示例： sleep 5 &\n");
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 6: 管道操作\n\n");
  printf("简单描述： myshell能够支持管道操作，在符号‘|’左边命令的输出将成为右边命令的输入\n\n");
  printf("实现原理： 在命令读取的过程中检查到‘|’时，将预先定义的表示是否为管道操作的标志置1。在标志为1的情况下，首先调用pipe()函数创建无名管道，管道两端的文件描述符分别保存在pipeFd[0]和pipeFd[1]中，管道的作用可以类比为一个共享文件，一个进程将信息写到管道内，另一个进程再从管道内读取信息，就完成了两个进程之间的通信。之后利用fork()函数先创建一个子进程pid1，在pid1中，将标准输出重定向到管道的读入端，并执行命令，命令的输出将输进管道文件。在主进程中，首先用waitpid()函数阻塞主进程，等待子进程pid1返回，待pid1返回后，再次利用fork()函数创建一个子进程pid2，在pid2中，将标准输入重定向到管道的输出端，并执行命令，命令的输入将从管道文件中读取。在主进程中，用waitpid()函数阻塞主进程，等待子进程pid2返回，待pid2也返回后，利用close()函数关闭管道两端即可\n\n");
  printf("使用示例： ls | wc\n");
  printf("*******************************************************************************\n");
  printf("\n\n");
  
  printf("*******************************************************************************\n");
  printf("Chapter 7: 程序环境\n\n");
  printf("简单描述： myshell利用extern直接使用Linux系统保存的环境变量environ，要显示所有的环境变量只需要循环打印即可，修改环境变量则使用getenv()，setenv()等函数即可实现。此外还在程序开头利用shmget()函数创建了一段共享内存用于存储后台进程表的相关信息，共享内存在连接之后可以被所有进程访问到，所以使用共享内存存储后台进程表是较合理的选择\n\n");
  printf("*******************************************************************************\n");
  printf("\n");
 
}

//该函数用来初始化后台进程表(前台进程也放在表中，但查看的时候只输出后台进程)
void Init_joblist()
{
  //利用shmget()函数创建共享内存并获取共享内存标识符,保存到shmID中
  int shmID = shmget((key_t)1234, sizeof(job)*MAX_JOB_NUM + sizeof(int), 0666 | IPC_CREAT);
  //如果shmID的值为-1，说明创建共享内存失败，输出错误信息并退出
  if(shmID == -1){
    fprintf(stderr, RED "[myshell] Error: Create shared memory failed!\n"); 
    exit(1);   
  }
  //利用shmat()函数将共享内存连接到当前进程的地址空间，函数返回值保存到shm中
  void* shm = shmat(shmID, 0, 0);
  //如果shm的值为-1，说明连接共享内存失败，输出错误信息并退出
  if(shm == (void*)-1 ){
    fprintf(stderr, RED "[myshell] Error: Connect shared memory failed!\n"); 
    exit(1);   
  }
  //将shmID赋给全局变量job_shmID
  job_shmID = shmID;
  //将shm赋给全局变量jobs,则jobs指向共享内存段
  jobs = (job*)shm;
  //将shm + sizeof(jobs)*MAX_JOB_NUM后的地址赋给job_count
  job_count = (int*)((char*)shm + sizeof(jobs)*MAX_JOB_NUM);

  
  //初始化后台进程表内的所有进程的pid为-1，说明暂时没有后台进程
  for(int i=0; i<MAX_JOB_NUM; i++)
    jobs[i].pid = -1;
  
  //将myshell进程作为进程表中的第一个进程
  jobs[0].pid = getpid();
  strcpy(jobs[0].jobname, "myshell");
  jobs[0].type = FG;
  jobs[0].status = RUN;
  
  //当前只有myshell一个进程
  *job_count = 1;
}

//该函数用来向后台进程表中添加进程
job* Add_job(pid_t pid, char* jobname, int type, int status)
{
  int count = *job_count;
  //如果进程数已经达到了最大值，则输出错误信息并退出
  if(count == MAX_JOB_NUM){
    fprintf(stderr, RED "[myshell] Error: The job list is full!\n");
    exit(1);
  } 
  
  //否则按传入的参数设置jobs[count]的相应参数
  jobs[count].pid = pid;
  jobs[count].type = type;
  jobs[count].status = status;
  strcpy(jobs[count].jobname, jobname);
  //*job_count的值增加1
  *job_count = count + 1;
  //返回新记录的地址
  return (jobs + count);
}

//该函数用来删除后台进程表中的进程
void Del_job(pid_t pid)
{
  int i;
  int count = *job_count;
  //遍历后台进程表查找进程号为pid的记录
  for(i=0; i<count; i++){
    //找到了则退出
    if(pid == jobs[i].pid)
      break;
  }
  
  //i < count说明找到了相应的记录
  if(i < count){
    //循环左移1位，覆盖了要删除的记录
    for(; i<count-1; i++)
      jobs[i] = jobs[i+1];
    //*job_count的值减去1
    *job_count = count - 1;
  }
}

//该函数用来删除储存后台进程表的共享内存段
void Free_joblist()
{
  //调用shmdt()函数来分离共享内存段与当前进程，失败则输出错误信息并退出
  if(shmdt((void*)jobs) == -1){
    fprintf(stderr, RED "[myshell] Error: Disconnect shared memory failed!\n");
    exit(1);
  }
  
  //调用shmctl()函数来删除共享内存段，失败则输出错误信息并退出
  if(shmctl(job_shmID, IPC_RMID, 0) == -1){
    fprintf(stderr, RED "[myshell] Error: Delete shared memory failed!\n");
    exit(1);
  }
}

//该函数是jobs命令的具体实现，作用是显示所有后台进程的信息
void Jobs_func()
{
  //count保存总进程数
  int count = *job_count;
  //done_count保存状态为DONE(已完成)的记录个数
  int done_count = 0;
  //done_pid[]保存状态为DONE(已完成)的记录的pid
  int done_pid[MAX_JOB_NUM];
  //BG_count保存后台进程的个数
  int BG_count = 0;
    
    //遍历后台进程表
    for(int i=0; i<count; i++){
      //如果jobs[i].type == BG，即为后台进程，则进行相应输出
      if(jobs[i].type == BG){
        //状态为SUSPEND，输出相应序号，进程号，命令名和SUSPEND
        if(jobs[i].status == SUSPEND)
          printf("[%d]  %d  SUSPEND		  %s\n",BG_count+1,jobs[i].pid,jobs[i].jobname);
          
        //状态为RUN，输出相应序号，进程号，命令名和RUN
        else if(jobs[i].status == RUN)
          printf("[%d]  %d  RUNNING		  %s\n",BG_count+1,jobs[i].pid,jobs[i].jobname);
          
        //状态为DONE，输出相应序号，进程号，命令名和DONE
        else{
          printf("[%d]  %d  DONE		  %s\n",BG_count+1,jobs[i].pid,jobs[i].jobname);
          done_pid[done_count] = jobs[i].pid;
          done_count++;     
        }  
        //BG_count递增1
        BG_count++;    
      }

    }
  
  //如果BG_count == 0，说明没有后台进程，输出相应信息    
  if(BG_count == 0)
  printf("No background job currently!\n");
  
  //状态为DONE的进程查看一次之后要进行删除  
  for(int j=0;j<done_count;j++)
    Del_job(done_pid[j]);

}

//该函数是fg命令的具体实现，作用是将后台进程转入前台执行
void Fg_func()
{
  //count保存总进程数
  int count = *job_count;
  int status;
  //BG_count保存后台进程的个数
  int BG_count = 0;
  //last_BG保存最后一个后台进程的下标
  int last_BG;
  
  //遍历后台进程表
  for(int i=0; i<count ;i++)
    if(jobs[i].type == BG){
      //统计后台进程数，保存到BG_count中
      BG_count++;
      //最后一个后台进程的下标保存到last_BG中
      last_BG = i;
    }
      
  //如果BG_count == 0，说明没有后台进程，输出相应信息        
  if(BG_count == 0)
    printf("No background job currently!\n");
    
  //BG_count不为0，则将最后一个后台进程放到前台运行
  else{
    //先输出这个后台进程的进程名
    printf("%s\n",jobs[last_BG].jobname);
    //如果这个进程是挂起的后台进程
    if(jobs[last_BG].status == SUSPEND){
      //改变该进程的类型为FG(前台)
      jobs[last_BG].type = FG; 
      //改变该进程的状态为RUN(运行中) 
      jobs[last_BG].status = RUN;
      //向该进程发送SIGCONT信号，使其继续运行
      kill(jobs[last_BG].pid, SIGCONT);
      //阻塞主进程，等待该进程执行完毕
      waitpid(jobs[last_BG].pid,&status,0);
    }
    
    //如果这个进程是正在运行的后台进程
    else if(jobs[last_BG].status == RUN){
      //改变该进程的类型为FG(前台)
      jobs[last_BG].type = FG;  
      //阻塞主进程，等待该进程执行完毕
      waitpid(jobs[last_BG].pid,&status,0);
    }
  }
}

//该函数是bg命令的具体实现，作用是将挂起的进程转为后台执行
void Bg_func()
{
  //count保存总进程数
  int count = *job_count; 
  //BG_count保存后台进程的个数
  int BG_count = 0;
  int i;
  //遍历后台进程表
  for(i=0; i<count; i++){
    //如果为后台进程
    if(jobs[i].type == BG){
      //如果进程状态为SUSPEND(挂起)
      if(jobs[i].pid > 0 && jobs[i].status == SUSPEND){
        //改变该进程的状态为RUN(运行中)
        jobs[i].status = RUN;
        //向该进程发送SIGCONT信号，使其继续运行
        kill(jobs[i].pid, SIGCONT);
        //输出该进程的序号和进程名
        printf("[%d] %s &\n",BG_count+1,jobs[i].jobname);
        break;
      }
      BG_count++;    
    }

  }
  
  //如果i == count说明没有挂起的后台进程，输出相应提示信息
  if(i == count)
    printf("No suspend job in the background now!\n");
}

