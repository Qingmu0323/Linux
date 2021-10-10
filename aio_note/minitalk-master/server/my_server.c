
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "my.h"
//USER1 0 
//USER2 1
void		my_aff(int bin)
{
  static int	a;
  static int	*str;
  int		b;

  if (str == 0)
    str = malloc(sizeof(int) * 7);
  if (a < 7)
    str[a++] = bin;
  if (a == 7)
    {
      b = bintodec(str);
      my_putchar(b);
      a = 0;
    }
}

int	my_server()
{
  pid_t	pid;

  pid = getpid();
  my_putstr("Mon pid est le ");
  my_putnbr(pid);
  write(1, "\n", 1);
  while (42)
  {
     if ((signal(SIGUSR1, return1)) == SIG_ERR)
	{
	  error(3);
	  return (-1);
	}
      else if ((signal(SIGUSR2, return2)) == SIG_ERR)
	{
	  error(4);
	  return (-1);
	}
  }
  return (0);
}
