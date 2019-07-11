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
#include <ts/cms_ts.h>
#include <protocol/cms_amf0.h>
#include <ts/cms_ts_chunk.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <string>
#include <string.h>

byte gSDT[] = { 0x47, 0x40, 0x11, 0x10, 0x00, 0x42, 0xf0, 0x25, 0x00, 0x01, 0xc1, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x01, 0xfc, 0x80,
	0x14, 0x48, 0x12, 0x01, 0x06, 0x46, 0x46, 0x6d, 0x70, 0x65, 0x67, 0x09, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30,
	0x31, 0xa7, 0x79, 0xa0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
//CRC表
uint32 crc32table[] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4 };


SHead dealHead(byte *inBuf, int inLen);
void dealTagSize(byte *inBuf, int inLen, int &pTagSize, int &tagSize);
SAudioInfo dealATag(byte *inBuf, int inLen);
SVideoInfo dealVTag(byte *inBuf, int inLen);
SDataInfo dealSTag(byte *inBuf, int inLen);
void setSDT(byte **outBuf, int &outLen);
uint32 crc32S(uint8 *data, int inLen);
void setPAT(int16 pmtPid, byte **outBuf, int &outLen);
void setPMT(int16 aPid, int16 vPid, int16 pcrPid, byte AstreamType, byte **outBuf, int &outLen);

CMux::CMux()
{
	mheadFlag = false;  //收到FVL头
	minfoTag = false;   //收到scriptTag
	memset(&mhead, 0, sizeof(mhead));		//头信息
	memset(&mscript, 0, sizeof(mscript));	//TAG信息
	memset(&mcurTag, 0, sizeof(mcurTag));	//scriptTag信息

	//byte		mSpsPpsNal[512]; //SpsPps
	//int		mSpsPpsNalLen;       //SpsPps的长度
	memset(mSpsNal, 0, sizeof(mSpsNal));
	mSpsNalLen = 0;        //Sps的长度
	memset(mPpsNal, 0, sizeof(mPpsNal)); //Pps
	mPpsNalLen = 0;       //Pps的长度

	memset(mheadBuf, 0, sizeof(mheadBuf)); //FVL头缓存
	mheadBufDL = 0;     //FVL头缓存数据长度

	mTagSize = 0;
	memset(mTagSizeBuf, 0, sizeof(mTagSizeBuf));
	mTagSizeBufDL = 0;

	memset(mTagBuf, 0, sizeof(mTagBuf));
	mTagBufDLen = 0;

	memset(mPAT, 0, sizeof(mPAT));
	memset(mPMT, 0, sizeof(mPMT));

	memset(&mAudioSpecificConfig, 0, sizeof(mAudioSpecificConfig));
	mAACFLag = false;
	mamcc = 0;
	mvmcc = 0;
	mpatmcc = 0;
	mpmtmcc = 0;
	mAstreamType = 0x0f;
}

CMux::~CMux()
{
}

//初始化一些全局变量，在处理不同流的时候需要重新初始化
void CMux::init()
{
	mheadFlag = false;
	minfoTag = false;

	mheadBufDL = 0;
	mTagSize = -1;
	mTagSizeBufDL = 0;
	mTagBufDLen = 0;

	mamcc = 0;
	mvmcc = 0;
	mpatmcc = 0;
	mpmtmcc = 0;
	mAstreamType = 0x0f;

}

//流断了重新接流
void CMux::reset()
{
	mheadFlag = false;
	minfoTag = false;

	mheadBufDL = 0;
	mTagSize = -1;
	mTagSizeBufDL = 0;
	mTagBufDLen = 0;

	mamcc = 0;
	mvmcc = 0;
	mpatmcc = 0;
	mpmtmcc = 0;
	mAstreamType = 0x0f;
}

//释放一些全局变量
void CMux::release() {
	mheadFlag = false;
	minfoTag = false;

	mheadBufDL = 0;
	mTagSize = -1;
	mTagSizeBufDL = 0;
	mTagBufDLen = 0;

	mamcc = 0;
	mvmcc = 0;
	mpatmcc = 0;
	mpmtmcc = 0;
	mAstreamType = 0x0f;
}



//送入完整的Tag并且解出数据再打包成TS
int CMux::tag2Ts(byte *inBuf, int inLen, byte *outBuf, int &outLen, byte &outType, uint64 &outPts)
{
	byte *PESbuf = NULL;
	int PESLen = 0;

	byte *TSbuf = NULL;
	int TSlen = -1;

	byte *&TsStream = outBuf;
	TsStream = new byte[int(inLen / 184 + 2) * TS_CHUNK_SIZE + TS_CHUNK_SIZE * 2];
	int &StreamLen = outLen;
	outPts = 0;
	outType = 'A';
	byte &framType = outType;
	//curTag.head
	mcurTag.head.tagType = inBuf[0];
	mcurTag.head.dataSize = (int)(inBuf[1]) << 16 | (int)(inBuf[2]) << 8 | (int)(inBuf[3]);
	mcurTag.head.timeStamp = (uint32)(inBuf[4]) << 16 | (uint32)(inBuf[5]) << 8 | (uint32)(inBuf[6]) | (uint32)(inBuf[7]) << 24; // |2|3|4|1|

	if (mcurTag.head.timeStamp > 47721858)
	{ //0xFFFFFFFF ÷ 90
		mcurTag.head.timeStamp = mcurTag.head.timeStamp - 47721858;
	}
	mcurTag.head.streamId = (int)(inBuf[8]) << 16 | (int)(inBuf[9]) << 8 | (int)(inBuf[10]);
	if (inBuf[0] == 0x09 && (inBuf[11] & 0x0f) != 7)
	{
		logs->debug("ERROR[TagToTs] Not H264 code!!!!");
	}

	if (mcurTag.head.tagType == 0x08)
	{
		//音频Tag
		mcurTag.flag = 'a';
		mcurTag.audio = dealATag(inBuf + 11, 1);
		mcurTag.head.deviation = 0;
		framType = 'A';
		packPES(inBuf + 11, inLen - 11, framType, mcurTag.head.timeStamp, &PESbuf, PESLen);
		if (PESLen > 0)
		{
			packTS(PESbuf, PESLen, framType, 0x01, 0x01, &mamcc, Apid, mcurTag.head.timeStamp, &TSbuf, TSlen);
			memcpy(TsStream + StreamLen, TSbuf, TSlen);
			StreamLen += TSlen;
		}
		else
		{
			logs->error("ts error packPES file %s line %d", __FILE__, __LINE__);
		}
	}
	else if (mcurTag.head.tagType == 0x09 && true) { //视频Tag

		mcurTag.flag = 'V';
		mcurTag.video = dealVTag(inBuf + 11, inLen - 11);
		mcurTag.head.deviation = (int)(inBuf[13]) << 16 | (int)(inBuf[14]) << 8 | (int)(inBuf[15]);//偏移量cts
		framType = 'P';
		if (mcurTag.flag == 'V' && mcurTag.video.framType == 1 && mcurTag.video.codeId == 7) {
			//关键帧
			framType = 'I';
			packPES(inBuf + 11, inLen - 11, framType, mcurTag.head.timeStamp, &PESbuf, PESLen);
			if (PESLen > 0) {
				packPSI();
				//CallBack(PAT[:], TS_CHUNK_SIZE)
				//CallBack(PMT[:], TS_CHUNK_SIZE)
				packTS(PESbuf, PESLen, framType, 0x01, 0x01, &mvmcc, Vpid, mcurTag.head.timeStamp, &TSbuf, TSlen);
				memcpy(TsStream + StreamLen, mPAT, sizeof(mPAT));
				StreamLen += TS_CHUNK_SIZE;
				memcpy(TsStream + StreamLen, mPMT, sizeof(mPMT));
				StreamLen += TS_CHUNK_SIZE;
				memcpy(TsStream + StreamLen, TSbuf, TSlen);
				StreamLen += TSlen;
				//CallBack(TSbuf, TSlen) //送入回调函数

			}
			else
			{
				logs->error("ts error packPES file %s line %d", __FILE__, __LINE__);
			}
		}
		else {
			packPES(inBuf + 11, inLen - 11, framType, mcurTag.head.timeStamp, &PESbuf, PESLen);
			if (PESLen > 0)
			{
				packTS(PESbuf, PESLen, framType, 0x01, 0x01, &mvmcc, Vpid, mcurTag.head.timeStamp, &TSbuf, TSlen);
				memcpy(TsStream + StreamLen, TSbuf, TSlen);
				StreamLen += TSlen;
				//CallBack(TSbuf, TSlen) //送入回调函数
			}
			else
			{
				logs->error("ts error packPES file %s line %d", __FILE__, __LINE__);
			}
		}

	}
	else if (mcurTag.head.tagType == 0x12) { //scriptTag
		mscript = dealSTag(inBuf + 11, mcurTag.head.dataSize);
		//packPSI()
		byte *SDT = NULL;
		int sdtLen = 0;
		setSDT(&SDT, sdtLen);
		memcpy(TsStream + StreamLen, SDT, sdtLen);
		StreamLen += TS_CHUNK_SIZE;
		//CallBack(SDT[:], TS_CHUNK_SIZE)
		/*CallBack(PAT[:], TS_CHUNK_SIZE)
		CallBack(PMT[:], TS_CHUNK_SIZE)*/
		minfoTag = true;
		return 0;
	}
	outPts = uint64(mcurTag.head.timeStamp) * 90 + uint64(mcurTag.head.deviation) * 90;
	return 0;

}

