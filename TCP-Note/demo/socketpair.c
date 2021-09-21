#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
void err_sys(const char *errmsg);
//父进程与子进程的通信
int main(void)
{
		int sockfd[2];
		pid_t pid;
		if ((socketpair(AF_LOCAL, SOCK_STREAM,0, sockfd))<0)
		    err_sys("socketpair");
		if ((pid = fork()) == -1)
	    	err_sys("fork");
    //子进程
		else if (pid == 0)
		{
        // use sockfd[0]
              char s[1000];
        		ssize_t n;
        	    close(sockfd[1]); //write port
        		if ((n = read(sockfd[0], s, sizeof(s))) <0)
        		err_sys("read error!\n");
        		printf("read:%s\n",s);
        		close(sockfd[0]);
        		exit(0);
		}
    //父进程
		else if (pid > 0)
		{ /* parent process */
        		char buf[] = "hello china";
        		ssize_t n;
        		close(sockfd[0]); //read port
        		if((n = write(sockfd[1], buf, sizeof(buf)))<0)
        		err_sys("write error!\n");
        		close(sockfd[1]);
        		wait(NULL);
		}
		return 0;
}
void err_sys(const char *errmsg)
{
		perror(errmsg);
		exit(1);
}