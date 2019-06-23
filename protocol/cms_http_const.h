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
#ifndef __CMS_HTTP_CONST_H__
#define __CMS_HTTP_CONST_H__
#include <common/cms_type.h>

#define HTTP_VERSION						"HTTP/1.1"
#define HTTP_HEADER_UPGRADE					"upgrade"
#define HTTP_HEADER_WEBSOCKET				"websocket"
#define HTTP_HEADER_WEBSOCKET_VERSION_NUM	"13"
#define HTTP_HEADER_CONNECTION				"connection"
#define HTTP_HEADER_SEC_WEBSOCKET_VER		"sec-websocket-version"
#define HTTP_HEADER_SEC_WEBSOCKET_KEY		"sec-websocket-key"
#define HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL	"sec-websocket-protocol"
#define HTTP_HEADER_ORIGIN					"origin"

#define HTTP_HEADER_RSP_CONTENT_TYPE				"Content-Type"
#define HTTP_HEADER_RSP_SERVER						"Server"
#define HTTP_HEADER_RSP_CACHE_CONTROL				"Cache-Control"
#define HTTP_HEADER_RSP_PRAGMA						"Pragma"
#define HTTP_HEADER_RSP_ACCESS_CONTROL_ALLOW_ORIGIN	"Access-Control-Allow-Origin"
#define HTTP_HEADER_RSP_CONNECTION					"Connection"
#define HTTP_HEADER_RSP_CONTENT_LENGTH				"Content-Length"

#define HTTP_HEADER_REQ_USER_AGENT					"User-Agent"
#define HTTP_HEADER_REQ_REFERER						"Referer"
#define HTTP_HEADER_REQ_ICY_METADATA				"Icy-MetaData"

//websocket
#define HTTP_HEADER_RSP_UPGRADE					"Upgrade"
#define HTTP_HEADER_RSP_CONNECTION				"Connection"
#define HTTP_HEADER_RSP_SEC_WEBSOCKET_ACCEPT	"Sec-WebSocket-Accept"
#define HTTP_HEADER_RSP_SEC_WEBSOCKET_PROTOCOL	"Sec-WebSocket-Protocol"
#define HTTP_HEADER_RSP_TRANSFER_ENCODING		"transfer-encoding"


#define HTTP_HEADER_HOST					"host"
#define HTTP_HEADER_LOCATION				"location"

//http code
#define HTTP_CODE_101	101
#define HTTP_CODE_200	200
#define HTTP_CODE_206	206
#define HTTP_CODE_301	301
#define HTTP_CODE_302	302
#define HTTP_CODE_303	303
#define HTTP_CODE_404	404

//http key-value
#define HTTP_VALUE_CHUNKED	"chunked"

#endif
