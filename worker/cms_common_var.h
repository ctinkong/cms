#ifndef __CMS_MASTER_COMMON_VAR_H__
#define __CMS_MASTER_COMMON_VAR_H__
#include <interface/cms_conn_listener.h>
#include <interface/cms_interf_conn.h>
#include <mem/cms_mf_mem.h>

#define CMS_CONN_TIMEOUT_MILSECOND 0.03

#define CMS_WORKER_ADD_CONN 0x01
#define CMS_WORKER_DEL_CONN 0x02

typedef struct _FdQueeu
{
	int  act;
	int  fd;
	Conn *conn;

	OperatorNewDelete
}FdQueeu;
#endif
