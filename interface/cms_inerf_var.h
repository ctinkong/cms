#ifndef __CMS_INTERFACE_COMMON_H__
#define __CMS_INTERFACE_COMMON_H__
#include <common/cms_type.h>
#include <mem/cms_mf_mem.h>

typedef struct _EvCallBackParam
{
	void  *base;
	void  *cls;
	int    fd;
	// 	uint64 uid;
	// 	uint32 conv;
	bool   isPassive;
	uint32 tick;
	OperatorNewDelete
}EvCallBackParam;

#endif
