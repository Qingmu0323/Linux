#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


void handler(int sig,siginfo_t *s_t,void *p)//能够接受额外数据的信号处理函数签名
{
  int tmp = 0;
  tmp = s_t->si_int;  //si_int和si_value.sival_int是一样的--针对额外数据是int的时候。
    printf("Aloha recv a sig=%d\tvar :%d\n and var is also: %d", sig,tmp,s_t->si_value.sival_int);  
}



int main(int argc, char *argv[])
{   
  pid_t     pid;
    int ret = 0;
  int i = 0;
  union sigval mysigval;//用来存放额外数据
    struct sigaction act;//用来注册信号

  /*使用sigaction必须要初始化的三个成员*/
  act.sa_sigaction = handler;//指定回调函数
  act.sa_flags = SA_SIGINFO;//尤其重要--只有等于SA_SIGINFO，信号处理函数才能接受额外数据
  sigemptyset(&act.sa_mask);//清空屏蔽字

  if(sigaction(SIGINT,&act,NULL) < 0)//注册信号--指定毁掉函数
  {
    perror("sigaction error!\n");
    exit(-1);
  }

  pid = fork();//创建子进程

  if(-1 == pid)
  {
    perror("fork");
    exit(-1);
  }
  else if(0 == pid)
  {
    mysigval.sival_int = 125;//设置要随着信号发送的额外数据
    for(i = 0;i < 10;i++)//子进程发送十次信号--SIGINT是不可靠信号--传送有点慢
    {
      ret = sigqueue(getppid(),SIGINT,mysigval);//开始发送信号
      if(ret != 0)//发送失败
      {
        perror("sigqueue");
        exit(-1);
      }
      else{//返回0表示信号发送成功
        printf("send ok!\n");
        sleep(1);
      }
    }
  }
  else if(pid > 0)
  {
    while(1);//父进程死循环
  } 

    return 0;
}