//处理FLV头信息
SHead dealHead(byte *inBuf, int inLen) {
	SHead head = { 0 };
	head.version = inBuf[3];
	head.streamInfo = inBuf[4];
	head.lenght = (int)(inBuf[5]) << 24 | (int)(inBuf[6]) << 16 | (int)(inBuf[7]) << 8 | (int)(inBuf[8]);
	return head;
}

//处理TagSize
void dealTagSize(byte *inBuf, int inLen, int &pTagSize, int &tagSize) {
	pTagSize = (int)(inBuf[0]) << 24 | (int)(inBuf[1]) << 16 | (int)(inBuf[2]) << 8 | (int)(inBuf[3]);
	tagSize = (int)(inBuf[5]) << 16 | (int)(inBuf[6]) << 8 | (int)(inBuf[7]);
	tagSize += 11;
	return;
}

//处理audioTag
SAudioInfo dealATag(byte *inBuf, int inLen) {
	SAudioInfo aInfo = { 0 };
	aInfo.codeType = inBuf[0] >> 4;
	aInfo.rate = (inBuf[0] << 4) >> 6;
	aInfo.precision = (inBuf[0] << 6) >> 7;
	aInfo.audioType = (inBuf[0] << 7) >> 7;
	return aInfo;
}

//处理videoTag
SVideoInfo dealVTag(byte *inBuf, int inLen) {
	SVideoInfo vInfo = { 0 };
	vInfo.framType = inBuf[0] >> 4;
	vInfo.codeId = (inBuf[0] << 4) >> 4;

	return vInfo;
}

//处理scriptTag
SDataInfo dealSTag(byte *inBuf, int inLen) {
	//fmt.Println("dealSTag")
	//跳过第一个AMF,一般第一个AMF为固定信息
	SDataInfo dInfo = { 0 };
	byte amfType = inBuf[13];//
	if (amfType == 1) {
		//示例，无意义
	}
	int amfSize = (int)(inBuf[14]) << 24 | (int)(inBuf[15]) << 16 | (int)(inBuf[16]) << 8 | (int)(inBuf[17]);

	float temp = 0;
	amf0::Amf0Block *block = NULL;
	if (inLen - 15 >= amfSize)
	{
		block = amf0::amf0Parse((char *)(inBuf + 15), amfSize);
	}
	if (block != NULL)
	{
		std::string strValue;
		amf0::amf0Block5Value(block, "duration", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.duration = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "width", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.width = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "height", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.height = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "videodatarate", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.videodatarate = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "framerate", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.framerate = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "videocodecid", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.videocodecid = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "audiosamplerate", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.audiosamplerate = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "audiosamplesize", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.audiosamplesize = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "stereo", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.stereo = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "audiocodecid", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.audiocodecid = int(temp);
		}

		strValue = "";
		amf0::amf0Block5Value(block, "filesize", strValue);
		if (!strValue.empty())
		{
			temp = str2float(strValue.c_str());
			dInfo.filesize = int(temp);
		}

		amf0::amf0BlockRelease(block);
	}

	return dInfo;
}

