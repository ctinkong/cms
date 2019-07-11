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
#ifndef __CMS_SSL_H__
#define __CMS_SSL_H__
#include <core/cms_buffer.h>
#include <s2n/s2n.h>
#include <string>

class CSSL
{
public:
	CSSL(int fd, std::string remoteAddr, bool isAsClient);
	~CSSL();
	bool	run();
	int		read(char **data, int &len);
	int		write(const char *data, int &len);
	int   	peek(char **data, int &len);
	void	skip(int len);
	int		bufferWriteSize();
	bool    isHandShake();
	int		flush();
	bool	isUsable();
private:
	int		handShakeTLS();
	bool					misTlsHandShake;
	struct s2n_connection	*ms2nConn;
	struct s2n_config		*mconfig;
	struct s2n_cert_chain_and_key *mchain8Key;
	bool					misAsClient;
	int						mfd;
	std::string				mremoteAddr;

	CBufferReader	*mrdBuff;
	CBufferWriter	*mwrBuff;

	//发送缓存
	char *mbuffer;
	int  mb;
	int  me;
	int  mbufferSize;
};
#endif
