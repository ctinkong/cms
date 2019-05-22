/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: Ìì¿ÕÃ»ÓÐÎÚÔÆ/kisslovecsh@foxmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __CMS_PROTOCOL_FLV_H__
#define __CMS_PROTOCOL_FLV_H__
#include <string>

#ifndef CmsVideoTyp
#define CmsVideoTyp
#define VideoTypeAVCKey		0x17
#define VideoTypeAVC		0x07
#define VideoTypeHEVCKey	0x1A
#define VideoTypeHEVC		0x0A
#endif


#define BitGet(Number,pos) ((Number)>>(pos)&1)
std::string getAudioType(unsigned char type);
std::string getVideoType(unsigned char type);
int         getAudioSampleRates(char *tag);
int			getAudioFrameRate(char *tag, int len);
int         getAudioFrameRate(int audioSampleRates);
#endif