//将一个Tag数据先打成PES包
int CMux::packPES(byte *inBuf, int inLen, byte framType, uint32 timeStamp, byte **outBuf, int &outLen) {
	*outBuf = new byte[inLen + 1024];
	outLen = 0;
	byte *&pespack = *outBuf;
	int headlen = 0; //整个头部长度
	int  packLen = 0;     //整个包长度
	uint64 pts = 0;
	uint64 dts = 0;

	uint64 cts = (uint64)(inBuf[2]) << 16 | (uint64)(inBuf[3]) << 8 | (uint64)(inBuf[4]); //偏移量cts
	if (framType == 'A') {
		cts = 0;
	}
	dts = (uint64)(timeStamp) * 90; //flv中记录的时间戳是DTS(TS中的时间是FLV中的1/90)
	pts = dts + cts * 90;

	if (pts < dts) {
		pts = dts;
		logs->debug("[PackTS] ERROR timeStamp:  pts < dts");
	}

	//包头由起始码前缀、数据流识别及PES包长信息3部分构成。包起始码前缀是用23个连续“0”和1个“1”构成的
	//3 0x000001 3字节的前缀
	//1	0xe0-0xef:v 0xc0-0xef:a
	//2 包长
	byte head[1024];
	head[0] = 0;
	head[1] = 0;
	head[2] = 1;
	head[3] = 0xE0; //pos= 3 原始流的类型和数目，取值从1100到1111之间
	//数据流识别, 数据流（视频，音频，或其它）
	head[4] = 0x00; //pos= 4 表示从此字节之后PES包长(单位字节)。
	head[5] = 0x03;
	headlen += 6;

	//fmt.Printf("[PackPES] timeStamp CTS dts PTS %d %d %d %d\n", curTag.head.timeStamp, curTag.head.deviation, dts, pts)

	byte PDflags = 0;
	//类型数值未确定
	if (framType == 'I') {
		//I帧
		PDflags = 0x03;
	}
	else if (framType == 'P') {
		//P帧
		PDflags = 0x03;
	}
	else if (framType == 'B') {
		//B帧
		PDflags = 0x03;
	}
	else if (framType == 'A') {
		//音频
		head[3] = 0xC0;
		PDflags = 0x02; //音频没有DTS
	}
	//包头识别标志由12个部分
	//head[6] = 0x80 //10000000
	byte fixBit = 0x02;//2 填充字节
	byte PESscramblingControl = 0; //2 PES有效负载的加密模式。0表示不加密，其余表示用户自定义。
	byte PESpriority = 0;           //1 PES数据包的优先级
	byte dataAlignmentIndicator = 0; //1 为时，表明此分组头部之后紧跟着数据流描述子中定义的访问单元类型
	byte copyright = 0;           //1 版权，表示有版权
	byte originalOrCopy = 0;       //1 pos= 6 1表示原始数据，0表示备份
	head[6] = fixBit << 6 | PESscramblingControl << 4 | PESpriority << 3 | dataAlignmentIndicator << 2 | copyright << 1 | originalOrCopy;

	byte PTSDTSflags = PDflags;     //2 10表示含有PTS字段，11表示含有PTS和DTS字段，00表示不含有pts_dts和DTS。
	byte ESCRflag = 0;         //1 1表示ESCR在PES首部出现，0表示不出现
	byte ESrateFlag = 0;      //1 1表示PES分组含有ES_rate字段。0表示不含有。
	byte DSMtrickModeFlag = 0;    //1 1表示有位的trick_mode_flag字段，0表示不出现此字段。只对DSM有效。
	byte additionalCopyInfoFlag = 0;//1 1表示有copy_info_flag字段，0表示不出现此字段。
	byte PESCRCflag = 0;           //1 1表示PES分组中有CRC字段，0表示不出现此字段。
	byte PESextensionFlag = 0;      //1 pos= 7 1表示扩展字段在PES包头存在，0表示扩展字段不存在

	head[7] = PTSDTSflags << 6 | ESCRflag << 5 | ESrateFlag << 4 | DSMtrickModeFlag << 3 | additionalCopyInfoFlag << 2 | PESCRCflag << 1 | PESextensionFlag;
	head[8] = 0; // PESheaderDataLength = 0;         //表示可选字段和填充字段所占的字节数。

	headlen += 3;

	if ((PTSDTSflags & 0x02) > 0) {
		byte PTSbuf[5];
		//注意前四位的填充
		PTSbuf[0] = PTSDTSflags << 4 | (byte)((pts & 0x1C0000000) >> 29) | 0x01; //pts&0x111000000 00000000 00000000 00000000
		PTSbuf[1] = (byte)((pts & 0x3fc00000) >> 22);                    //pts&0x 00111111 11000000 00000000 00000000
		PTSbuf[2] = (byte)((pts & 0x3f8000) >> 14) | 0x01;                   //pts&0x 00000000 00111111 10000000 00000000
		PTSbuf[3] = (byte)((pts & 0x7f80) >> 7);                         //pts&0x 00000000 00000000 01111111 10000000
		PTSbuf[4] = (byte)(pts << 1) | 0x01;                             //pts&0x 00000000 00000000 00000000 01111111

		head[8] += 5;
		memcpy(head + headlen, PTSbuf, sizeof(PTSbuf));
		headlen += 5;
		/*pts
		 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
		|0|0|1|1|.|.|.|1|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|1|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|1|
				pts1     pts2                            pts3
		*/
	}
	if ((PTSDTSflags & 0x01) > 0) {
		byte DTSbuf[5];
		DTSbuf[0] = 0x01 << 4 | (byte)((dts & 0x1C0000000) >> 29) | 0x01;
		DTSbuf[1] = (byte)((dts & 0x3fc00000) >> 22);
		DTSbuf[2] = (byte)((dts & 0x3f8000) >> 14) | 0x01;
		DTSbuf[3] = (byte)((dts & 0x7f80) >> 7);
		DTSbuf[4] = (byte)(dts << 1) | 0x01;

		head[8] += 5;
		memcpy(head + headlen, DTSbuf, sizeof(DTSbuf));
		headlen += 5;
		/*dts
		 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
		|0|0|0|1|.|.|.|1|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|1|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|1|
				 dts1    dts2                            dts3
		*/
	}

	if (ESCRflag > 0) {

	}

	if (ESrateFlag > 0) {

	}

	if (ESrateFlag > 0) {

	}

	if (DSMtrickModeFlag > 0) {

	}

	if (additionalCopyInfoFlag > 0) {

	}

	if (PESextensionFlag > 0) {
		//这里其实还有一层
	}

	if (framType == 'I') { //H264视频码流
		byte *NALbuf = NULL;
		int NALlen = 0;
		parseNAL(inBuf, inLen, &NALbuf, NALlen);
		if (NALlen <= 0) { //为SPS/PPS的Tag，不单独写成PES包(放在I帧前和I帧一起打包)
// 			logs->error("ts error parseNAL file %s line %d", __FILE__, __LINE__);
			delete[] * outBuf;
			outLen = 0;
			return 0;
		}

		packLen = headlen + mSpsNalLen + mPpsNalLen + 6 + NALlen;
		head[4] = 0;// byte((packLen - 6) >> 8)
		head[5] = 0; // byte(packLen - 6)

		memcpy(pespack, head, headlen);

		pespack[headlen] = 0;
		pespack[headlen + 1] = 0;
		pespack[headlen + 2] = 0;
		pespack[headlen + 3] = 1;
		pespack[headlen + 4] = 9;
		pespack[headlen + 5] = 0xf0;                              //添加slice前加上0x00 0x00 0x00 0x01 0x09 0xf0 起始码
		memcpy(pespack + headlen + 6, mSpsNal, mSpsNalLen);//添加I帧前添加一次Sps
		memcpy(pespack + headlen + 6 + mSpsNalLen, mPpsNal, mPpsNalLen); //添加I帧前添加一次Pps
		memcpy(pespack + headlen + mSpsNalLen + mPpsNalLen + 6, NALbuf, NALlen);

	}
	else if (framType == 'A') {

		if (inBuf[0] >> 4 == 10) { //AAC
			mAstreamType = 0x0f;
			byte *AACbuf = NULL;
			int AAClen = 0;
			parseAAC(inBuf, inLen, &AACbuf, AAClen);
			if (AAClen <= 0)
			{
				// 				logs->error("ts error parseAAC file %s line %d", __FILE__, __LINE__);
				delete[] * outBuf;
				outLen = 0;
				return 0;
			}
			packLen = headlen + AAClen;
			head[4] = (byte)((packLen - 6) >> 8);
			head[5] = (byte)(packLen - 6);
			memcpy(pespack, head, headlen);
			memcpy(pespack + headlen, AACbuf, AAClen);
		}
		else if (inBuf[0] >> 4 == 2) { //Mp3
			mAstreamType = 0x03; //音频编码类型
			packLen = headlen + inLen - 1;
			head[4] = (byte)((packLen - 6) >> 8); //去掉6位头
			head[5] = (byte)(packLen - 6);
			memcpy(pespack, head, headlen);
			memcpy(pespack + headlen, inBuf + 1, inLen - 1);

		}
		else { //非AAC目前做简单处理
		 //fmt.Println("[PackPES] Audio Is Not AAC")
			packLen = headlen + inLen - 1;
			head[4] = (byte)((packLen - 6) >> 8);//去掉6位头
			head[5] = (byte)(packLen - 6);
			memcpy(pespack, head, headlen);
			memcpy(pespack + headlen, inBuf + 1, inLen - 1);
		}
	}
	else {
		byte *NALbuf = NULL;
		int  NALlen = 0;
		parseNAL(inBuf, inLen, &NALbuf, NALlen);
		if (NALlen <= 0)
		{
			// 			logs->error("ts error parseNAL file %s line %d", __FILE__, __LINE__);
			delete[] * outBuf;
			outLen = 0;
			return 0;
		}
		packLen = headlen + NALlen + 6;
		head[4] = 0; // byte((packLen - 6) >> 8)
		head[5] = 0; // byte(packLen - 6)

		memcpy(pespack, head, headlen);
		pespack[headlen] = 0;
		pespack[headlen + 1] = 0;
		pespack[headlen + 2] = 0;
		pespack[headlen + 3] = 1;
		pespack[headlen + 4] = 9;
		pespack[headlen + 5] = 0xf0; //添加slice前加上0x00 0x00 0x00 0x01 0x09 0xf0 起始码
		memcpy(pespack + headlen + 6, NALbuf, NALlen);
	}
	return 0;
}

//
int CMux::parseAAC(byte *inBuf, int inLen, byte **out, int &outLen) {
	*out = new byte[inLen + 128];
	memset(*out, 0, inLen + 128);
	byte *&outBuf = *out;
	outLen = 0;
	if (inBuf[1] == 0x00) { //AACPacketType为0x00说明是AAC sequence header
		mAudioSpecificConfig.ObjectType = (inBuf[2] >> 3);
		mAudioSpecificConfig.SamplerateIndex = (inBuf[2] & 0x07) << 1 | inBuf[3] >> 7;
		mAudioSpecificConfig.Channels = (inBuf[3] << 1) >> 4;
		mAudioSpecificConfig.FramLengthFlag = (inBuf[3] & 4) >> 2;
		mAudioSpecificConfig.DependOnCCoder = (inBuf[3] & 2) >> 1;
		mAudioSpecificConfig.ExtensionFlag = inBuf[3] & 1;

		if (mAudioSpecificConfig.ExtensionFlag > 0) {

		}
		mAACFLag = false;
		delete[]outBuf;
		return 0;
	}
	else {
		int aacFrameLength = inLen - 2 + 7; //这里是带上ADTS头7字节的全帧长度，不是帧数据长度
		outBuf[0] = 0xFF;
		outBuf[1] = 0xF1;
		outBuf[2] = ((mAudioSpecificConfig.ObjectType - 1) << 6) |
			((mAudioSpecificConfig.SamplerateIndex & 0x0F) << 2) |
			(mAudioSpecificConfig.Channels >> 7);
		outBuf[3] = (mAudioSpecificConfig.Channels << 6) | (byte)((aacFrameLength & 0x1800) >> 11);
		outBuf[4] = (byte)((aacFrameLength & 0x7f8) >> 3);
		outBuf[5] = (byte)((aacFrameLength & 0x7) << 5) | 0x1f;
		outBuf[6] = 0xFC | 0x00;
		/*
			//adts_fixed_header
				syncword 				//12 同步头 总是0xFFF, all bits must be 1，代表着一个ADTS帧的开始
				ID						//1 MPEG Version: 0 for MPEG-4, 1 for MPEG-2
				Layer					//2 always: '00'
				protectionAbsent		//1
				profile					//2 表示使用哪个级别的AAC，有些芯片只支持AAC LC 。在MPEG-2 AAC中定义了3种：
				samplingFrequencyIndex 	//4 表示使用的采样率下标
				privateBit				//1
				channelConfiguration	//3
				originalCopy			//1
				home					//1

			//adts_variable_header
				copyrightIBit			//1
				copyrightIStart			//1
				frameLength	 			//13 一个ADTS帧的长度包括ADTS头和AAC原始流.
				adtsBufferFullness 	    //11 0x7FF 说明是码率可变的码流
				NumOfRawDataBlockInFrame//2*/

		memcpy(outBuf + 7, inBuf + 2, inLen - 2);
		outLen = inLen + 5;
		return 0;
	}
}

