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
#ifndef __CMS_ERRNO_H__
#define __CMS_ERRNO_H__

enum CmsErrnoCode
{
	CMS_ERRNO_TIMEOUT = 10086100,
	CMS_ERRNO_FIN,
	CMS_S2N_ERR_T_IO, /* Underlying I/O operation failed, check system errno */
	CMS_S2N_ERR_T_CLOSED, /* EOF */
	CMS_S2N_ERR_T_BLOCKED, /* Underlying I/O operation would block */
	CMS_S2N_ERR_T_ALERT, /* Incoming Alert */
	CMS_S2N_ERR_T_PROTO, /* Failure in some part of the TLS protocol. Ex: CBC verification failure */
	CMS_S2N_ERR_T_INTERNAL, /* Error internal to s2n. A precondition could have failed. */
	CMS_S2N_ERR_T_USAGE, /* User input error. Ex: Providing an invalid cipher preference version */
	CMS_KCP_ERR_RESET_BY_PEER,
	CMS_KCP_USER_CLOSED_CONN,
	CMS_KCP_ERR_EOF,
	CMS_KCP_ERR_FAIL,/* User KCP protocol check fail */
	CMS_ERROR_UNKNOW,
	CMS_ERRNO_NONE,
};

char *cmsStrErrno(int code);
#endif
