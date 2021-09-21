/******************************************************************************

                  ��Ȩ���� (C), 2017-2017, chandler

 ******************************************************************************
  �� �� ��   : server.c
  �� �� ��   : ����
  ��    ��   : chandler
  ��������   : 2018��5��6��
  ����޸�   :
  ��������   : ��������ʵ���ļ�
  �����б�   :
*
*       1.                SRV_accpet_process
*       2.                SRV_creat_proc_server
*       3.                SRV_handle_client
*       4.                SRV_init
*       5.                SRV_msg_process
*       6.                SRV_recv_msg
*       7.                SRV_release
*       8.                SRV_write_msg
*

  �޸���ʷ   :
  1.��    ��   : 2018��5��6��
    ��    ��   : chandler
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/


#include "./include/server.h"
/*****************************************************************************
 �� �� ��  : SRV_init
 ��������  : �������ĳ�ʼ��,�������� ��ʼ������
 �������  : ��
 �������  : ��
 �� �� ֵ  : int 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��9��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
int  SRV_init( )
{
   unsigned int i=0;
   g_ptSrvOBJ = (struct server_obj *)malloc(sizeof(struct server_obj));
   if(g_ptSrvOBJ ==NULL)
   {
	  return -1;
   }
   memset(g_ptSrvOBJ,0,sizeof(struct server_obj));
   
   g_ptSrvOBJ->current_clicnt = 0;
   
   return 0;
}


/*****************************************************************************
 �� �� ��  : SRV_creat_proc_server
 ��������  : int
 �������  : const char *ip
             int port
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��9��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
int SRV_creat_proc_server( const char *ip, int port )
{
   int  fd;
   fd=socket(AF_INET,SOCK_STREAM,0);
   
   if(fd==-1)
   {
	 printf("SRV_creat_proc_server socket error\n");
	 return -1;
   }
   
   //���÷������ĵ�ַ
   bzero(&g_ptSrvOBJ->seraddr,sizeof(g_ptSrvOBJ->seraddr));
   g_ptSrvOBJ->seraddr.sin_family = AF_INET;
   inet_pton(AF_INET,ip,&g_ptSrvOBJ->seraddr.sin_addr);
   g_ptSrvOBJ->seraddr.sin_port =htons(port);

   //�󶨵�ַ
   if(bind(fd,(struct sockaddr*)&(g_ptSrvOBJ->seraddr),sizeof(g_ptSrvOBJ->seraddr))==-1)
   {
   		perror("bind error\n");
		return -1;
   }

   //��������������
   listen(fd,LISTSIZE);
   
   return fd;
}


/*****************************************************************************
 �� �� ��  : SRV_release
 ��������  : �������ͷź���
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��9��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
void SRV_release( void )
{
    if(g_ptSrvOBJ)
    {
		free(g_ptSrvOBJ);
		g_ptSrvOBJ=NULL;
	}
}




/*****************************************************************************
 �� �� ��  : SRV_msg_process
 ��������  : �������ݺ���
 �������  : int fd                     
             char *buf                  
             struct epoll_event *event  
 �������  : ��
 �� �� ֵ  : static int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��5��6��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
static int SRV_msg_process( int fd, char *buf,struct epoll_event *event)
{
	/*���������*/
    assert(buf);
	printf("recv data:%s\n",buf);
	event->events = EPOLLOUT;
	epoll_ctl((g_ptSrvOBJ->epollfd),EPOLL_CTL_MOD,fd,event);
	return 1;
}


/*****************************************************************************
 �� �� ��  : SRV_write_msg
 ��������  : ������ͨ�ź���
 �������  : int writefd ,struct epoll_event *event int cnt
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��11��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SRV_write_msg(int writefd,struct epoll_event *event,int cnt)
{
	int iRet=0;
	char buf[BUFFSIZE]={0};
	fprintf(buf,"hello,client[%d]",cnt);
	iRet = write(writefd,buf,strlen(buf));
	if(iRet <0)
	{
		/*д��ʧ�� �رտͻ���*/
		printf("client close\n");
		close(writefd);
		epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_DEL,writefd,event);
		g_ptSrvOBJ->current_clicnt--;
		return;
	}
	/*д��ɹ����þ�����¼�*/
	event->events = EPOLLIN;
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_MOD,writefd,event);
}



