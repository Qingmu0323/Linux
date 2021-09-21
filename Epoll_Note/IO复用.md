# cLinux中文件IO管理&&数据通信之IO复用模块

1. 标准流和流功能
stdin   0 scanf
stdout  1 printf 
stderr  2  perror

```c
 p *(struct _IO_FILE *)&_IO_2_1_stdin_
```

write(目的，源，length)
write(stdout,inet_ntoa(in.sin_addr),sizeof(in.sin_addr))
printf 

read(0)
scanf
write
read 

fd=open("/dev/tty");
write(fd,buf,length)


recv 
send 

2. 文件流指针
fopen
_IO_FILE

struct _IO_FILE {
int _flags; /* High-order word is _IO_MAGIC; rest is flags. */
#define _IO_file_flags _flags
/* The following pointers correspond to the C++ streambuf protocol. */
/* Note: Tk uses the _IO_read_ptr and _IO_read_end fields directly. */
char* _IO_read_ptr; /* Current read pointer */
char* _IO_read_end; /* End of get area. */
char* _IO_read_base;    /* Start of putback+get area. */
char* _IO_write_base;   /* Start of put area. */
char* _IO_write_ptr;    /* Current put pointer. */
char* _IO_write_end;    /* End of put area. */
char* _IO_buf_base; /* Start of reserve area. */
char* _IO_buf_end;  /* End of reserve area. */
/* The following fields are used to support backing up and undo. */
char *_IO_save_base; /* Pointer to start of non-current get area. */
char *_IO_backup_base; /* Pointer to first valid character of backup area */
char *_IO_save_end; /* Pointer to end of non-current get area. */

struct _IO_marker *_markers;

struct _IO_FILE *_chain;

int _fileno;
int _blksize;
_IO_off_t _old_offset; /* This used to be _offset but it's too small. */

#define __HAVE_COLUMN /* temporary */
/* 1+column number of pbase(); 0 is unknown. */
unsigned short _cur_column;
signed char _vtable_offset;
char _shortbuf[1];

/* char* _save_gptr; char* _save_egptr; */

_IO_lock_t *_lock;
#ifdef _IO_USE_OLD_IO_FILE
};

scanf()   /n
read(0,buf,size)  EOF 
gets() /n
printf 
write(0,buf)
puts(0) /n

全缓冲区 
	BUFSIZE
	fflush 
	默认开启
行缓冲区
	遇到换行符才进行刷新 
无缓冲区
	read 
	write
	strerr

指定缓冲区
   void setbuf(FILE *stream, char *buf);
		* buf参数必须指向一个长度为BUFSIZE的缓冲区  

   void setbuffer(FILE *stream, char *buf, size_t size);
		* 
      void setlinebuf(FILE *stream);
		

_IONBF 无缓冲
_IOLBF 行缓冲
_IOFBF 全缓冲 


* IO的权限问题 
	FILE *fopen(const char *path, const char *mode);
	r 
	r+ rb+
	w
	w+ 
	a
	a+ 

* 文件读写函数 
	fgetc
	getc
	gets 
	
	puts -->\n
	
* gcc -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64


	read(fd,buf,size)
	write(fd,buf,size)
	
	文件流定位
	 int fseek(FILE *stream, long offset, int whence);
			SEEK_SET  文件开始的位置
			SEEK_CUR  
			SEEK_END   文件结束的位置
	
	long ftell(FILE *stream);  文件读写指针位置距离文件开始的字节数
	void rewind(FILE *stream);  文件读写指针复位


IO复用的内核实现 

select 
	syscall_select 
		do_select 
			for (;;)
				if (retval || !*timeout || signal_pending(current))
			


## 区别和联系
1. 区别
	文件的量不同 
	处理的实时性要求不同 
	
2. 联系
	都是文件可以使用同样的函数 



## socket的IO模型
* 非阻塞IO处理 
* 阻塞IO处理
* IO复用的IO方式
* 异步IO
* 信号驱动式IO模型
https://www.cnblogs.com/LittleHann/p/3897910.html

## 概念 
* 单个句柄占用一个输入 输出 造成资源过剩，使用文件集合占用一个输入输出 
* 非阻塞IO处理 
	

## 方法
* select
* poll
* epoll

### select IO 复用手段
* 对象-->fd_set 文件集合
* 使用领域
* IO复用流程 

1. struct fd_set 
* 分配 struct fd_set rfds;
* 设置 
	初始化    FD_ZERO(&rfds);
	加入句柄  FD_SET(fd, &rfds);
	移除句柄  FD_CLR(int fd, fd_set *set);
* 使用
	int select(int nfds, fd_set *readfds,fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);
	
	int nfds 文件集合中最大的fd+1
	struct timeval *timeout
    struct timeval {
       long    tv_sec;         /* seconds */
       long    tv_usec;        /* microseconds */
     };

	判断由哪个fd引起的返回  int FD_ISSET(int fd, fd_set *set);

2. 使用方法 
首先进行select 等待 
accept 

* 注意： 每次重新发起select时，需要对fd_set重新添加 
* 注意： 讲放入到select中的句柄都设置为非阻塞状态
glibc 

#### IO复用流程

#### select demo演示





