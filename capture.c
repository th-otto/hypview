#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(void)
{
	char buf[1024];
	FILE *out = fopen("post", "wb");
	int nread;
	
	while ((nread = read(0, buf, sizeof(buf))) > 0)
	{
		fwrite(buf, 1, nread, out);
	}
	fclose(out);
	return 0;
}