//解FLV中H264的NAL
int CMux::parseNAL(byte *inBuf, int inLen, byte **out, int &outLen) {
	*out = new byte[inLen + 256];
	memset(*out, 0, inLen + 256);
	byte *&outBuf = *out;
	outLen = 0;
	int NALlen = 0;
	byte curNALtype = 0;

	//var NALtype byte = 0
	if (inBuf[1] == 0x00) { //AVCPacketType为0x00说明是SPS/PPS
		int pos = 0;
		mSpsNalLen = 0;
		mPpsNalLen = 0;
		/*configurationVersion := inBuf[5]
		AVCProfileIndication := inBuf[6]
		profile_compatibility := inBuf[7]
		AVCLevelIndication := inBuf[8]
		lengthSizeMinusOne := inBuf[9]*/                                 //<- 非常重要，是 H.264 视频中 NALU 的长度，计算方法是 1 + (lengthSizeMinusOne & 3)
		int numOfSequenceParameterSets = (int)(inBuf[10] & 0x1F);              //<- SPS 的个数，计算方法是 numOfSequenceParameterSets & 0x1F
		int sequenceParameterSetLength = (int)(inBuf[11]) << 8 | (int)(inBuf[12]); // <- SPS 的长度

		for (int i = 0; i < numOfSequenceParameterSets; i++) {
			mSpsNal[mSpsNalLen] = 0x00;
			mSpsNal[mSpsNalLen + 1] = 0x00;
			mSpsNal[mSpsNalLen + 2] = 0x00;
			mSpsNal[mSpsNalLen + 3] = 0x01;
			memcpy(mSpsNal + mSpsNalLen + 4, inBuf + 13 + i * sequenceParameterSetLength, sequenceParameterSetLength);
			mSpsNalLen += (4 + sequenceParameterSetLength);
		}

		pos = 13 + sequenceParameterSetLength * numOfSequenceParameterSets;

		int numOfPictureParameterSets = (int)(inBuf[pos]);                        //<- PPS 的个数
		int pictureParameterSetLength = (int)(inBuf[pos + 1]) << 8 | (int)(inBuf[pos + 2]); //<- PPS 的长度
		for (int i = 0; i < numOfPictureParameterSets; i++) {
			mPpsNal[mPpsNalLen] = 0x00;
			mPpsNal[mPpsNalLen + 1] = 0x00;
			mPpsNal[mPpsNalLen + 2] = 0x00;
			mPpsNal[mPpsNalLen + 3] = 0x01;
			memcpy(mPpsNal + mPpsNalLen + 4, inBuf + pos + 3 + i * pictureParameterSetLength, pictureParameterSetLength);
			mPpsNalLen += (4 + pictureParameterSetLength);
		}
		delete[]outBuf;
		return 0;
	}
	else if (inBuf[1] == 0x01) { //AVCPacketType为slice
		for (int i = 5; i < inLen - 4;) {
			NALlen = (int)(inBuf[i]) << 24 | (int)(inBuf[i + 1]) << 16 | (int)(inBuf[i + 2]) << 8 | (int)(inBuf[i + 3]);
			if (NALlen < 0) {
				logs->error("[slice][ParserNAL] parse NALlen ERROR: %d", NALlen);

				break;
			}
			curNALtype = inBuf[i + 4] & 0x1f;
			if (curNALtype > 0 && curNALtype < 7 /*&& curNALtype != 6*/) {
				if (NALlen + i + 4 > inLen) {
					NALlen = inLen - 4 - i;
				}

				outBuf[outLen] = 0x00;
				outBuf[outLen + 1] = 0x00;
				outBuf[outLen + 2] = 0x01;

				if (NALlen > 655356) {
					logs->error("[slice][ParserNAL] A BIG NAL outLen NALlen  curNALtype [%d %d %d]\n", outLen, NALlen, curNALtype);
					logs->error("[slice][ParserNAL] inLen NALlen inBuf[i-5] inBuf[i-4] inBuf[i-3] inBuf[i-2] inBuf[i-1] inBuf[i] inBuf[i+1] %d %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
						inLen, NALlen, inBuf[i - 5], inBuf[i - 4], inBuf[i - 3], inBuf[i - 2], inBuf[i - 1], inBuf[i], inBuf[i + 1]);
				}
				memcpy(outBuf + outLen + 3, inBuf + i + 4, NALlen);
				outLen += (NALlen + 4);
			}
			else {
				if (curNALtype > 6 && curNALtype < 13) {

				}
				else {
					logs->error("[slice][ParserNAL] unusual NAL curNALtype %d", curNALtype);
				}
				if (curNALtype == 7) {
					mSpsNalLen = 0;
					mSpsNal[mSpsNalLen] = 0x00;
					mSpsNal[mSpsNalLen + 1] = 0x00;
					mSpsNal[mSpsNalLen + 2] = 0x00;
					mSpsNal[mSpsNalLen + 3] = 0x01;
					memcpy(mSpsNal + mSpsNalLen + 4, inBuf + i + 4, NALlen);
					mSpsNalLen += (4 + NALlen);
				}
				else if (curNALtype == 8) {
					mPpsNalLen = 0;
					mPpsNal[mPpsNalLen] = 0x00;
					mPpsNal[mPpsNalLen + 1] = 0x00;
					mPpsNal[mPpsNalLen + 2] = 0x00;
					mPpsNal[mPpsNalLen + 3] = 0x01;
					memcpy(mPpsNal + mPpsNalLen + 4, inBuf + i + 4, NALlen);
					mPpsNalLen += (4 + NALlen);

				}
			}
			i += (NALlen + 4);
		}
		return 0;
	}
	delete[]outBuf;
	return 0;
	/*
	 FLV中的NAL和TS中的NAl不一样，TS中是由 0x00 0x00 0x00 0x01分割
	*/
	/*
			 Nalu_type:
				0x67 (0 11 00111) SPS  	非常重要       	type = 7
				0x68 (0 11 01000) PPS  	非常重要       	type = 8
				0x65 (0 11 00101) IDR帧 	关键帧  非常重要 	type = 5
				0x41 (0 10 00001) P帧   	重要         	type = 1
				0x01 (0 00 00001) B帧   	不重要        	type = 1
				0x06 (0 00 00110) SEI   不重要        	type = 6
	*/

}

