#ifndef __CMS_MASTER_COMMON_VAR_H__
#define __CMS_MASTER_COMMON_VAR_H__
#include <interface/cms_conn_listener.h>
#include <interface/cms_interf_conn.h>
#include <mem/cms_mf_mem.h>

#define CMS_PIPE_BUF_SIZE 8192
#define CMS_CONN_TIMEOUT_MILSECOND 0.03

typedef struct _FdQueeu
{
	int  fd;
	Conn *conn;

	OperatorNewDelete
}FdQueeu;
#endif