/*****************************************************************************
 �� �� ��  : SRV_recv_msg
 ��������  : ������ͨ�ź���
 �������  : fd_set *readfds
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��11��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SRV_recv_msg(int readfd,struct epoll_event *event)
{
	int iRet=0;
	char buf[BUFFSIZE]={0};
	iRet = read(readfd,buf,BUFFSIZE);
	if(iRet <=0)
	{
		/*����Ϊ�� ����رտͻ���*/
		printf("client close\n");
		close(readfd);
		epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_DEL,readfd,event);
		g_ptSrvOBJ->current_clicnt--;
		return;
	}
	/*���ݵ��߼�����ģ��*/
	SRV_msg_process(readfd,buf,event);
}


/*****************************************************************************
 �� �� ��  : SRV_accpet_process
 ��������  : ����������
 �������  : int srvfd
 �������  : ��
 �� �� ֵ  : static int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��11��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
static int SRV_accpet_process( int srvfd )
{
	struct sockaddr_in tCliaddr;
	socklen_t 		   tCliaddlen;
	tCliaddlen = sizeof(tCliaddr);
	int confd  = -1;
	unsigned int i =0;
ACCEPT:
    /*�����µ�����*/
	confd=accept(srvfd,(struct sockaddr*)(&tCliaddr),&tCliaddlen);
	if(confd == -1)
	{
		if(errno==EINTR)
		{
			goto ACCEPT;
		}
		else
		{
			fprintf(stderr,"accept error,error:%s\n",strerror(errno));
			return -1;
		}
	}
	/*���Ի�ȡ�µ�����*/
	fprintf(stdout,"accept new client:%s:%d\n",inet_ntoa(tCliaddr.sin_addr),tCliaddr.sin_port);

	/*�����������ӷ���epollfd��*/
	struct epoll_event te;
	te.events = EPOLLIN;
	te.data.fd = confd;
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_ADD,confd,&te);
	g_ptSrvOBJ->current_clicnt++;
	return 1;
}

/*****************************************************************************
 �� �� ��  : SRV_handle_client
 ��������  : ����������ͻ�������
 �������  : int srvfd
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��7��9��
    ��    ��   : chandler
    �޸�����   : �����ɺ���

*****************************************************************************/
void SRV_handle_client( int srvfd )
{
	int iRet=0;
	unsigned int i =0;
	int clifd=0;
	/*����epollfd����ܵ��ļ�������*/
	g_ptSrvOBJ->epollfd = epoll_create(LISTSIZE);
	/*���õ�ǰ�ļ��������epoll_event�ṹ�壬��epoll_event����������*/
	struct epoll_event tEvent;
	tEvent.events  = EPOLLIN;
	tEvent.data.fd = srvfd;
	/*�����úõ�epoll_event��ӵ�epollfd��*/
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_ADD,srvfd,&tEvent);
	g_ptSrvOBJ->current_clicnt = 0;
	
    while(1)
    {	
		/*���е�ǰepoll�ȴ�,����ֵΪ�������¼�����*/
		iRet=epoll_wait(g_ptSrvOBJ->epollfd,g_ptSrvOBJ->evs,LISTSIZE,-1);
		if(iRet ==-1)
		{
			perror("poll error\n");
			return;
		}

		/*�жϷ������ٸ�io*/
		for(i=0;i<iRet;i++)
		{
			clifd = g_ptSrvOBJ->evs[i].data.fd;
			/*�ж��Ƿ�Ϊ����������¼�����*/
			if((clifd==srvfd)&&(g_ptSrvOBJ->evs[i].events & EPOLLIN))
			{
				/*����accpet����*/
				SRV_accpet_process(srvfd);
			}
			/*�ж��Ƿ�Ϊ�ͻ����Ӿ�����¼�����*/
			if(g_ptSrvOBJ->evs[i].events & EPOLLIN)
			{
			    /*����ͻ��˷��͵���Ϣ*/
			    SRV_recv_msg(g_ptSrvOBJ->evs[i].data.fd,&g_ptSrvOBJ->evs[i]);
			}
			if(g_ptSrvOBJ->evs[i].events & EPOLLOUT)
			{
			    /*�ظ��ͻ�����Ϣ*/
			    SRV_write_msg(g_ptSrvOBJ->evs[i].data.fd,&g_ptSrvOBJ->evs[i],i);
			}
		}

	}
}
