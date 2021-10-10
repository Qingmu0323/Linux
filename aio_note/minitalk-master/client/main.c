
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

int	my_getnbr(char *);
int	client(int, char *);
void	error(int);

int	main(int argc, char **argv)
{
  if (argc < 3 || argv[1][0] == '\0' || argv[2][0] == '\0')
    error(0);
  else
    {
      if (client(my_getnbr(argv[1]), argv[2]) == -1)
	return (-1);
    }
  return (0);
}
