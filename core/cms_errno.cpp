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
#include <core/cms_errno.h>
#include <string.h>

char *gstrErrno[CMS_ERRNO_NONE - CMS_ERRNO_TIMEOUT] = {
	(char *)"Timeout",
	(char *)"Connection has been EOF",
	(char *)"Underlying I/O operation failed, check system errno",
	(char *)"Connection has been EOF",
	(char *)"Underlying I/O operation would block",
	(char *)"Incoming Alert",
	(char *)"Failure in some part of the TLS protocol. Ex: CBC verification failure",
	(char *)"Error internal to s2n. A precondition could have failed",
	(char *)"User input error. Ex: Providing an invalid cipher preference version",
	(char *)"Connection is reset by peer",
	(char *)"Use a closed conn",
	(char *)"Connection has been EOF",
	(char *)"User KCP protocol check fail"
};

char *cmsStrErrno(int code)
{
	if (code >= CMS_ERRNO_TIMEOUT && code < CMS_ERRNO_NONE)
	{
		return gstrErrno[code - CMS_ERRNO_TIMEOUT];
	}
	return strerror(code);
}