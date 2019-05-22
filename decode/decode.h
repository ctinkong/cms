#ifndef __EX_DECODE_H__
#define __EX_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

	void decodeWidthHeight(char *buf, int size, int *width, int *height);
	void decodeVideoFrameRate(char *buf, int size, int *frameRate);

#ifdef __cplusplus    
}
#endif

#endif
