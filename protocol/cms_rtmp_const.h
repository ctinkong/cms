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
#ifndef __RTMP_CONST_H__
#define __RTMP_CONST_H__
#include <mem/cms_mf_mem.h>

enum RtmpConnStatus
{
	RtmpStatusError = -1,
	RtmpStatusShakeNone = 0,
	RtmpStatusShakeC0C1,
	RtmpStatusShakeS0,
	RtmpStatusShakeS1,
	RtmpStatusShakeC0,
	RtmpStatusShakeC1,
	RtmpStatusShakeSuccess,
	RtmpStatusConnect,
	RtmpStatusCreateStream,
	RtmpStatusPublish,
	RtmpStatusPlay1,
	RtmpStatusPlay2,
	RtmpStatusPause,
	RtmpStatusStop
};


enum RtmpType
{
	RtmpTypeNone,
	RtmpAsClient2Play,			//作为客户端去play流
	RtmpAsClient2Publish,			//作为客户端去publish流
	RtmpAsServerBPlay,			//作为服务端响应play命令
	RtmpAsServerBPublish,			//作为服务端响应publish命令
	RtmpAsServerBPlayOrPublish	//作为服务端开始状态还不确定
};

const std::string RtmpTypeString[] = {
	"rtmp unknow type",
	"rtmp as client 2 play",
	"rtmp as client 2 publish",
	"rtmp as server be played",
	"rtmp as server be published",
	"rtmp as server be played or published"
};

//时间戳
#define TIMESTAMP_EXTENDED	0xFFFFFF

//cs id
#define CHUNK_STREAM_ID_PROTOCOL		0x02
#define CHUNK_STREAM_ID_COMMAND			0x03
#define CHUNK_STREAM_ID_USER_CONTROL	0x04
#define CHUNK_STREAM_ID_OVERSTREAM      0x05
#define CHUNK_STREAM_VIDEO_AUDIO        0x06
#define CHUNK_STREAM_ID_PLAY_PUBLISH    0x08


#define HEADER_FORMAT_FULL						0x00  //包头：header type、时间戳、数据大小、数据类型、流ID
#define HEADER_FORMAT_SAME_STREAM				0x01  //包头：header type、时间戳、数据大小、数据类型
#define HEADER_FORMAT_SAME_LENGTH_AND_STREAM	0x02  //包头：header type、时间戳
#define HEADER_FORMAT_CONTINUATION				0x03  //包头：header type

//MESSAGE_TYPE_PING 通道
#define USER_CONTROL_STREAM_BEGIN		0x00 //服务端向客户端发送本事件通知对方一个流开始起作用可以用于通讯。在默认情况下，服务端在成功地从客户端接收连接命令之后发送本事件，事件ID为0。事件数据是表示开始起作用的流的ID。
#define USER_CONTROL_STREAM_EOF			0x01 //服务端向客户端发送本事件通知客户端，数据回放完成。如果没有发行额外的命令，就不再发送数据。客户端丢弃从流中接收的消息。4字节的事件数据表示，回放结束的流的ID。
#define USER_CONTROL_STREAM_DRY			0x02 //服务端向客户端发送本事件通知客户端，流中没有更多的数据。如果服务端在一定周期内没有探测到更多的数据，就可以通知客户端流枯竭。4字节的事件数据表示枯竭流的ID
#define USER_CONTROL_SET_BUFFER_LENGTH	0x03 //客户端向服务端发送本事件，告知对方自己存储一个流的数据的缓存的长度（毫秒单位）。当服务端开始处理一个流得时候发送本事件。事件数据的头四个字节表示流ID，后4个字节表示缓存长度（毫秒单位）。
#define USER_CONTROL_STREAM_LS_RECORDED 0x04 //服务端发送本事件通知客户端，该流是一个录制流。4字节的事件数据表示录制流的ID。
#define USER_CONTROL_PING_REQUEST		0x06 //服务端通过本事件测试客户端是否可达。事件数据是4个字节的事件戳。代表服务调用本命令的本地时间。客户端在接收到kMsgPingRequest之后返回kMsgPingResponse事件。
#define USER_CONTROL_PING_RESPONSE		0x07 //客户端向服务端发送本消息响应ping请求。事件数据是接收kMsgPingRequest请求 的 时间。


