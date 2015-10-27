#include <sys/types.h>
#ifdef _SYS_BIOS
#include <cstdlib>
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>
#include <cstring>
#endif


char *
strdup(const char *str)
{
	size_t len;
	char *copy;
	int i;

	len = strlen(str) + 1;
	if (!(copy = malloc((u_int)len)))
		return (NULL);
    for(i=0;i<len;i++)
    	copy[i]=str[i];
	return (copy);
}
