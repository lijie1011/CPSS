#ifndef __S57_MANUAL_UPDATE_DEF_H__
#define __S57_MANUAL_UPDATE_DEF_H__

#include <cstring>
#include "commonDef.h"

#pragma pack(push, 8)

typedef enum _ManUpdState
{
	MUS_ADD_NEW,		// Newly created manual update object.
	MUS_REMOVED_BY_MODIFY,
	MUS_RMV_FROM_ENC,	// Object from ENC removed.
	MUS_ENC_OBJ_MOVED	// Object from ENC moved.
}ManUpdState;

typedef union _ManUpdAttVal
{
	int valInt;
	int valIntLst[64];  // The 1th element contains the number of ints in list. 
	double valReal;
	char valStr[256];	// Null terminated.
}ManUpdAttVal;

typedef struct _ManUpdObjAtt
{
	char attrName[7];
	ManUpdAttVal val;
}ManUpdObjAtt;

typedef struct _ManUpdObjData
{
	char className[7];		// S57 Object class name.
	EnclPrimitiveType type;		// Primitive type.
	char ownerChartName[9];	// Set to 0 length string to indicate no ownerChart.
	int masterObjId;		// Set to 0 if no master object exists.
	int numAttrs;			// Set to 0 if no attribute available.
	ManUpdObjAtt * pAttrs;	// Set to 0 if no attribute available.
	EnclObjectPtr prevObjId;
}ManUpdObjData;

typedef struct _ManualUpdateObjectAttrs
{
	char author[20];
	char remark[2048];
	int64_t updTime;
}ManUpdObjAttrs;

typedef struct _EnclManualUpdateObject
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
	_EnclManualUpdateObject (): coords(0), ptCount(0),length(0),s57attrs(0){}
}EnclManualUpdateObject;

#pragma pack(pop)

#endif 
