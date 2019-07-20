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
#ifndef __CMS_UTILITY_H__
#define __CMS_UTILITY_H__
#include <string>
#include <vector>
#include <map>
#include <common/cms_type.h>
#include <protocol/cms_rtmp_const.h>

#define CMS_OK 0
#define CMS_ERROR -1

#define CMS_SPEED_DURATION	30 //速度统计间隔

#ifdef WIN32 /* WIN32 */
#define CMS_INVALID_SOCK INVALID_SOCKET 
#else /* posix */
#define CMS_INVALID_SOCK -1
#endif /* posix end */

#ifndef cmsMinMax
#define cmsMin(a,b)            (((a) < (b)) ? (a) : (b))
#define cmsMax(a, b)       ((a) > (b) ? (a) : (b))
#endif

#ifdef WIN32 /* WIN32 */
#include <windows.h>
#define cmsSleep(e) waitTime(e)
#else /* posix */
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#define gettid() syscall(__NR_gettid)
#define _atoi64(val) strtoll(val, NULL, 10)
#define str2float(val) strtod(val,NULL)
#define hex2int64(val) strtoll(val, NULL, 16)
#define cmsSleep(e) waitTime(e)
#endif /* posix end */
#define iclock() getTickCount()

unsigned int hash2Idx(HASH &hash);
std::string beforeHashUrl(std::string url);			//获取算hash字符串前预处理
std::string beforeMajorHashUrl(std::string url);	//获取算hash字符串前预处理
void getStrHash(char* input, int len, char* hash);
void trueHash2FalseHash(unsigned char hash[20]);
void falseHash2TrueHash(unsigned char hash[20]);
std::string hash2Char(const unsigned char* hash);
void char2Hash(const char* chars, unsigned char* hash);
void urlEncode(const char *src, int nLenSrc, char *dest, int& nLenDest);
void urlDecode(const char *src, int nLenSrc, char* dest, int& nLenDest);
std::string getUrlDecode(std::string strUrl);
std::string getUrlEncode(std::string strUrl);
std::string getBase64Decode(std::string strUrl);
std::string getBase64Encode(std::string strUrl);
unsigned long long ntohll(unsigned long long val);
unsigned long long htonll(unsigned long long val);
unsigned long getTickCount();
unsigned long long getMilSeconds();
int getTimeStr(char *dstBuf);
void getDateTime(char* szDTime);
long long getTimeUnix();
long long getNsTime();
int getTimeDay();
void waitTime(int n);
int cmsMkdir(const char *dirname);
bool isLegalIp(const char * const szIp);
unsigned long ipStr2ipInt(const char* szIP);
void ipInt2ipStr(unsigned long iIp, char* szIP);
std::string readMajorUrl(std::string strUrl);
std::string readHashUrl(std::string strUrl);
std::string readMainHost(std::string strHost);
void split(std::string& s, std::string& delim, std::vector< std::string > &ret);
std::string trim(std::string &s, std::string &delim);
HASH makeHash(const char *bytes, int len);
HASH makeUrlHash(std::string url);
HASH makePushHash(std::string url);
std::string parseSpeed8Mem(int64 speed, bool isSpeed);
void parseSpeed8Mem(int64 speed, bool isSpeed, std::string &c, std::string &unit);
bool nonblocking(int fd);
bool blocking(int fd);
void printTakeTime(std::map<unsigned long, unsigned long> &mapSendTakeTime, unsigned long ttB, unsigned long ttE, char *str, bool bPrint);
unsigned long long getVid();
long long MathAbs(long long value);
bool readAll(const char* file, char **data);
void setThreadName(const char *name);
unsigned long long addr2uid(char *ip, uint16 port);
unsigned long long addr2uid(uint64 ip, uint16 port);
std::string getConnType(ConnType &ct);
bool isHttp(ConnType &ct);
bool isHttps(ConnType &ct);
bool isRtmp(ConnType &ct);
bool isQuery(ConnType &ct);
//拷贝可是字符串，如果dst不为空，则会释放
void xCopyString(char **dst, const char *src, int len);
//拷贝内存，如果dst不为空，则会释放
void xCopyMem(char **dst, char *src, int len);
void xCopyHash(char **dst, HASH &hash);
#endif

