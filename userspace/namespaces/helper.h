#include <stdlib.h>

void fatalErr(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}