//将一个PES包打成TS包(不包含PSI)
int CMux::packTS(byte *inBuf, int inLen, byte framType, byte pusi, byte afc, byte *mcc, int16 pid, uint32 timeStamp, byte **outBuf, int &outLen) {
	int max = int(inLen / 184 + 1) * TS_CHUNK_SIZE + TS_CHUNK_SIZE * 2;
	*outBuf = new byte[max];
	outLen = 0;
	byte *&TSstream = *outBuf;
	int &streamLen = outLen;
	bool StuffFlag = false;
	int StuffLen = 0;

	// 	if len(inBuf) < inLen {
	// 		fmt.Println("[PackTS] ERROR inLen :len(inBuf),inLen ", len(inBuf), inLen)
	// 		inLen = len(inBuf)
	// 	}

	uint64 DTS = uint64(timeStamp) * 90; //flv中记录的时间戳是DTS(TS中的时间是FLV中的1/90)

	int dealPos = 0;
	if (inLen > 65536 * 4) {
		logs->debug("[slice][PackTS] too long inLen");
	}
	//除去PSI的打包
	for (dealPos = 0; dealPos < inLen;) {
		byte TSpack[TS_CHUNK_SIZE];
		int headlen = 0;
		byte head[4];
		byte adaptionFiled[184];
		int adaptationFiledLength = 0;

		byte synByte = 0x47;         //同步字节, 固定为,表示后面的是一个TS分组
		byte transportErrorIndicator = 0;     //传输误码指示符
		byte payloadUnitStartIndicator = pusi; //有效荷载单元起始指示符, 1标明为负载起始包
		byte transport_priority = 0;           //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
		int16 PID = pid;                     //PID = 0x100

		byte transportScramblingControl = 0; //传输加扰控制(加密)
		byte adaptionFieldControl = afc;     //自适应控制 01仅含有效负载，10仅含调整字段，11都有
		byte continuityCounter = *mcc;      //连续计数器 一个4bit的计数器，范围0-15
		head[0] = synByte;
		head[1] = transportErrorIndicator << 7 | payloadUnitStartIndicator << 6 | transport_priority << 5 | (byte)((PID << 3) >> 11);
		head[2] = (byte)(PID);
		head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
		headlen += 4;

		if (inLen - dealPos < 182 && pusi == 1) { //数据不够，需要填充满188字节
			adaptionFieldControl = 0x03;
			head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
			StuffFlag = true;
		}
		else if (inLen - dealPos < 184) {
			if (pusi == 0) {
				adaptionFieldControl = 0x03;
				head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
				StuffFlag = true;
			}
			else if (pusi == 1 && framType != 'I' && framType != 'A') {
				adaptionFieldControl = 0x03;
				head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
				StuffFlag = true;
			}
		}
		if ((framType == 'I' || framType == 'A') && pusi == 1) { //包含原始流节点
			adaptionFieldControl = 0x03;
			head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
		}
		if (adaptionFieldControl > 1) {
			byte discontinuityIndicator = 0; //不连续状态指示
			byte randomAccessIndicator = 0; //
			if ((framType == 'I' || framType == 'A') && pusi == 1) {
				randomAccessIndicator = 1;
			}
			byte elementaryStreamPriorityIndicator = 0; //为1当前分组有较高优先级,有多个负载在会用到（可以不考虑）
			byte pcrFlag = 0;                      //pcr标志   pes包的前面 （待定）
			if (pusi == 1 && framType == 'I') {
				pcrFlag = 1;
			}
			byte opcrFlag = 0;               //（待定）
			byte splicingPointFlag = 0;     //拼接点（貌似没用）
			byte transportPrivateDataFlag = 0;//私用数据字节（可以不考虑）
			byte adaptationFiledExtension = 0;//扩展自适应字段
			adaptionFiled[1] = discontinuityIndicator << 7 | randomAccessIndicator << 6 | elementaryStreamPriorityIndicator << 5 | pcrFlag << 4 | opcrFlag << 3 | splicingPointFlag << 2 | transportPrivateDataFlag << 1 | adaptationFiledExtension;
			adaptationFiledLength += 2;
			if (pcrFlag > 0) {
				byte *pcrBuf = NULL;
				int pcrLen = 0;
				setPcr(DTS, &pcrBuf, pcrLen);
				memcpy(adaptionFiled + adaptationFiledLength, pcrBuf, pcrLen);
				adaptationFiledLength += 6;
				// 48 bit
			}
			if (opcrFlag > 0) {
				// 48 bit
			}
			if (splicingPointFlag > 0) {
				// 8 bit
			}
			if (transportPrivateDataFlag > 0) {
				byte transportPrivateDataLen = 0; // 8 bit
				if (transportPrivateDataLen > 0) {
					transportPrivateDataLen += 0;
				}

			}
			if (adaptationFiledExtension > 0) {
				//这里也还有一层
				byte adaptationExFiledLength = 0;
				if (false) {
					adaptationExFiledLength++;
				}
			}

			if (StuffFlag) { //需要填充
				StuffLen = 184 - (inLen - dealPos) - 2;
				for (int i = adaptationFiledLength; StuffLen > 0 && i < StuffLen + adaptationFiledLength; i++) {
					adaptionFiled[i] = 0xFF;
				}
				adaptationFiledLength += StuffLen;
			}
			adaptionFiled[0] = (byte)(adaptationFiledLength - 1);
			memcpy(TSpack + 4, adaptionFiled, adaptationFiledLength);
			headlen += int(adaptationFiledLength);
		}
		*mcc = (*mcc + 1) % 16;
		memcpy(TSpack, head, sizeof(head));
		if (dealPos + TS_CHUNK_SIZE - headlen <= inLen) {
			memcpy(TSpack + headlen, inBuf + dealPos, TS_CHUNK_SIZE - headlen);
		}
		else {
			//fmt.Printf(" [PackTS]dealPos+TS_CHUNK_SIZE-headlen, inLen ", dealPos+TS_CHUNK_SIZE-headlen, inLen)
			memcpy(TSpack + headlen, inBuf + dealPos, cmsMin(inLen - dealPos, TS_CHUNK_SIZE - headlen));
		}

		if (streamLen + TS_CHUNK_SIZE > max) {
			logs->error("[slice][PackTS] too big [max, inLen, streamLen+TS_CHUNK_SIZE] ", max, inLen, streamLen + TS_CHUNK_SIZE);
		}
		dealPos = dealPos + TS_CHUNK_SIZE - headlen;
		memcpy(TSstream + streamLen, TSpack, sizeof(TSpack));
		streamLen = streamLen + TS_CHUNK_SIZE;
		pusi = 0;
	}

	return 0;
}

/*设置PCR*/
void CMux::setPcr(uint64 DTS, byte **outBuf, int &outLen) {
	uint64 pcr = DTS;
	uint64 pcrExt = pcr >> 33;
	mPCR[0] = (byte)(pcr >> 25 & 0xff);
	mPCR[1] = (byte)(pcr >> 17 & 0xff);
	mPCR[2] = (byte)(pcr >> 9 & 0xff);
	mPCR[3] = (byte)(pcr >> 1 & 0xff);

	byte PCRbase2 = (byte)(pcr & 0x01);
	byte PCRExt1 = (byte)(pcrExt >> 8 & 0x01);
	byte PCRExt2 = (byte)(pcrExt & 0xff);

	mPCR[4] = PCRbase2 << 7 | 0 | PCRExt1;
	mPCR[5] = PCRExt2;
	*outBuf = mPCR;
	outLen = sizeof(mPCR);
	return;
}

//打包PSI，这里只打包PAT和PMT表
void CMux::packPSI() {
	int PATlen = 0;
	int PMTlen = 0;
	byte *tPAT = NULL;
	byte *tPMT = NULL;
	//PAT
	byte head[5] = { 0 };

	/*byte synByte  = 0x47 ;              //同步字节, 固定为0x47,表示后面的是一个TS分组
	byte transportErrorIndicator  = 0 ;   //传输误码指示符
	byte payloadUnitStartIndicator  = 1 ; //有效荷载单元起始指示符, 1标明为负载起始包
	byte transport_priority  = 0 ;        //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
	int16 PID  = PATpid        ;          //PID
	byte transportScramblingControl  = 0; //传输加扰控制(加密)
	byte adaptionFieldControl  = 1;       //自适应控制 01仅含有效负载，10仅含调整字段，11都有
	byte continuityCounter  = 0;          //连续计数器 一个4bit的计数器，范围0-15(patpmt不计入连续)
	head[0] = synByte;
	head[1] = transportErrorIndicator<<7 | payloadUnitStartIndicator<<6 | transport_priority<<5 | byte((PID<<3)>>11);
	head[2] = byte(PID);
	head[3] = transportScramblingControl<<6 | adaptionFieldControl<<4 | continuityCounter;*/
	head[0] = 0x47;
	head[1] = 0x40;
	head[2] = 0x00;
	head[3] = 0x10 | mpatmcc;
	mpatmcc = (mpatmcc + 1) % 16;
	head[4] = 0; //负载与头间有个0x00的pointer_field
	setPAT(PMTpid, &tPAT, PATlen);
	memcpy(mPAT, head, sizeof(head));
	memcpy(mPAT + 5, tPAT, PATlen);
	delete[]tPAT;
	for (int pos = PATlen + 5; pos < TS_CHUNK_SIZE; pos++) {
		mPAT[pos] = 0xFF;
	}

	//PMT
	/*synByte = 0x47;               //同步字节, 固定为,表示后面的是一个TS分组
	transportErrorIndicator = 0;    //传输误码指示符
	payloadUnitStartIndicator = 1;  //有效荷载单元起始指示符, 1标明为负载起始包
	transport_priority = 0 ;        //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
	PID = PMTpid;                   //PID
	transportScramblingControl = 0; //传输加扰控制(加密)
	adaptionFieldControl = 1;       //自适应控制 01仅含有效负载，10仅含调整字段，11都有
	continuityCounter = 0  ;        //连续计数器 一个4bit的计数器，范围0-15(patpmt不计入连续)
	head[0] = synByte;
	head[1] = transportErrorIndicator<<7 | payloadUnitStartIndicator<<6 | transport_priority<<5 | byte((PID<<3)>>11);
	head[2] = byte(PID);
	head[3] = transportScramblingControl<<6 | adaptionFieldControl<<4 | continuityCounter;*/
	head[0] = 0x47;
	head[1] = 0x50;
	head[2] = 0x00;
	head[3] = 0x10 | mpmtmcc;
	mpmtmcc = (mpmtmcc + 1) % 16;
	head[4] = 0; //负载与头间有个0x00的pointer_field
	setPMT(Apid, Vpid, PCRpid, mAstreamType, &tPMT, PMTlen);
	memcpy(mPMT, head, sizeof(head));
	memcpy(mPMT + 5, tPMT, PMTlen);
	delete[]tPMT;
	for (int pos = PMTlen + 5; pos < TS_CHUNK_SIZE; pos++) {
		mPMT[pos] = 0xFF;
	}
}

