#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
main(int argc, const char **argv)
{
	u_int addr;
	struct hostent *hp;
	char **p;
	char name[100];
	if (argc != 2)
	{
		printf("usage: %s IP-address\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	if ((int) (addr = inet_addr (argv[1])) == -1) 
	{
		printf("IP-address must be of the form a.b.c.d\n");
		exit (EXIT_FAILURE);
	}
	hp=gethostbyaddr((char *) &addr, sizeof (addr), AF_INET);
	memset(name,0,100);
	if (hp == NULL) 
	{
		printf("host information for %s no found \n", argv[1]);
		exit (EXIT_FAILURE);
	}
	for (p = hp->h_addr_list; *p!=0;p++)
	{
		struct sockaddr_in in;
		char **q;
		memcpy(&(in.sin_addr), *p, sizeof(in.sin_addr));
		//memcpy(name,hp->h_name,sizeof(hp->h_name));
		//printf("%s\n",inet_ntoa(in.sin_addr));
		for (q=hp->h_aliases;*q != 0; q++)
			printf("%s", *q);
		printf("\n");
	}
	exit (EXIT_SUCCESS);
}