```
	/* According to POSIX.1-2001 */
	#include <sys/select.h>
	
	/* According to earlier standards */
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
```

#### 使用场合
  服务器：
	tcp 与 udp同时进行监听     
	既作为服务器 又作为客户端  代理  select
  客户端：
	监听多个句柄时()	 1000
	
stdin 
fd 进程间通信句柄
socket




#### poll机制
* 分析和已有模型的区别
select --->fd_set包含需要进行IO复用的句柄 
poll  ---->pollfd包含需要进行IO复用的句柄  pollfd句柄链表   句柄链表的头 
* 对象
```
struct pollfd{
	int fd;  //当前的处理句柄
	short events; 句柄关心的事件
	short revents;
}

event
/* These are specified by iBCS2 */
#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

listen_fd

pollfd 
 .fd = listen_fd
 .events = POLLIN | POLLPRI 
int poll(struct pollfd *fds, nfds_t nfds, int timeout);





/* The rest seem to be more-or-less nonstandard. Check them! */
#define POLLRDNORM	0x0040
#define POLLRDBAND	0x0080
#ifndef POLLWRNORM
#define POLLWRNORM	0x0100
#endif
#ifndef POLLWRBAND
#define POLLWRBAND	0x0200
#endif
#ifndef POLLMSG
#define POLLMSG		0x0400
#endif
#ifndef POLLREMOVE
#define POLLREMOVE	0x1000
#endif
#ifndef POLLRDHUP
#define POLLRDHUP       0x2000
#endif

#define POLLFREE	0x4000	/* currently only for epoll */

#define POLL_BUSY_LOOP	0x8000


revents 回复事件
事件返回指针 	
struct pollfd;
pollfd.fd
pollfd.event
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

revents
struct pollfd{
	int fd;  //当前的处理句柄
	short events; 句柄关心的事件
	short revents;
}


fd_set  pollfd
```


1. 使用poll select 实现回射服务器（客户端发什么 服务端回什么）
要求：客户端和服务端都需要使用select的机制 
2. 设计一个tcp的代理




select 与 poll的内核机制分析 
select poll
	int 80 systemcall
		__syscall_start
			sys_select
			   core_sys_select
				  do_select
						*timeout -= __timeout;
							   	const struct file_operations	*f_op;
										unsigned int (*poll) (struct file *, struct poll_table_struct *);
									f_op->poll --->mask
unsigned int datagram_poll(struct file *file, struct socket *sock,
			   poll_table *wait)
{
	struct sock *sk = sock->sk;
	unsigned int mask;

	poll_wait(file, sk->sk_sleep, wait);
	mask = 0;
	
	/* exceptional events? */
	if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
		mask |= POLLERR;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= POLLRDHUP;
	if (sk->sk_shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;
	
	/* readable? */
	if (!skb_queue_empty(&sk->sk_receive_queue) ||
	    (sk->sk_shutdown & RCV_SHUTDOWN))
		mask |= POLLIN | POLLRDNORM;
	
	/* Connection-based need to check for termination and startup */
	if (connection_based(sk)) {
		if (sk->sk_state == TCP_CLOSE)
			mask |= POLLHUP;
		/* connection hasn't started yet? */
		if (sk->sk_state == TCP_SYN_SENT)
			return mask;
	}
	
	/* writable? */
	if (sock_writeable(sk))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
	else
		set_bit(SOCK_ASYNC_NOSPACE, &sk->sk_socket->flags);
	
	return mask;
}



#epoll 				

* 并发性高，可同时监听C10K以上级别的客户事件
	select ----> fd--->fd_set --轮训
	poll    ---- fd--->pollfd
	epoll   ----> fd---event
epoll   ： 大规模并发服务框架
		   epoll+threadpool+mysql

* 对象 
	epoll_fd  epoll_event 
```
创建 epollfd对象：
	int epoll_create(int size); 返回值 epoll_fd epoll_event
设置 epollfd对象：
	创建 epoll_event对象
		 struct epoll_event ep_ev;
	设置 epoll_event对象
		   ep_ev.events = EPOLLIN;//数据的读取
    	   ep_ev.data.fd = listen_sock;	
	使用 epoll_event对象
		  epoll_ctl(int epfd,int op, int fd, struct epoll_event *event);
		  epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_sock,&ep_ev);
使用epollefd对象： 
	 int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);
// 第二个struct epoll_event *events是用来做返回的
// maxevents 
// timeout




typedef union epoll_data {
       void    *ptr;
       int      fd;
       uint32_t u32;
       uint64_t u64;
   } epoll_data_t;

   struct epoll_event {
       uint32_t     events;    /* Epoll events */
       epoll_data_t data;      /* User data variable */
   };


epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
EPOLL_CTL_ADD
EPOLL_CTL_MOD
EPOLL_CTL_DEL


```

* demo 1
* demo 2




* 项目设计 


1. epooll 如何应用于大并发服务器的IO通信模块设计
2. 聊天室这类程序的设计方式(接收转发类程序)
3. 面向对象的编程思想 


struct 
{
	sendfd
	recvfd
	ip
}

struct protocal
{
	int addr
	char *buf
	int number;
}


sem_open
sem_wait
sem_post	



作业：
	1. 转发程序 端口转发程序
		端口转发程序的客户端 与 服务端