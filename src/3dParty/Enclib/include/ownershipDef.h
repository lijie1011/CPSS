#ifndef __OWNERSHIP_DEF_H__
#define __OWNERSHIP_DEF_H__

#include "commonDef.h"

#pragma pack(push, 8)

typedef struct _ShipMovePara
{
	_ShipMovePara():lat(0),lon(0),heading(0),cogcrs(0),ctwcrs(0),sogspd(0),stwspd(0){}
	double lat;
	double lon;
	double heading;			//heading
	double cogcrs;			//course over ground
	int ctwcrs;				//course through water
	double sogspd;			//speed over ground
	double stwspd;			//speed through water
    int64_t updateTime;
}ShipMovePara;

typedef struct _ShipDimension
{
	double length;	// length of ship in meters
	double breadth;   // breadth (width) of ship in meters
	double draught;	// draught of ship in meters
	double airDraught;// airdraught (height) of ship in meters
} ShipDimension;

typedef enum {PTS_TICK, PTS_POINT}PastTrackStyle;
typedef enum {PS_PRIMARY, PS_SENCONDARY}PositionSource;

typedef struct PastTrackPoint
{
	// Set recordTime to 0 if you want SDK to set the time stamp for you when
	// adding past track point.
    int64_t recordTime;	// UTC time in seconds since Jan 1, 1970 midnight.
	double lat;
	double lon;
	PositionSource source;
	double speed;
	double heading;
}PastTrackInfo;

#pragma pack(pop)

#endif