//设置PAT的包负载信息
void setPAT(int16 pmtPid, byte **outBuf, int &outLen) {
	*outBuf = new byte[TS_CHUNK_SIZE];
	memset(*outBuf, 0, TS_CHUNK_SIZE);
	outLen = 0;
	byte *&PATpack = *outBuf;
	int &PATlen = outLen;
	byte section_len = 13;

	byte tableId = 0x00;                       //固定为0x00 ，标志是该表是PAT
	byte sectionLength1 = (section_len >> 8) & 0x0f; //表示这个字节后面有用的字节数，包括CRC32
	byte reserved1 = 0x03;                   // 保留位
	byte zero = 0;                                   //0
	byte sectionSyntaxIndicator = 1;        //段语法标志位，固定为1

	byte sectionLength2 = section_len & 0xff; //(section_length1 & 0x0F) << 8 | section_length2;

	byte transportStream_id1 = 0x00;
	byte transportStream_id2 = 0x01; //该传输流的ID，区别于一个网络中其它多路复用的流

	byte currentNextIndicator = 0x01; //发送的PAT是当前有效还是下一个PAT有效
	byte versionNumber = 0x00;        //范围0-31，表示PAT的版本号
	byte reserved2 = 0x03;            // 保留位

	byte sectionNumber = 0x00;     //分段的号码。第一段为00，以后每个分段加1，最多可能有256个分段
	byte lastSectionNumber = 0x00; //最后一个分段的号码

	byte prograNumber1 = 0x00; //节目号
	byte programNumber2 = 0x01;

	byte programMapPID1 = (byte)(pmtPid >> 8) & 0x1f; //节目映射表的PID 节目号为0时对应的PID为network_PID
	byte reserved3 = 0x07;                       // 保留位

	byte programMapPID2 = (byte)(pmtPid & 0xff); // program_map_PID1 << 8 | program_map_PID2

	PATpack[0] = tableId;
	PATpack[1] = sectionSyntaxIndicator << 7 | zero << 6 | reserved1 << 4 | sectionLength1;
	PATpack[2] = sectionLength2;
	PATpack[3] = transportStream_id1;
	PATpack[4] = transportStream_id2;
	PATpack[5] = reserved2 << 6 | versionNumber << 1 | currentNextIndicator;
	PATpack[6] = sectionNumber;
	PATpack[7] = lastSectionNumber;
	PATpack[8] = prograNumber1;
	PATpack[9] = programNumber2;
	PATpack[10] = reserved3 << 5 | programMapPID1;
	PATpack[11] = programMapPID2;

	uint32 crc32 = crc32S(PATpack, 12);
	PATpack[12] = (byte)(crc32 >> 24);
	PATpack[13] = (byte)(crc32 >> 16);
	PATpack[14] = (byte)(crc32 >> 8);
	PATpack[15] = (byte)(crc32);
	PATlen = 16;
	return;
	/*
		//PAT:
		tableId                byte	//8
		SectionSyntaxIndicator byte	//1 通常设为“1”
		"0"                    byte //1
		Reserved               byte //2
		SectionLength          byte	//12
		transportStreamId      byte	//16
		Reserved               byte //2
		VersionNumber          byte	//5
		CurrentNextIndicator   byte	//1
		SectionNumber          byte	//8
		lastSectionNumber      byte	//8

		for(i=0;i<N;i++){
			programNumber          	//16
			Reserved                // 3
			if(programNumber == 0){
				networkPID         	//13
			}
			else{
				programMapPID     	//13
			}
		}
		CRCW32             			//32

	*/

}

//设置PMT的包负载信息
void setPMT(int16 aPid, int16 vPid, int16 pcrPid, byte AstreamType, byte **outBuf, int &outLen) {
	*outBuf = new byte[TS_CHUNK_SIZE];
	memset(*outBuf, 0, TS_CHUNK_SIZE);
	outLen = 0;
	bool VFlag = true;
	bool AFlag = true;
	//PMT:
	byte *&PMTpack = *outBuf;
	int &PMTlen = outLen;
	int sectionLen = 16 - 3;

	byte tableId = 0x02;                              //0固定为0x02, 表示PMT表
	byte sectionLength1 = (byte)(sectionLen >> 8) & 0x0f; //1首先两位bit置为00，它指示段的byte数，由该域开始，包含CRC。
	byte reserved1 = 0x03;                            //1 0x03
	byte zero = 0x00;                                 //1 0x01
	byte sectionSyntaxIndicator = 0x01;               //1 固定为0x01

	byte sectionLength2 = (byte)(sectionLen & 0xff); //2 (section_length1 & 0x0F) << 8 | section_length2;

	byte programNumber1 = 0x00; //3 指出该节目对应于可应用的Program map PID
	byte programNumber2 = 0x01;

	byte currentNextIndicator = 0x01; //5 当该位置1时，当前传送的Program map section可用；
	byte versionNumber = 0x00;        //5 指出TS流中Program map section的版本号
	byte reserved2 = 0x03;            //5 0x03

	byte sectionNumber = 0x00; //6 固定为0x00

	byte lastSectionNumber = 0x00; //7 固定为0x00

	byte pcrPID1 = (byte)(PCRpid >> 8) & 0x1f; //8 指明TS包的PID值，该TS包含有PCR域，
	byte reserved3 = 0x07;                 //8 0x07

	byte pcrPID2 = (byte)(PCRpid & 0xff); //9 PCR_PID1 << 8 | PCR_PID2
	//该PCR值对应于由节目号指定的对应节目。
	//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
	byte programInfoLength1 = 0x00; //10 前两位bit为00。该域指出跟随其后对节目信息的描述的byte
	byte reserved4 = 0x0f;          //10 预留为0x0F

	byte programInfoLength2 = 0x00; //11 program_info_length1 <<8 | program_info_length2

	int Pos = 12;
	if (VFlag) {
		//视频信息
		byte videoStreamType = 0x1b; //17指示特定PID的节目元素包的类型。该处PID由elementary PID指定

		byte videoElementaryPID1 = (byte)((vPid >> 8) & 0x1f); //18该域指示TS包的PID值。这些TS包含有相关的节目元素
		byte videoReserved1 = 0x07;

		byte videoElementaryPID2 = (byte)(vPid & 0xff); //19 m_elementary_PID1 <<8 | m_elementary_PID2

		byte videoESnfoLength1 = 0x00; //20前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
		byte videoReserved2 = 0x0f;

		byte videoESnfoLength2 = 0x00; //21 m_ES_nfo_length1 <<8 | m_ES_nfo_length2*/

		PMTpack[Pos] = videoStreamType;
		PMTpack[Pos + 1] = videoReserved1 << 5 | videoElementaryPID1;
		PMTpack[Pos + 2] = videoElementaryPID2;
		PMTpack[Pos + 3] = videoReserved2 << 4 | videoESnfoLength1;
		PMTpack[Pos + 4] = videoESnfoLength2;
		Pos += 5;
		sectionLen += 5;
	}
	if (AFlag) {
		//音频信息
		byte audioStreamType = AstreamType; //12指示特定PID的节目元素包的类型。该处PID由elementary PID指定

		byte audioElementaryPID1 = (byte)((aPid >> 8) & 0x1f); //13该域指示TS包的PID值。这些TS包含有相关的节目元素
		byte audioReserved1 = 0x07;

		byte audioElementaryPID2 = (byte)(aPid & 0xff); //14 m_elementary_PID1 <<8 | m_elementary_PID2

		byte audioESnfoLength1 = 0x00; //15前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
		byte audioReserved2 = 0x0f;

		byte audioESnfoLength2 = 0x00; //16 m_ES_nfo_length1 <<8 | m_ES_nfo_length2

		PMTpack[Pos] = audioStreamType;
		PMTpack[Pos + 1] = audioReserved1 << 5 | audioElementaryPID1;
		PMTpack[Pos + 2] = audioElementaryPID2;
		PMTpack[Pos + 3] = audioReserved2 << 4 | audioESnfoLength1;
		PMTpack[Pos + 4] = audioESnfoLength2;
		Pos += 5;
		sectionLen += 5;
		//audio_descriptor;  //丢弃
	}

	sectionLength2 = (byte)(sectionLen & 0xff);
	sectionLength1 = (byte)(sectionLen >> 8) & 0x0f;

	PMTpack[0] = tableId;
	PMTpack[1] = sectionSyntaxIndicator << 7 | zero << 6 | reserved1 << 4 | sectionLength1;
	PMTpack[2] = sectionLength2;
	PMTpack[3] = programNumber1;
	PMTpack[4] = programNumber2;
	PMTpack[5] = reserved2 << 6 | versionNumber << 1 | currentNextIndicator;
	PMTpack[6] = sectionNumber;
	PMTpack[7] = lastSectionNumber;
	PMTpack[8] = reserved3 << 5 | pcrPID1;
	PMTpack[9] = pcrPID2;
	PMTpack[10] = reserved4 << 4 | programInfoLength1;
	PMTpack[11] = programInfoLength2;

	uint32 crc32 = crc32S(PMTpack, Pos);
	PMTpack[Pos] = (byte)(crc32 >> 24);
	PMTpack[Pos + 1] = (byte)(crc32 >> 16);
	PMTpack[Pos + 2] = (byte)(crc32 >> 8);
	PMTpack[Pos + 3] = (byte)(crc32);

	PMTlen = sectionLen + 3;

	return;
	/*
		table_id                //8
		Section_syntax_indicator//1 通常设为“1”
		"0"                     //1
		Reserved                //2
		Section_length          //12
		program_number          //16
		Reserved                //2
		Version_number          //5
		Current_next_indicator  //1
		Section_number          //8
		last_section_number     //8
		reserved                //3
		PCR_PID                 //13
		reserved                //4
		program_info_length     //12 头两位为"00"
		for(i=0;i<N;i++){
			descriptor()
		}
		for(i=0;i<N1;i++){
			stream_type         //8
			reserved            //3
			elementary_PID      //13
			reserved            //4
		   ES_info_length       //12 头两位为"00"
			for(j=0;j<N2;j++){
				descriptor();
			}
		}
		CRC_32                  //32 rpchof
	*/
}

