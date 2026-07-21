#ifndef __TT_OBJECT_DEFINE_H__
#define __TT_OBJECT_DEFINE_H__

#include <time.h>
#include <string.h>
#include "commonDef.h"

#pragma pack(push, 8)

typedef struct _RcTTTargetInfo
{
    _RcTTTargetInfo()
    {
        memset(globalLabel, 0, 16);
        memset(targetName, 0, 16);
        memset(UTCtime, 0, 10);

        targetNum = 0;
        Lat = 91;
        Lon = 181;
        targetDistance = 0;
        bearing = 511;
        TOrR1 = 0;
        targetSpeed = 1023;
        targetCourse = 3600;
        TOrR2 = 0;
        distance = 0;
        CPATime = 0;
        speedUnit = 0;
        targetName[16] = 0;
        targetStatus = 0;
        refTarget = 0;
        UTCtime[10] = 0;
        acquisitionType = 0;

        blost = false;
        bshow = false;
        bdanger = false;
        bselected = false;
        bassociated = false;
        bshowassociated = false;
    }

	~_RcTTTargetInfo(){}

	int		 targetNum;			    /* [ Target number, 00 to 99  ] */
	char     globalLabel[16];
	double	 Lat;
	double   Lon;
	double	 targetDistance;		/* [ distance in nautical miles ] */
	double	 bearing;		        /* [ bearing: 0..360 degrees ] */
	char		 TOrR1;           /* [ Bearing from own ship, degrees true/relativ e (T/R)  ] */
	double	 targetSpeed;			/* [ Target course, degrees true/relative (T/R)  ] */
	double    targetCourse;
	char		 TOrR2;			/* [Target course, degrees true/relative (T/R) ] */
	double	 distance;				/* [ Distance of closest-point-of-approach  ] */
	double	 CPATime;				/* [ Time to CPA, min., "-" increasing  ] */
	char		 speedUnit;				/* [Speed/distance units, K/N/S  ] */
	char		 targetName[16];        /* [ target Name ] */
	char		 targetStatus;			/* [Target statusL = Lost, tracked target has been lost; Q = Query, target in the process of acquisition; T = racking] */
	char		 refTarget;				/* [ Reference target ] */
	char		 UTCtime[10];           /* [Time of data (UTC)] */
	char		 acquisitionType;	    /* [ shows if acquisition is done manual or automatical ] */

	bool						blost;				//lost status
	bool						bshow;				//show status
	bool						bdanger;			//danger status
	bool						bselected;			//selected status
	bool						bassociated;	//ais/radar
	bool                        bshowassociated;
}RcTTTargetInfo;

typedef struct _TTTargetStatus
{
	int status;

}TTTargetStatus;

typedef enum {PTS_NOLINE_TT, PTS_LINE_TT}PastTrackTtStyle;

typedef struct PastTrackTtPoint
{
	// Set recordTime to 0 if you want SDK to set the time stamp for you when
	// adding past track point.
	int64_t recordTime;	// UTC time in seconds since Jan 1, 1970 midnight.
	double lat;
	double lon;
	double speed;
	double heading;
}PastTrackTtInfo;

#pragma pack(pop)

#endif
