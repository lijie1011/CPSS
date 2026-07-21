#ifndef __S57_USER_CHART_DEF_H__
#define __S57_USER_CHART_DEF_H__

#include <cstring>
#include "commonDef.h"

#pragma pack(push, 8)

typedef enum _UsrChrtState
{
	UCS_ADD_NEW,		// Newly created user chart object.
	UCS_REMOVED_BY_MODIFY,
}UsrChrtState;

typedef union _UsrChrtAttVal
{
	int valInt;
	int valIntLst[64];  // The 1th element contains the number of ints in list. 
	double valReal;
	char valStr[256];	// Null terminated.
}UsrChrtAttVal;

typedef struct _UsrChrtObjAtt
{
	char attrName[7];
	UsrChrtAttVal val;
}UsrChrtObjAtt;

typedef struct _UsrChrtObjData
{
	char className[7];		// S57 Object class name.
	EnclPrimitiveType type;		// Primitive type.
	char ownerChartName[9];	// Set to 0 length string to indicate no ownerChart.
	int numAttrs;			// Set to 0 if no attribute available.
	UsrChrtObjAtt * pAttrs;	// Set to 0 if no attribute available.
	EnclObjectPtr prevObjId;
}UsrChrtObjData;

typedef struct _UserChartObjectAttrs
{
	char author[20];
	char remark[2048];
	int64_t updTime;
}UsrChrtObjAttrs;

typedef struct _EnclUserChartObject
{
	char ownerChartName[9];	// Set to 0 length string to indicate no ownerChart.
	char name[64];
	EnclPrimitiveType type;		// Primitive type.
	int ptCount;
	double * coords;
	char author[20];
	unsigned long time;
	char remark[64];
	int status;
	int removed;	//1: removed	0: normal
	int length;
	char * s57attrs;
	_EnclUserChartObject (): coords(0), ptCount(0),length(0),s57attrs(0){}
}EnclUserChartObject;

#pragma pack(pop)

#endif 
