#include <sys/types.h>
#include <cstdlib>
#include <cstring>


char *
strdup(char const  *str)
{
	size_t len;
	char *copy;
	int i;

	len = strlen(str) + 1;
	if (!(copy = (char*)malloc((u_int)len)))
		return (NULL);
    for(i=0;i<len;i++)
    	copy[i]=str[i];
	return (copy);
}
