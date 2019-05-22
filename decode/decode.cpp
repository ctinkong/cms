#include "decode.h"
#include "ParseSequenceHeader.h"
#include "ParseHevcHeader.h"
#include "ParseH264Header.h"
#include <stdio.h>

void decodeWidthHeight(char *buf, int size, int *width, int *height)
{
	CParseSequenceHeader *p = new CParseHevcHeader;
	int ret = p->DecodeExtradata((uint8_t*)buf, size);
	if (ret < 0)
	{
		printf("decodeWidthHeight decode error\n");
	}
	else
	{
		*width = p->GetWidth();
		*height = p->GetHeight();
	}
	delete p;
}

void decodeVideoFrameRate(char *buf, int size, int *frameRate)
{
	CParseSequenceHeader *p = new CParseH264Header;
	int ret = p->DecodeExtradata((uint8_t*)buf, size);
	if (ret < 0)
	{
		printf("decodeVideoFrameRate decode error\n");
	}
	else
	{
		*frameRate = p->GetFps();
	}
	delete p;
}