void setSDT(byte **outBuf, int &outLen) {
	outLen = TS_CHUNK_SIZE;
	*outBuf = new byte[outLen];
	byte* &SDT = *outBuf;
	memcpy(SDT, gSDT, outLen);
}

//CRC校验
uint32 crc32S(uint8 *data, int inLen) {
	int i = 0;
	uint32 crc = 0xFFFFFFFF;
	for (i = 0; i < inLen; i++) {
		crc = (crc << 8) ^ crc32table[((crc >> 24) ^ (uint32)(data[i])) & 0xFF];
	}

	return crc;
}

//
void CMux::onData(TsChunkArray *tca, byte *inBuf, int inLen, byte framType, uint32 timestamp) {
	byte StartCode[6] = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xf0 };
	int StartCodeLen = 6;
	byte NalDivide[4] = { 0x00, 0x00, 0x00, 0x01 };
	int NalDivideLen = 4;
	int PcrLen = 6;

	//基本属性设置
	byte PcrFlag = 0;
	byte RadomAFlag = 0;
	byte DTSFlag = 0;
	bool StartCodeFlag = false;
	bool SpsPpsFlag = false;

	byte *data = NULL;
	int dataLen = 0;
	int pesLen = 0;
	int StuffLen = 0;
	byte *mcc = &mvmcc;
	int16 pid = Vpid;

	int PackRemain = 0;

	uint64 dts = (uint64)(timestamp) * 90;
	uint64 cts = (uint64)(inBuf[2]) << 16 | (uint64)(inBuf[3]) << 8 | (uint64)(inBuf[4]); //偏移量cts

	if (framType == 'I') {
		PcrFlag = 1;
		RadomAFlag = 1;
		DTSFlag = 1;
		StartCodeFlag = true;
		SpsPpsFlag = true;

		parseNAL(inBuf, inLen, &data, dataLen);
		if (dataLen <= 0) {
			logs->error("[OnData] GetSlice Failure");
			return;
		}
		pesLen += PcrLen + mSpsNalLen + mPpsNalLen + StartCodeLen + dataLen;

	}
	else if (framType == 'A') {
		RadomAFlag = 1;
		cts = 0;
		mcc = &mamcc;
		pid = Apid;
		if (mAstreamType == 0x0f) {
			parseAAC(inBuf, inLen, &data, dataLen);
		}
		else {
			dataLen = inLen - 1;
			data = new byte[dataLen];
			memcpy(data, inBuf + 1, dataLen);
		}

		if (dataLen <= 0) {
			return;
		}
		pesLen += dataLen;
	}
	else {
		DTSFlag = 1;
		StartCodeFlag = true;

		parseNAL(inBuf, inLen, &data, dataLen);
		if (dataLen <= 0) {
			logs->error("[OnData] GetSlice Failure");
			return;
		}
		pesLen += StartCodeLen + dataLen;
	}

	//写PES头
	uint64 pts = dts + cts * 90;
	byte PESHead[64] = { 0 };
	int PESHeadLen = 0;
	pesHeadPack(DTSFlag, pts, dts, dataLen*((int)(~(DTSFlag & 0x01))), PESHead, PESHeadLen);
	pesLen += PESHeadLen;
	byte pusi = 1;
	byte afc = 0x03;
	if (pesLen < 182) {
		StuffLen = 184 - pesLen;
	}
	byte tsHead[TS_CHUNK_SIZE] = { 0 };
	int tsHeadLen = 0;
	tsHeadPack(afc, pusi, mcc, pid, PcrFlag, RadomAFlag, StuffLen, timestamp, tsHead, tsHeadLen); //打包首个TS头
	pusi = 0;
	StuffLen = 0;
	RadomAFlag = 0;
	PcrFlag = 0;
	afc = 0x01;
	PackRemain = writeChunk(tca, (char *)tsHead, tsHeadLen);
	PackRemain = writeChunk(tca, (char *)PESHead, PESHeadLen);

	//写适应apple的起始码
	if (StartCodeFlag) {
		PackRemain = writeChunk(tca, (char *)StartCode, StartCodeLen);
	}

	//写SPS/PPS
	if (SpsPpsFlag) {
		if (PackRemain > mSpsNalLen) {
			PackRemain = writeChunk(tca, (char *)mSpsNal, mSpsNalLen);
		}
		else { //当SPS大于一个ts包长度
			int totalWriteLen = PackRemain;
			PackRemain = writeChunk(tca, (char *)mSpsNal, PackRemain);
			for (; totalWriteLen < mSpsNalLen;) {
				tsHeadLen = 0;
				tsHeadPack(afc, pusi, mcc, pid, PcrFlag, RadomAFlag, StuffLen, timestamp, tsHead, tsHeadLen);
				PackRemain = writeChunk(tca, (char *)tsHead, tsHeadLen);
				int writeLen = PackRemain;
				if (mSpsNalLen - totalWriteLen < writeLen/*184*/) {
					writeLen = mSpsNalLen - totalWriteLen;
					logs->debug("[CMux::onData] writeLen = mSpsNalLen - totalWriteLen");
				}
				PackRemain = writeChunk(tca, (char *)mSpsNal + totalWriteLen, writeLen);
				totalWriteLen += writeLen;
			}
		}

		if (PackRemain > mPpsNalLen) {
			PackRemain = writeChunk(tca, (char *)mPpsNal, mPpsNalLen);
		}
		else { //当PPS大于一个ts包长度
			int totalWriteLen = PackRemain;
			PackRemain = writeChunk(tca, (char *)mPpsNal, PackRemain);
			for (; totalWriteLen < mPpsNalLen;) {
				tsHeadPack(afc, pusi, mcc, pid, PcrFlag, RadomAFlag, StuffLen, timestamp, tsHead, tsHeadLen);
				PackRemain = writeChunk(tca, (char *)tsHead, tsHeadLen);
				int writeLen = PackRemain;
				if (mPpsNalLen - totalWriteLen < writeLen/*184*/) {
					writeLen = mSpsNalLen - totalWriteLen;
					logs->debug("[CMux::onData] writeLen = mSpsNalLen - totalWriteLen");
				}
				PackRemain = writeChunk(tca, (char *)mPpsNal + totalWriteLen, writeLen);
				totalWriteLen += writeLen;
			}
		}
	}

	//写NAL分隔符
	if (false && framType != 'A') {
		if (PackRemain > NalDivideLen) {
			PackRemain = writeChunk(tca, (char *)NalDivide, NalDivideLen);
		}
		else {
			int writeTotalLen = PackRemain;
			PackRemain = writeChunk(tca, (char *)NalDivide, PackRemain);
			tsHeadPack(afc, pusi, mcc, pid, PcrFlag, RadomAFlag, StuffLen, timestamp, tsHead, tsHeadLen);
			PackRemain = writeChunk(tca, (char *)tsHead, tsHeadLen);
			PackRemain = writeChunk(tca, (char *)NalDivide + writeTotalLen, NalDivideLen - writeTotalLen);
		}

	}

	//写数据
	int start = PackRemain;
	if (PackRemain > 0 && PackRemain < TS_CHUNK_SIZE) {
		PackRemain = writeChunk(tca, (char *)data, PackRemain);
	}
	else {
		logs->debug("[OnData] PackRemain, dataLen %d-%d", PackRemain, dataLen);
	}

	for (; start < dataLen;) {
		if (dataLen - start < 184) {
			StuffLen = 184 - (dataLen - start);
			afc = 0x03;
		}
		tsHeadPack(afc, pusi, mcc, pid, PcrFlag, RadomAFlag, StuffLen, timestamp, tsHead, tsHeadLen);
		PackRemain = writeChunk(tca, (char *)tsHead, tsHeadLen);
		int writeLen = PackRemain;

		if ((dataLen - start) < 184 && tsHeadLen + (dataLen - start) != TS_CHUNK_SIZE) {
			logs->debug("[OnData] LENERROR dataLen - start,tsHeadLen, StuffLen %d-%d-%d", dataLen - start, tsHeadLen, StuffLen);
		}
		if (writeLen > (dataLen - start)) {
			logs->debug("[OnData]WRITEERROR writeLen,dataLen - start,tsHeadLen, StuffLen %d-%d-%d-%d", writeLen, dataLen - start, tsHeadLen, StuffLen);
			//writeLen = dataLen - start
		}

		writeChunk(tca, (char *)data + start, writeLen);
		start += writeLen;
	}
	if (data)
	{
		delete[]data;
	}
}

