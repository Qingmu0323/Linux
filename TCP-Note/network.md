#              Linux 网络编程
## 目标
* 网络协议：ipv4 ipv6 icmp arp  ospf rip igmp mpls pppoe ptp....... 
		1.   网络数据分析   直接分析数据包---抓包分析
		2.   协议图 (静态 动态)
* 网络服务器模型：
  		1.  数据通信模块
	      1.1 阻塞接收 
		
		​	1.2 非阻塞接收
		​	1.3 IO复用接收 select poll epoll 
		​	1.4 异步接收 signal sigation 
		
		2.  数据处理模块
			2.1 多进程数据处理能力
			2.2 多进程的IPC	
				有名管道
				无名管道
				信号
				消息队列
				信号量
				共享内存
				域socket
			2.3 多线程数据处理能力
			2.4 多线程的互斥与同步
				  mutex 
				  spin
				  rwlock
				  condition
			2.5 多线程多进程（线程池 任务池)
		3.  数据存储模块
		4.  网络服务器设计框架
			4.1 reactor
			4.2 practor
		5.  功能协议实现
			5.1  http
			5.2  https
			5.3  ftp 
			.......
		6.  功能框架
			6.1  web服务器框架 
## 网络协议：
一切应用层的功能，都是内核层实现 
Linux中都是面向对象的思想  

aaaaa --> fd    ----> open(/tmp/test.txt)--->write---->read
aaaaa -->socket ----> socket();  ------------------>write-----read-----accetp--connect--setsocketopt
					  bind() struct sockeraddr_in 
					  listen(size)
ip tcp udp 
层次：
	物理层 :PCS(物理编码层) PMA(物理适配层)
	LLC层(MAC)  :局域网链路问题  ARP 
	IP层   :网络传输问题 
	TCP层  :进程接收数据 
协议数据：
	
协议图：

demo:

* 网络编程的基础知识 
	* TCP/IP协议镞 
		1. 分层的概念 
			MAC （源MAC 目的MAC L/T）--->ARP  
			ip   
			传输层 TCP UDP --->端口 
			应用层HTTP HTTP FTP TELNET SSH  
		2. ip 格式
		   点分十进制  二进制
		   网络字节序(大端序)  本地字节序（小端序 ）
		    case 1: 点分十进制------>网络字节序的二进制 
			in_addr_t inet_addr(const char *cp);
		    case 2: 网络字节序的二进制 --->点分十进制
		    char *inet_ntoa(struct in_addr in);
		    case 3:点分十进制------>网络字节序的二进制 
		    int inet_aton(const char *cp, struct in_addr *inp);
		3. 子网掩码
		   24  11111111111111111100000000
		   255.255.255.0 
		   ip---->ip段 
		   192.168.1.1 --->192.168.1.1/24
		4.拆包和封包 
			HTTP协议
0x12345678
0x78 0x56 0x34 0x12

* tcp的编程原理和编程流程 
	监听句柄-->接收新的链接--->电话号
	链接句柄-->接收新的客户端--->呼入电话
* tcp的简单程序 


### socket模型

流套接字 TCP  SOCK_STREAM
数据报套接字 UDP SOCK_DGRAM
原始套接字  raw SOCK_RAW

tcp 三次握手 
icmp --- ping traceroute
元数据 

```C
const struct proto_ops inet_stream_ops = {
	.family		   = PF_INET,
	.owner		   = THIS_MODULE,
	.release	   = inet_release,
	.bind		   = inet_bind,
	.connect	   = inet_stream_connect,
	.socketpair	   = sock_no_socketpair,
	.accept		   = inet_accept,
	.getname	   = inet_getname,
	.poll		   = tcp_poll,
	.ioctl		   = inet_ioctl,
	.listen		   = inet_listen,
	.shutdown	   = inet_shutdown,
	.setsockopt	   = sock_common_setsockopt,
	.getsockopt	   = sock_common_getsockopt,
	.sendmsg	   = tcp_sendmsg,
	.recvmsg	   = sock_common_recvmsg,
	.mmap		   = sock_no_mmap,
	.sendpage	   = tcp_sendpage,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};
```

```C
const struct proto_ops inet_dgram_ops = {
	.family		   = PF_INET,
	.owner		   = THIS_MODULE,
	.release	   = inet_release,
	.bind		   = inet_bind,
	.connect	   = inet_dgram_connect,
	.socketpair	   = sock_no_socketpair,
	.accept		   = sock_no_accept,
	.getname	   = inet_getname,
	.poll		   = udp_poll,
	.ioctl		   = inet_ioctl,
	.listen		   = sock_no_listen,
	.shutdown	   = inet_shutdown,
	.setsockopt	   = sock_common_setsockopt,
	.getsockopt	   = sock_common_getsockopt,
	.sendmsg	   = inet_sendmsg,
	.recvmsg	   = sock_common_recvmsg,
	.mmap		   = sock_no_mmap,
	.sendpage	   = inet_sendpage,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};

```

```
static const struct proto_ops inet_sockraw_ops = {
	.family		   = PF_INET,
	.owner		   = THIS_MODULE,
	.release	   = inet_release,
	.bind		   = inet_bind,
	.connect	   = inet_dgram_connect,
	.socketpair	   = sock_no_socketpair,
	.accept		   = sock_no_accept,
	.getname	   = inet_getname,
	.poll		   = datagram_poll,
	.ioctl		   = inet_ioctl,
	.listen		   = sock_no_listen,
	.shutdown	   = inet_shutdown,
	.setsockopt	   = sock_common_setsockopt,
	.getsockopt	   = sock_common_getsockopt,
	.sendmsg	   = inet_sendmsg,
	.recvmsg	   = sock_common_recvmsg,
	.mmap		   = sock_no_mmap,
	.sendpage	   = inet_sendpage,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};
```




