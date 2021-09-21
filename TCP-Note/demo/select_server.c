#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_LEN 1024
#define SERV_PORT 6000
#define FD_SIZE 100
#define MAX_BACK 100

int main( int argc, char ** argv )
{
 
   int          listenfd,connfd, sockfd, maxfd, maxi, i;
    int          nready,client[FD_SIZE];      //!> ����select����ֵ������ͻ����׽���
    int          lens;
    ssize_t   n;            //!> read�ֽ���
   fd_set       rset,allset;   //!> ��Ҫ���ɾ�ֻ�ܱ���һ������ʵfd_set�е����װ������
    char      buf[BUF_LEN];            
    socklen_t   clilen;
    struct sockaddr_in servaddr, chiaddr;
   
    if( (listenfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
       printf("Create socket Error : %d\n", errno );
       exit(EXIT_FAILURE );
    }
   
   //!>
   //!> �����ǽӿ���Ϣ
    bzero(&servaddr, sizeof( servaddr ) );
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr  =htonl( INADDR_ANY);
   servaddr.sin_port = htons( SERV_PORT );
   
   //!>
   //!> ��
    if( bind(listenfd, ( struct sockaddr * )&servaddr, sizeof(servaddr ) ) == -1 )
   {   
       printf("BindError : %d\n", errno);
      exit(EXIT_FAILURE  );
    }
   
   //!>
   //!> ����
    if( listen(listenfd, MAX_BACK ) == -1 )
    {
      printf("Listen Error : %d\n", errno );
       exit(EXIT_FAILURE );
    }
   
   //!> ��ǰ���ĸ���Ȥ���׽���fd
    maxfd =listenfd;   //!> ��ǰ��֪ͨ������fd
    maxi =-1;         //!> ������Ϊ��client����ĺô���
   
    for( i = 0;i < FD_SIZE; i++)   //!> ������Ϊȫ-1
    {
       client[i] =-1;      //!> ����client�ĵȴ���������û�еģ�����ȫ����Ϊ-1
    }
   
    FD_ZERO(&allset);      //!> �Ƚ�����Ϊ0
    FD_SET(listenfd, &allset );
               //!> ˵����ǰ�ҶԴ��׽�������Ȥ���´�select��ʱ��֪ͨ�ң�
   
    while( 1)
    {
       rset =allset;//!> ����allset����ÿ��һ��ѭ��֮���б仯������ÿ�ζ���ֵһ��
       if( (nready= select( maxfd + 1, &rset, NULL, NULL, NULL )) ==-1)
      {               //!> if ���ڹ�ע
         printf("Select Erorr : %d\n", errno );
          exit(EXIT_FAILURE );
       }
      
       if( nready<= 0)         //!> if ���еĸ���Ȥ��û�оͽ��Ż�ȥselect
       {
          continue;
       }

       if(FD_ISSET( listenfd, &rset ))         //!> if �Ǽ����ӿ��ϵġ����硱
      {                                 //!>

         // printf("server listen ...\n");
          clilen =sizeof( chiaddr );
         
         printf("Start doing... \n");
         
            if( (connfd  = accept( listenfd, (struct sockaddr*)&chiaddr, &clilen ) ) == -1)
           {                              //!> accept ���صĻ����׽���
               printf("Accept Error : %d\n", errno );
              continue;
            }
           
           
            for( i = 0;i < FD_SIZE; i++)   //!> ע��˴�������ѭ�����տ�ʼ����
                                      //!> Ϊ����ֱ������һ��end_i��ֱ�Ӵ�
                                      //!> ��,ʵ���ǲ����Եģ���Ϊÿ���׽�
           {                           //!> �ֵ��˳�ʱ���ǲ�һ���ģ������
               if(client[i] < 0)            //!> �������˳�����ô�����ˣ�����ֻ
              {                        //!> �������ˣ�
                  client[i] =connfd;         //!> ��client���������ӱ���
                  break;
               }
            }
           
            if( i ==FD_SIZE )            //!> The last one
            {
                printf( "Tomany ... " );
                close(connfd );         //!> if ������ô�Ͳ��������ˣ��رհ�
                continue;               //!> ����
            }
                                 //!> listen�����þ����������м����׽��֣�
          FD_SET(connfd, &allset);   //!> ˵�����ڶ����������Ҳ�Ǹ���Ȥ�ģ�
                                 //!> ���Լ���allset������
          if( connfd> maxfd)         //!> �������Ϊ�˽�����߰��������ģ��
                                 //!> �Ĵ���
          {
             maxfd =connfd;
          }
         
          if( i> maxi)               //!> ͬ��
          {
             maxi =i;
          }
       }

      //!> ������Ǵ������ݺ���( ��ʵ˵���ʵ�select���Ǵ��� )
       for( i = 0;i <= maxi; i++)      //!> �����е���������Ĵ���
       {
          if( ( sockfd= client[i] ) > 0)   //!> ����Ϊ�˲�����������
         {         //!> Ҳ��˵client���鲻��������ȫ��������-1�������Ǿ��״��
             if(FD_ISSET( sockfd, &rset ))   //!> if ��ǰ��������׽�����Ҫ����
              {
                 memset( buf,0, sizeof( buf ));   //!> �˲���Ҫ����Ҫ��ʱ�����
             
                 n = read(sockfd, buf, BUF_LEN);
                 if( n< 0 )
                 {
                   printf("Error!\n");
                    close(sockfd );         //!> ˵�����������˿��ϳ����ˣ�
                   FD_CLR(sockfd, &allset );
                   client[i] =-1;
                  continue;
                 }
                if( n == 0)
                {
                   printf("nodata\n");
                   close(sockfd );         //!> ˵�����������˿��϶����ˣ�
                   FD_CLR(sockfd, &allset );
                   client[i] =-1;
                  continue;
                }
               
               printf("Server Recv: %s\n", buf);
               
                if( strcmp(buf, "q" ) == 0)            //!> �ͻ������롰q���˳���־
                {
                   close(sockfd );
                   FD_CLR(sockfd, &allset );
                   client[i] =-1;
                  continue;
                }
               
               printf("Server send : %s\n", buf);
                write(sockfd, buf, n);      //!> ��������д��ȥ
             }
          }
       }
      
    }
   
    return 0;
}