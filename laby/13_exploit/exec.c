#include <unistd.h>

int main()
{
	char *names[] = {"/bin/sh", 0};
	execve(names[0], names, names + 1);
	return 1;
}
