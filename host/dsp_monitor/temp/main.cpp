#include <stdio.h>
#include "xxx"

int main()
{
    FILE *f = fopen ("./xxx_unembedded", "w");
    fwrite (embed_xxx, sizeof(embed_xxx), 1, f);
    fprintf(f,"\n");
    fclose(f);
}
