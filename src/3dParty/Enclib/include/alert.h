#ifndef __ALERT_H__
#define __ALERT_H__

#pragma pack(push, 8)

typedef enum 
{
	AC_A, 
	AC_B, 
	AC_C
}EnclAlertCategory;

typedef enum 
{
	AP_EMERGENCY, 
	AP_ALARM, 
	AP_WARNING,
	AP_CAUTION
}EnclAlertPriority;

typedef enum 
{
	AS_ACTIVE_UNACKNOWLEDGED, 
	AS_ACTIVE_SILENCED,
	AS_ACTIVE_ACKNOWLEDGED,
	AS_ACTIVE_TRANSFERRED,
	AS_RECTIFIED_UNACKNOWLEDGED,
	AS_NORMAL
}EnclAlertState;

typedef struct  
{
	EnclAlertCategory category;
	EnclAlertPriority priority;
	EnclAlertState state;
	int id;
	int instance;
}EnclAlert;

typedef struct  
{
	int alertId;
	int alertInstance;
	EnclAlertCategory category;
	EnclAlertPriority priority;
	char title[64];
	char description[256];
	bool aggreate;
	bool standard;
}EnclAlertTableEntry;

#pragma pack(pop)
#endif
