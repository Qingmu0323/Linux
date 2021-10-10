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


void handler(int sig,siginfo_t *s_t,void *p)//�ܹ����ܶ������ݵ��źŴ�����ǩ��
{
  int tmp = 0;
  tmp = s_t->si_int;  //si_int��si_value.sival_int��һ����--��Զ���������int��ʱ��
    printf("Aloha recv a sig=%d\tvar :%d\n and var is also: %d", sig,tmp,s_t->si_value.sival_int);  
}



int main(int argc, char *argv[])
{   
  pid_t     pid;
    int ret = 0;
  int i = 0;
  union sigval mysigval;//������Ŷ�������
    struct sigaction act;//����ע���ź�

  /*ʹ��sigaction����Ҫ��ʼ����������Ա*/
  act.sa_sigaction = handler;//ָ���ص�����
  act.sa_flags = SA_SIGINFO;//������Ҫ--ֻ�е���SA_SIGINFO���źŴ��������ܽ��ܶ�������
  sigemptyset(&act.sa_mask);//���������

  if(sigaction(SIGINT,&act,NULL) < 0)//ע���ź�--ָ���ٵ�����
  {
    perror("sigaction error!\n");
    exit(-1);
  }

  pid = fork();//�����ӽ���

  if(-1 == pid)
  {
    perror("fork");
    exit(-1);
  }
  else if(0 == pid)
  {
    mysigval.sival_int = 125;//����Ҫ�����źŷ��͵Ķ�������
    for(i = 0;i < 10;i++)//�ӽ��̷���ʮ���ź�--SIGINT�ǲ��ɿ��ź�--�����е���
    {
      ret = sigqueue(getppid(),SIGINT,mysigval);//��ʼ�����ź�
      if(ret != 0)//����ʧ��
      {
        perror("sigqueue");
        exit(-1);
      }
      else{//����0��ʾ�źŷ��ͳɹ�
        printf("send ok!\n");
        sleep(1);
      }
    }
  }
  else if(pid > 0)
  {
    while(1);//��������ѭ��
  } 

    return 0;
}