//消息类型 
#define MESSAGE_TYPE_NONE                0x00
#define MESSAGE_TYPE_CHUNK_SIZE          0x01
#define MESSAGE_TYPE_ABORT               0x02
#define MESSAGE_TYPE_ACK                 0x03
#define MESSAGE_TYPE_USER_CONTROL        0x04
#define MESSAGE_TYPE_WINDOW_SIZE         0x05
#define MESSAGE_TYPE_BANDWIDTH           0x06
#define MESSAGE_TYPE_DEBUG				 0x07
#define MESSAGE_TYPE_AUDIO               0x08
#define MESSAGE_TYPE_VIDEO               0x09
#define MESSAGE_TYPE_FLEX                0x0F
#define MESSAGE_TYPE_AMF3_SHARED_OBJECT  0x10
#define MESSAGE_TYPE_AMF3                0x11
#define MESSAGE_TYPE_INVOKE              0x12
#define MESSAGE_TYPE_AMF0_SHARED_OBJECT  0x13
#define MESSAGE_TYPE_AMF0                0x14
#define MESSAGE_TYPE_STREAM_VIDEO_AUDIO  0x16 //这是FMS3出来后新增的数据类型,这种类型数据中包含AudioData和VideoData
#define MESSAGE_TYPE_CUSTOME_RANGE		 0x17 //自定义命令
#define MESSAGE_TYPE_FIRST_VIDEO_AUDIO	 0x18 //自定义命令
/* MESSAGE_TYPE_STREAM_VIDEO_AUDIO 类型数据内容，需要再次解码
用途		大小(Byte)		数据含义
StreamType	1				流的种类（0x08=音频，0x09=视频）
MediaSize	3				媒体数据区域大小
TiMMER		3				绝对时间戳,单位毫秒
Reserve		4				保留,值为0
MediaData	MediaSize		媒体数据，音频或视频
TagLen		4				帧的大小，值为媒体数据区域大小+参数长度(MediaSize+1+3+3+4) */

//AMF
#define AMF0	0x00
#define AMF3	0x03

#define DEFAULT_CHUNK_SIZE		128
#define DEFAULT_RTMP_CHUNK_SIZE	(8 * 1024)
#define DEFAULT_WINDOW_SIZE		2500000
#define DEFAULT_BANDWIDTH_SIZE	2500000

enum AudioSupport
{
	SUPPORT_SND_NONE = 0x0001, //原始音频数据，无压缩
	SUPPORT_SND_ADPCM = 0x0002, //ADPCM 压缩
	SUPPORT_SND_MP3 = 0x0004, //mp3 压缩
	SUPPORT_SND_INTEL = 0x0008, //没有使用
	SUPPORT_SND_UNUSED = 0x0010, //没有使用
	SUPPORT_SND_NELLY8 = 0x0020, //NellyMoser 8KHZ压缩
	SUPPORT_SND_NELLY = 0x0040, //NellyMose压缩（5，11，22和44KHZ）
	SUPPORT_SND_G711A = 0x0080, //G711A 音频压缩（只用于flash media server）
	SUPPORT_SND_G711U = 0x0100, //G711U音频压缩（只用于flash media server）
	SUPPORT_SND_NELLY16 = 0x0200, //NellyMoser 16KHZ压缩
	SUPPORT_SND_AAC = 0x0400, //AAC编解码
	SUPPORT_SND_SPEEX = 0x0800, //Speex音频
	SUPPORT_SND_ALL = 0x0fff //所有RTMP支持的音频格式
};
enum VideoSupport
{
	SUPPORT_VID_UNUSED = 0x0001, //废弃的值
	SUPPORT_VID_JPEG = 0x0002, //废弃的值
	SUPPORT_VID_SORENSON = 0x0004, //Sorenson Flash video
	SUPPORT_VID_HOMEBREW = 0x0008, //V1 screen sharing
	SUPPORT_VID_VP6 = 0x0010, //On2 video (Flash 8+)
	SUPPORT_VID_VP6ALPHA = 0x0020, //On2 video with alpha channel
	SUPPORT_VID_HOMEBREWV = 0x0040, //Screen sharing version 2(Flash 8+)
	SUPPORT_VID_H264 = 0x0080, //H264 视频
	SUPPORT_VID_ALL = 0x00ff //RTMP支持的所有视频编解码器
};

enum BandWidthLimit
{
	BandWidth_Limit_Force = 0,	//收到服务器的窗口大小消息，必须发送客户端带宽确认
	BandWidth_Limit_SOFT = 1,   //客户端可灵活觉得发送？
	BandWidth_Limit_Dynamic = 2 //带宽既可以是硬限制也可以是软限制
};


enum RtmpCommand
{
	RtmpCommandConnect,
	RtmpCommandCreateStream,
	RtmpCommandPlay,
	RtmpCommandReleaseStream,
	RtmpCommandPublish,
	RtmpCommandDeleteStream
};

