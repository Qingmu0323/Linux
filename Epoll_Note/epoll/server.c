/******************************************************************************

                  版权所有 (C), 2017-2017, chandler

 ******************************************************************************
  文 件 名   : server.c
  版 本 号   : 初稿
  作    者   : chandler
  生成日期   : 2018年5月6日
  最近修改   :
  功能描述   : 服务器的实现文件
  函数列表   :
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

  修改历史   :
  1.日    期   : 2018年5月6日
    作    者   : chandler
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


#include "./include/server.h"
/*****************************************************************************
 函 数 名  : SRV_init
 功能描述  : 服务器的初始化,创建对象 初始化对象
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月9日
    作    者   : chandler
    修改内容   : 新生成函数

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
 函 数 名  : SRV_creat_proc_server
 功能描述  : int
 输入参数  : const char *ip
             int port
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月9日
    作    者   : chandler
    修改内容   : 新生成函数

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
   
   //设置服务器的地址
   bzero(&g_ptSrvOBJ->seraddr,sizeof(g_ptSrvOBJ->seraddr));
   g_ptSrvOBJ->seraddr.sin_family = AF_INET;
   inet_pton(AF_INET,ip,&g_ptSrvOBJ->seraddr.sin_addr);
   g_ptSrvOBJ->seraddr.sin_port =htons(port);

   //绑定地址
   if(bind(fd,(struct sockaddr*)&(g_ptSrvOBJ->seraddr),sizeof(g_ptSrvOBJ->seraddr))==-1)
   {
   		perror("bind error\n");
		return -1;
   }

   //开启服务器监听
   listen(fd,LISTSIZE);
   
   return fd;
}


/*****************************************************************************
 函 数 名  : SRV_release
 功能描述  : 服务器释放函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月9日
    作    者   : chandler
    修改内容   : 新生成函数

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
 函 数 名  : SRV_msg_process
 功能描述  : 发送数据函数
 输入参数  : int fd                     
             char *buf                  
             struct epoll_event *event  
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月6日
    作    者   : chandler
    修改内容   : 新生成函数

*****************************************************************************/
static int SRV_msg_process( int fd, char *buf,struct epoll_event *event)
{
	/*做参数检测*/
    assert(buf);
	printf("recv data:%s\n",buf);
	event->events = EPOLLOUT;
	epoll_ctl((g_ptSrvOBJ->epollfd),EPOLL_CTL_MOD,fd,event);
	return 1;
}


/*****************************************************************************
 函 数 名  : SRV_write_msg
 功能描述  : 新链接通信函数
 输入参数  : int writefd ,struct epoll_event *event int cnt
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月11日
    作    者   : chandler
    修改内容   : 新生成函数

*****************************************************************************/
static void SRV_write_msg(int writefd,struct epoll_event *event,int cnt)
{
	int iRet=0;
	char buf[BUFFSIZE]={0};
	fprintf(buf,"hello,client[%d]",cnt);
	iRet = write(writefd,buf,strlen(buf));
	if(iRet <0)
	{
		/*写入失败 关闭客户端*/
		printf("client close\n");
		close(writefd);
		epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_DEL,writefd,event);
		g_ptSrvOBJ->current_clicnt--;
		return;
	}
	/*写入成功重置句柄的事件*/
	event->events = EPOLLIN;
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_MOD,writefd,event);
}



/*****************************************************************************
 函 数 名  : SRV_recv_msg
 功能描述  : 新链接通信函数
 输入参数  : fd_set *readfds
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月11日
    作    者   : chandler
    修改内容   : 新生成函数

*****************************************************************************/
static void SRV_recv_msg(int readfd,struct epoll_event *event)
{
	int iRet=0;
	char buf[BUFFSIZE]={0};
	iRet = read(readfd,buf,BUFFSIZE);
	if(iRet <=0)
	{
		/*数据为空 代表关闭客户端*/
		printf("client close\n");
		close(readfd);
		epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_DEL,readfd,event);
		g_ptSrvOBJ->current_clicnt--;
		return;
	}
	/*数据的逻辑处理模块*/
	SRV_msg_process(readfd,buf,event);
}


/*****************************************************************************
 函 数 名  : SRV_accpet_process
 功能描述  : 处理监听句柄
 输入参数  : int srvfd
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月11日
    作    者   : chandler
    修改内容   : 新生成函数

*****************************************************************************/
static int SRV_accpet_process( int srvfd )
{
	struct sockaddr_in tCliaddr;
	socklen_t 		   tCliaddlen;
	tCliaddlen = sizeof(tCliaddr);
	int confd  = -1;
	unsigned int i =0;
ACCEPT:
    /*接收新的链接*/
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
	/*可以获取新的链接*/
	fprintf(stdout,"accept new client:%s:%d\n",inet_ntoa(tCliaddr.sin_addr),tCliaddr.sin_port);

	/*将新来的链接放入epollfd中*/
	struct epoll_event te;
	te.events = EPOLLIN;
	te.data.fd = confd;
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_ADD,confd,&te);
	g_ptSrvOBJ->current_clicnt++;
	return 1;
}

/*****************************************************************************
 函 数 名  : SRV_handle_client
 功能描述  : 服务器处理客户端请求
 输入参数  : int srvfd
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月9日
    作    者   : chandler
    修改内容   : 新生成函数

*****************************************************************************/
void SRV_handle_client( int srvfd )
{
	int iRet=0;
	unsigned int i =0;
	int clifd=0;
	/*创建epollfd这个总的文件描述符*/
	g_ptSrvOBJ->epollfd = epoll_create(LISTSIZE);
	/*设置当前的监听句柄的epoll_event结构体，用epoll_event代表监听句柄*/
	struct epoll_event tEvent;
	tEvent.events  = EPOLLIN;
	tEvent.data.fd = srvfd;
	/*将设置好的epoll_event添加到epollfd中*/
	epoll_ctl(g_ptSrvOBJ->epollfd,EPOLL_CTL_ADD,srvfd,&tEvent);
	g_ptSrvOBJ->current_clicnt = 0;
	
    while(1)
    {	
		/*进行当前epoll等待,返回值为发生的事件个数*/
		iRet=epoll_wait(g_ptSrvOBJ->epollfd,g_ptSrvOBJ->evs,LISTSIZE,-1);
		if(iRet ==-1)
		{
			perror("poll error\n");
			return;
		}

		/*判断发生多少个io*/
		for(i=0;i<iRet;i++)
		{
			clifd = g_ptSrvOBJ->evs[i].data.fd;
			/*判断是否为监听句柄的事件发生*/
			if((clifd==srvfd)&&(g_ptSrvOBJ->evs[i].events & EPOLLIN))
			{
				/*处理accpet程序*/
				SRV_accpet_process(srvfd);
			}
			/*判断是否为客户链接句柄的事件发生*/
			if(g_ptSrvOBJ->evs[i].events & EPOLLIN)
			{
			    /*处理客户端发送的消息*/
			    SRV_recv_msg(g_ptSrvOBJ->evs[i].data.fd,&g_ptSrvOBJ->evs[i]);
			}
			if(g_ptSrvOBJ->evs[i].events & EPOLLOUT)
			{
			    /*回复客户端消息*/
			    SRV_write_msg(g_ptSrvOBJ->evs[i].data.fd,&g_ptSrvOBJ->evs[i],i);
			}
		}

	}
}
