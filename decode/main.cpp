#include "decode.h"

int main()
{
	char *p = "123";
	int width;
	int height;
	decodeWidthHeight(p,3,&width,&height);
	return 0;
}