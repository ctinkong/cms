/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: 天空没有乌云/kisslovecsh@foxmail.com

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
#ifndef __CMS_TS_H__
#define __CMS_TS_H__
#include <ts/cms_ts_common.h>
#include <ts/cms_ts_chunk.h>
#include <common/cms_type.h>
//解码器
class CSMux
{
public:
	CSMux();
	~CSMux();

	void init();
	void release();
	int  packPES(byte *inBuf, int inLen, byte frameType, uint32 timestamp, byte **outBuf, int &outLen, uint64 &pts64);
	int	 packTS(byte *inBuf, int inLen, byte frameType, byte pusi, byte afc,
		byte *mcc, int16 pid, uint32 timestamp, byte **outBuf, int &outLen);

	int  packPES(byte *inBuf, int inLen, byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, uint64 &pts64, int pesLen);
	int  packTS(byte *inBuf, int inLen, byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, uint64 &pts64);
	int  writeTsHeader(byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, int pesLen);
	void writeES(char *inBuf, int inLen, byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, uint64 &pts64, int &pesLen);

	void packPSI();
	int  parseAAC(byte *inBuf, int inLen, byte **outBuf, int &outLen);
	int  parseNAL(byte *inBuf, int inLen, byte **outBuf, int &outLen);
	int  parseAACEx(byte *inBuf, int inLen, byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, uint64 &pts64, int &pesLen);
	int  parseNALEx(byte *inBuf, int inLen, byte frameType, uint32 timestamp, byte pusi, byte afc,
		byte *mcc, int16 pid, TsChunkArray **tca, TsChunkArray *lastTca, uint64 &pts64, int &pesLen);

	int  parseAACLen(byte *inBuf, int inLen);
	int  parseNALLen(byte *inBuf, int inLen);
	int  packPESLen(byte *inBuf, int inLen, byte frameType, uint32 timestamp);

	void reset();
	void setPcr(uint64 DTS, byte **outBuf, int &outLen);
	int  tag2ts(byte *inBuf, int inLen, byte **outBuf, int &outLen, byte &outType, uint64 &outPts);

	byte &getAmcc();
	byte &getVmcc();
	byte &getPatmcc();
	byte &getPmtmcc();
	byte *getPat();
	byte *getPmt();
private:
	bool		mheadFlag;
	bool		minfoTag;
	SHead		mhead;
	SDataInfo	mscript;
	STagInfo	mcurTag;

	byte		mSpsPpsNal[512];
	int			mSpsPpsNalLen;

	byte		mheadBuf[9];
	int			mheadBufDL;

	int			mTagSize;
	byte		mTagSizeBuf[8];
	int			mTagSizeBufDL;

	byte		*mTagBuf;
	int			mTagBufDLen;

	byte		mPAT[188];
	byte		mPMT[188];

	SAudioSpecificConfig	mAudioSpecificConfig;
	bool		mAACFLag;

	byte		mamcc;
	byte		mvmcc;
	byte		mpatmcc;
	byte		mpmtmcc;
};
#endif