```BASH
struct tcphdr {
	__be16	source;
	__be16	dest;
	__be32	seq;
	__be32	ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16	doff:4,
		res1:4,
		cwr:1,
		ece:1,
		urg:1,
		ack:1,
		psh:1,
		rst:1,
		syn:1,
		fin:1;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif	
	__be16	window;
	__sum16	check;
	__be16	urg_ptr;
};


struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	ihl:4,
		version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8	version:4,
  		ihl:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	tos;
	__be16	tot_len;
	__be16	id;
	__be16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__sum16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

```



listen : 
int listen(int sockfd, int backlog);

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); //返回连接connect_fd

TCP 
•	read()/write()
•	recvmsg()/sendmsg()
UDP 
•	recvfrom()/sendto()

   ssize_t read(int fd, void *buf, size_t count);
   ssize_t write(int fd, const void *buf, size_t count);

   ssize_t send(int sockfd, const void *buf, size_t len, int flags);
  ssize_t recv(int sockfd, void *buf, size_t len, int flags);

  ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen);
  ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);


客户端
int  socket(int protofamily, int type, int protocol);//返回sockfd
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

```
#include <unistd.h>
int close(int fd);
```

inet_addr 192.168.2.55 --- int addr 
inet_ntoa(peer.sin_addr)


tcpdump -iany tcp port xxxx > data.pacp 




unsigned char * buf;
buf=malloc(sizeof(struct iphdr));
struct iphdr * iph;
iph = (struct iphdr * )buf;
iph->ihl=
iph->saddr=


send (buf);



int socket(int domain, int type, int protocol);
TCP
UDP 

ICMP
 socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
SYN
 socket(AF_INET,SOCK_RAW,IPPROTO_IP);



int bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
struct sockaddr {
           sa_family_t sa_family;
           char        sa_data[14];
       }
struct sockaddr_in



五元组：源IP 目的IP 源port 目的port 类型type

1.创建对象
	fd=socket(协议簇，类型，协议)
	int  socket(int protofamily, int type, int protocol);//返回sockfd
	协议族：
		AF_INET
		AF_INET6
		AF_LOCAL
		AF_ROUTE
	套接字类型
		SOCK_STREAM
		SOCK_DGRAM
		SOCK_RAW
	协议：
		IPPROTO_TCP
		IPPTOTO_UDP
		IPPROTO_SCTP
		IPPROTO_TIPC
2.设置对象		
	int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	

	地址填充 ，端口填充使用
	struct sockaddr_in {
		 sa_family_t    sin_family; /* address family: AF_INET */
		 in_port_t      sin_port;   /* port in network byte order */ short 
		 struct in_addr sin_addr;   /* internet address */		
	};
	struct in_addr {
		    uint32_t       s_addr;     /* address in network byte order */
	};

  192.168.199.67 
  127.0.0.1  本机地址
  0.0.0.0    本机回环
  INADDR_ANY
	

    192.168.199.67：80 
    0.0.0.0：80
    127.0.0.1 ：80


	sockaddr 
		接口传递用到
		
		字节序概念
		本地字节序
		网络字节序
		htons()
		htonl
	
		hton
		htons(80)
	    ntohs
		ntohl
				
	0x5689
	x86 win   小端序  89 56
	x86 linux 大端序  56 89 
		   网络字节序 大端序
	
	发送构造数据包   本地字节序---->网络字节序
	接收解析数据包   网络字节序---->本地字节序 
	
	192.168.1.1  点分十进制ip地址 
	unsigned int ip 	
	
	 点分十进制 转化为无符号整形;
	 int =inet_addr(str)
	 无符号整形转化为点分十进制 
	 str=inet_ntoa(uint32_t s_addr)

u32 = getsockbyname (www.baidu.com)

www.baidu.com--->ip

dns的请求包 --->8.8.8.8 --->ip



0--->stdin  read(0,buf,len)---scanf

1--->stdout write(1,buf,len)---prinf

2--->stderr 



3.使用对象
 listen(fd，100)；
 socket_fd=accpet(fd)

linux 一切皆为文件 
文件打开
open
write
read
close
llseek
ioctl
.
.
.
fd


ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen);


* UDP的点对点通信 
路由器是不转发广播数据包 
交换机会转发广播数据包  
* UDP广播数据包
广播只能在一个广播域(局域网)中传播，而不能跨网段传播 
* 组播数据包
能够在组播组中进行传播的，并且路由器可以进行组播数据转发  



* 如何识别广播包
	MAC：FF:FF:FF:FF:FF:FF 
	IP：10.0.0.0/8 --->10.255.255.255
		192.168.199.1---->192.168.199.255 


socket选项设置与读取函数 
int getsockopt(int sockfd, int level, int optname,void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname,const void *optval, socklen_t optlen);

tcpdump -i eth0 -w ./xxx.pcap

struct hostent {
       char  *h_name;            /* official name of host */
       char **h_aliases;         /* alias list */
       int    h_addrtype;        /* host address type */
       int    h_length;          /* length of address */
       char **h_addr_list;       /* list of addresses */
   }


struct ip_mreq {
     struct in_addr imr_multiaddr; /* multicast group to join */
     struct in_addr imr_interface; /* interface to join one */
}

1.创建绑定UDP
2.将当前UDP加入到组播组中


localhost 0.0.0.0  

主机进行域名解析的顺序：   DNS服务器 
/etc/hosts 
/etc/resolv.conf 

DNS--> 

ssr ss 


