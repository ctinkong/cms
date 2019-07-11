# cms
cms is an industrial-strength live streaming server,support rtmp,http-flv,hls.in the future,it will support more protocol.
# Building cms with new version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
./configure
```
# Building cms not change version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
make
```
# Notice
```shell
to build cms needs s2n,whitch supports TLS/SSL protocols.see https://github.com/awslabs/s2n
and current version,s2n is builded with openssl-1.1.1b.
```
# Config
```shell
{
  "listen": {
    "http": ":80",	//http listen addr
    "https": ":443",	//https listen addr,this will be effected by tls configure, 
			//when tls is not exist or is empty, or cert-key is not exist or empty,
			//https will do not listen any more.
    "rtmp": ":1935",	//rtmp listen addr
    "query": ":8888"	//server status query listen addr, see http://xxx.xxx.xxx.xxx:8888/query/info
  },
  "tls": {		//tls configure, see https://github.com/awslabs/s2n
    "cert-key": [
      {
        "cert": "push.test1.com.crt",
        "key": "push.test1.com.key"
      },
      {
        "cert": "push.test2.com.crt",
        "key": "push.test2.com.key"
      }
    ],
    "dhparam": "dhparam.pem",
    "version": "default"
  },
  "worker": {
    "count": 4				//thread worker number
  },
  "media": {
    "first_play_milsecond": 1000,	//skip millseconds of video,when the first time to play 
    "reset_timestamp": true,		//should reset timestamp,which timestamp will be increase from 0,when be played
    "no_timeout": false,		//
    "stream_timeout": 30000,		//stream should be timeout,when no data recv for long time
    "no_hash_timeout": 15000,		//timeout to wait for task creates successfully,when be played.
    "real_time_stream": true,		//should be a real time stream or not
    "cache_ms": 10000,			//stream should cache time in millseconds.
    "open_hls": true,			//should open hls support.
    "ts_duration": 3,				
    "ts_num": 3,
    "ts_save_num": 5
  },
  "log": {
    "file": "./log",			//log file.
    "level": "debug",			//log level debug < info < warn < error < fatal
    "size": 524288000,			//max size of log file
    "console": true
  }
}
```