//组装TS头
void CMux::tsHeadPack(byte afc, byte pusi, byte* mcc, int16 pid, byte PcrFlag, byte RadomAFlag, int StuffLen, uint32 timeStamp, byte *outBuf, int &outLen) {
	bool StuffFlag = false;
	if (StuffLen > 0) {
		StuffFlag = true;
	}

	uint64 DTS = uint64(timeStamp) * 90; //flv中记录的时间戳是DTS(TS中的时间是FLV中的1/90)

	outLen = 0;
	int &headlen = outLen;
	byte *&head = outBuf;
	int adaptationFiledLength = 0;

	byte synByte = 0x47;                   //同步字节, 固定为,表示后面的是一个TS分组
	byte transportErrorIndicator = 0;      //传输误码指示符
	byte payloadUnitStartIndicator = pusi; //有效荷载单元起始指示符, 1标明为负载起始包
	byte transport_priority = 0;           //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
	int16 PID = pid;                       //PID = 0x100
	byte transportScramblingControl = 0;   //传输加扰控制(加密)
	byte adaptionFieldControl = afc;       //自适应控制 01仅含有效负载，10仅含调整字段，11都有
	byte continuityCounter = *mcc;         //连续计数器 一个4bit的计数器，范围0-15

	head[0] = synByte;
	head[1] = transportErrorIndicator << 7 | payloadUnitStartIndicator << 6 | transport_priority << 5 | (byte)((PID << 3) >> 11);
	head[2] = (byte)(PID);
	head[3] = transportScramblingControl << 6 | adaptionFieldControl << 4 | continuityCounter;
	headlen += 4;

	if (adaptionFieldControl > 1) {
		byte discontinuityIndicator = 0;            //不连续状态指示
		byte randomAccessIndicator = RadomAFlag;    //
		byte elementaryStreamPriorityIndicator = 0; //为1当前分组有较高优先级,有多个负载在会用到（可以不考虑）
		byte pcrFlag = PcrFlag;                   //pcr标志   pes包的前面 （待定）
		byte opcrFlag = 0;                         //（待定）
		byte splicingPointFlag = 0;                //拼接点（貌似没用）
		byte transportPrivateDataFlag = 0;         //私用数据字节（可以不考虑）
		byte adaptationFiledExtension = 0;         //扩展自适应字段
		head[headlen + 1] = discontinuityIndicator << 7 | randomAccessIndicator << 6 | elementaryStreamPriorityIndicator << 5 | pcrFlag << 4 | opcrFlag << 3 | splicingPointFlag << 2 | transportPrivateDataFlag << 1 | adaptationFiledExtension;
		adaptationFiledLength += 2;

		if (pcrFlag == 1) {
			byte *pcrbuf = NULL;
			int pcrLen = 0;
			setPcr(DTS, &pcrbuf, pcrLen);
			memcpy(head + headlen + adaptationFiledLength, pcrbuf, pcrLen);
			adaptationFiledLength += 6;
			// 48 bit
		}

		if (StuffFlag) { //需要填充
			StuffLen = StuffLen - 2;
			for (int i = headlen + adaptationFiledLength; StuffLen > 0 && i < StuffLen + adaptationFiledLength; i++) {
				head[i] = 0xFF;
			}
			adaptationFiledLength += StuffLen;
		}
	}
	head[headlen] = (byte)(adaptationFiledLength - 1);
	headlen += int(adaptationFiledLength);
	*mcc = (*mcc + 1) % 16;

	return;
}

// func (Mux *SMux) GetSlice(inBuf []byte, inLen int) ([]byte, int) {
// 	var outBuf []byte
// 	var outLen int = 0
// 	var NALlen int = 0
// 	var curNALtype byte = 0
// 
// 	//var NALtype byte = 0
// 	if inBuf[1] == 0x01 { //AVCPacketType为slice
// 		for i := 5; i < inLen-4; {
// 			NALlen = int(inBuf[i])<<24 | int(inBuf[i+1])<<16 | int(inBuf[i+2])<<8 | int(inBuf[i+3])
// 			if NALlen < 0 {
// 				ulog.Error("[slice][ParserNAL] parse NALlen ERROR:", NALlen)
// 				break
// 			}
// 			curNALtype = inBuf[i+4] & 0x1f
// 			if curNALtype > 0 && curNALtype < 6 && NALlen < inLen {
// 				return inBuf[i+4 : NALlen+i+4], NALlen
// 
// 			} else {
// 				ulog.Error("[slice][ParserNAL] unusual NAL curNALtype %d", curNALtype)
// 			}
// 			i += (NALlen + 4)
// 		}
// 		return outBuf[:], outLen
// 	}
// 	return outBuf[:], -1
// }

//组装PES头
void CMux::pesHeadPack(byte DTSFlag, uint64 pts, uint64 dts, int dataLen, byte *outBuf, int &outLen) {
	outLen = 0;
	byte *&head = outBuf;
	int &headlen = outLen;
	head[0] = 0;
	head[1] = 0;
	head[2] = 1;
	head[3] = 0xE0;

	head[4] = 0x00;
	head[5] = 0x03;
	headlen += 6;

	byte PDflags = 0x02 | DTSFlag;
	if (DTSFlag < 1) {
		head[3] = 0xC0;
	}

	byte fixBit = 0x02;              //2 填充字节
	byte PESscramblingControl = 0;   //2 PES有效负载的加密模式。0表示不加密，其余表示用户自定义。
	byte PESpriority = 0;            //1 PES数据包的优先级
	byte dataAlignmentIndicator = 0; //1 为时，表明此分组头部之后紧跟着数据流描述子中定义的访问单元类型
	byte copyright = 0;              //1 版权，表示有版权
	byte originalOrCopy = 0;         //1 pos= 6 1表示原始数据，0表示备份
	head[6] = fixBit << 6 | PESscramblingControl << 4 | PESpriority << 3 | dataAlignmentIndicator << 2 | copyright << 1 | originalOrCopy;

	byte PTSDTSflags = PDflags;      //2 10表示含有PTS字段，11表示含有PTS和DTS字段，00表示不含有pts_dts和DTS。
	byte ESCRflag = 0;               //1 1表示ESCR在PES首部出现，0表示不出现
	byte ESrateFlag = 0;             //1 1表示PES分组含有ES_rate字段。0表示不含有。
	byte DSMtrickModeFlag = 0;       //1 1表示有位的trick_mode_flag字段，0表示不出现此字段。只对DSM有效。
	byte additionalCopyInfoFlag = 0; //1 1表示有copy_info_flag字段，0表示不出现此字段。
	byte PESCRCflag = 0;             //1 1表示PES分组中有CRC字段，0表示不出现此字段。
	byte PESextensionFlag = 0;       //1 pos= 7 1表示扩展字段在PES包头存在，0表示扩展字段不存在

	head[7] = PTSDTSflags << 6 | ESCRflag << 5 | ESrateFlag << 4 | DSMtrickModeFlag << 3 | additionalCopyInfoFlag << 2 | PESCRCflag << 1 | PESextensionFlag;
	head[8] = 0; //表示可选字段和填充字段所占的字节数。

	headlen += 3;

	if ((PTSDTSflags & 0x02) > 0) {
		byte PTSbuf[5] = { 0 };
		PTSbuf[0] = PTSDTSflags << 4 | (byte)((pts & 0x1C0000000) >> 29) | 0x01;	//pts&0x111000000 00000000 00000000 00000000
		PTSbuf[1] = (byte)((pts & 0x3fc00000) >> 22);								//pts&0x 00111111 11000000 00000000 00000000
		PTSbuf[2] = (byte)((pts & 0x3f8000) >> 14) | 0x01;						//pts&0x 00000000 00111111 10000000 00000000
		PTSbuf[3] = (byte)((pts & 0x7f80) >> 7);									//pts&0x 00000000 00000000 01111111 10000000
		PTSbuf[4] = (byte)(pts << 1) | 0x01;										//pts&0x 00000000 00000000 00000000 01111111

		head[8] += 5;
		memcpy(head + headlen, PTSbuf, sizeof(PTSbuf));
		headlen += 5;

	}
	if ((PTSDTSflags & 0x01) > 0) {
		byte DTSbuf[5];
		DTSbuf[0] = 0x01 << 4 | (byte)((dts & 0x1C0000000) >> 29) | 0x01;
		DTSbuf[1] = (byte)((dts & 0x3fc00000) >> 22);
		DTSbuf[2] = (byte)((dts & 0x3f8000) >> 14) | 0x01;
		DTSbuf[3] = (byte)((dts & 0x7f80) >> 7);
		DTSbuf[4] = (byte)(dts << 1) | 0x01;

		head[8] += 5;
		memcpy(head + headlen, DTSbuf, sizeof(DTSbuf));
		headlen += 5;
	}

	if (dataLen > 0) {
		int packLen = headlen + dataLen;
		head[4] = (byte)((packLen - 6) >> 8);
		head[5] = (byte)(packLen - 6);
	}
	else {
		head[4] = 0;
		head[5] = 0;
	}
	return;
}