#define		Amf0CommandConnect          "connect"
#define 	Amf0CommandCreateStream     "createStream"
#define 	Amf0CommandCloseStream      "closeStream"
#define 	Amf0CommandDeleteStream     "deleteStream"
#define 	Amf0CommandPlay             "play"
#define 	Amf0CommandPause            "pause"
#define 	Amf0CommandOnBwDone         "onBWDone"
#define 	Amf0CommandOnStatus         "onStatus"
#define 	Amf0CommandResult           "_result"
#define 	Amf0CommandError            "_error"
#define 	Amf0CommandReleaseStream    "releaseStream"
#define 	Amf0CommandFcPublish        "FCPublish"
#define 	Amf0CommandUnpublish        "FCUnpublish"
#define 	Amf0CommandPublish          "publish"
#define 	Amf0DataSampleAccess        "|RtmpSampleAccess"
#define 	Amf0SetDataFrame            "@setDataFrame"
#define 	Amf0MetaData                "onMetaData"
#define 	Amf0MetaCheckbw             "_checkbw"
#define 	Amf0SupportP2pReq           "supportPPReq"
#define 	Amf0SupportP2pRsp           "supportPPRsp"
#define 	Amf0PushBackServer          "pushBackServer"
#define 	Amf0CommandEdge             "edgeTT"       //自定义命令
#define 	Amf0CommandYFPing           "yfping"       //自定义命令
#define 	Amf0CommandYFResponse       "yfresponse"   //自定义命令
#define 	Amf0CommandYFCheckDelay     "yfcheckdelay" //自定义命令
#define 	Amf0CommandYFCheckDelayRsp  "yfdelayrsp"   //自定义命令
// FMLE
#define 	Amf0CommandOnFcPublish    "onFCPublish"
#define 	Amf0CommandOnFcUnpublish  "onFCUnpublish"
	// the signature for packets to client.
#define 	RtmpSigFmsVer    "3,5,3,888"
#define 	RtmpSigAmf0Ver   0
#define 	RtmpSigClientId  "cms rtmp"
	// onStatus consts.
#define 	StatusLevel        "level"
#define 	StatusCode         "code"
#define 	StatusDescription  "description"
#define 	StatusDetails      "details"
#define 	StatusClientId     "clientid"
#define 	StatusRedirect     "redirect"
	// status value
#define 	StatusLevelStatus  "status"
	// status error
#define 	StatusLevelError  "error"
	// code value
#define 	StatusCodeConnectSuccess        "NetConnection.Connect.Success"
#define 	StatusCodeConnectRejected       "NetConnection.Connect.Rejected"
#define 	StatusCodeStreamReset           "NetStream.Play.Reset"
#define 	StatusCodeStreamStart           "NetStream.Play.Start"
#define 	StatusCodeStreamPublishNotify   "NetStream.Play.PublishNotify"
#define 	StatusCodeStreamPause           "NetStream.Pause.Notify"
#define 	StatusCodeStreamUnpause         "NetStream.Unpause.Notify"
#define 	StatusCodePublishStart          "NetStream.Publish.Start"
#define 	StatusCodeStreamFailed          "NetStream.Play.Failed"
#define 	StatusCodeStreamStreamNotFound  "NetStream.Play.StreamNotFound"
#define 	StatusCodeDataStart             "NetStream.Data.Start"
#define 	StatusCodeUnpublishSuccess      "NetStream.Unpublish.Success"
#define		StatusCodeCallFail              "NetConnection.Call.Failed"
	//description
#define 	DescriptionConnectionSucceeded  "Connection succeeded."
#define 	DescriptionConnectionRejected   "Connection rejected."

typedef struct _RtmpHeader
{
	unsigned int msgLength;
	unsigned int msgTypeID;
	unsigned int msgStreamID;
	unsigned int timestamp;
	unsigned int extendedTimestamp;

	OperatorNewDelete
}RtmpHeader;

typedef struct
{
	unsigned int	msgType;
	unsigned int	streamId;
	unsigned int	timestamp;
	unsigned int	absoluteTimestamp;
	unsigned int    bufLen;
	unsigned int	dataLen;
	char			*buffer;
	OperatorNewDelete
}RtmpMessage;

typedef struct _OutBoundChunkStream
{
	unsigned int	id;
	RtmpHeader		*lastHeader;
	unsigned int	lastOutAbsoluteTimestamp;
	unsigned int	lastInAbsoluteTimestamp;
	unsigned int	startAtTimestamp;
	OperatorNewDelete
}OutBoundChunkStream;

typedef struct _InboundChunkStream
{
	unsigned int	id;
	RtmpHeader		*lastHeader;
	unsigned int	lastOutAbsoluteTimestamp;
	unsigned int	lastInAbsoluteTimestamp;
	RtmpMessage		*currentMessage;
	OperatorNewDelete
}InboundChunkStream;

#endif