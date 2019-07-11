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
#ifndef __CMS_MUX_H__
#define __CMS_MUX_H__
#include <ts/cms_ts_common.h>
#include <ts/cms_ts_chunk.h>

class CMux
{
public:
	CMux();
	~CMux();

	void init();
	void reset();
	void release();
	int  tag2Ts(byte *inBuf, int inLen, byte *outBuf, int &outLen, byte &outType, uint64 &outPts);
	int  parseAAC(byte *inBuf, int inLen, byte **outBuf, int &outLen);
	int  parseNAL(byte *inBuf, int inLen, byte **outBuf, int &outLen);
	int  packTS(byte *inBuf, int inLen, byte framType, byte pusi, byte afc, byte *mcc, int16 pid, uint32 timeStamp, byte **outBuf, int &outLen);
	void onData(TsChunkArray *tca, byte *inBuf, int inLen, byte framType, uint32 timestamp);
	void packPSI();
	byte *getPAT() { return mPAT; };
	byte *getPMT() { return mPMT; };
	void setAudioType(byte a) { mAstreamType = a; };
private:
	int  packPES(byte *inBuf, int inLen, byte framType, uint32 timeStamp, byte **outBuf, int &outLen);
	void setPcr(uint64 DTS, byte **outBuf, int &outLen);
	void tsHeadPack(byte afc, byte pusi, byte* mcc, int16 pid, byte PcrFlag, byte RadomAFlag, int StuffLen, uint32 timeStamp, byte *outBuf, int &outLen);
	void pesHeadPack(byte DTSFlag, uint64 pts, uint64 dts, int dataLen, byte *outBuf, int &outLen);
	bool		mheadFlag;  //收到FVL头
	bool		minfoTag;   //收到scriptTag
	SHead		mhead;		//头信息
	SDataInfo	mscript;	//TAG信息
	STagInfo	mcurTag;	//scriptTag信息

	//byte		mSpsPpsNal[512]; //SpsPps
	//int		mSpsPpsNalLen;       //SpsPps的长度
	byte		mSpsNal[512]; //Sps
	int			mSpsNalLen;        //Sps的长度
	byte		mPpsNal[512]; //Pps
	int			mPpsNalLen;       //Pps的长度

	byte		mheadBuf[9]; //FVL头缓存
	int			mheadBufDL;     //FVL头缓存数据长度

	int			mTagSize;
	byte		mTagSizeBuf[8];
	int			mTagSizeBufDL;

	byte		mTagBuf[65536];
	int			mTagBufDLen;

	byte mPAT[TS_CHUNK_SIZE];
	byte mPMT[TS_CHUNK_SIZE];
	byte mPCR[6];

	SAudioSpecificConfig mAudioSpecificConfig;
	bool mAACFLag;
	byte mamcc;
	byte mvmcc;
	byte mpatmcc;
	byte mpmtmcc;
	byte mAstreamType;
};
#endif
