#ifndef __AIS_OBJECT_DEFINE_H__
#define __AIS_OBJECT_DEFINE_H__

#include <time.h>
#include <string.h>
#include "commonDef.h"

#pragma pack(push, 8)
//      Describe Transponder Class
typedef enum ais_transponder_class
{
	AIS_CLASS_A = 0,
	AIS_CLASS_B,
	AIS_ATON,    // Aid to Navigation   pjotrc 2010/02/01
	AIS_BASE,     // Base station
	AIS_GPSG_BUDDY, // GpsGate Buddy object
	AIS_DSC,	// DSC target
	AIS_SART,   // SART
	AIS_SAR_AIRCRAFT,   // SAR aircraft
}_ais_transponder_class;

typedef enum ais_nav_status
{
    UNDERWAY_USING_ENGINE = 0,
    AT_ANCHOR,
    NOT_UNDER_COMMAND,
    RESTRICTED_MANOEUVRABILITY,
    CONSTRAINED_BY_DRAFT,
    MOORED,
    AGROUND,
    FISHING,
    UNDERWAY_SAILING,
    HSC,
    WIG,
    RESERVED_11,
    RESERVED_12,
    RESERVED_13,
    SART_ACTIVE,
    UNDEFINED,
    ATON_VIRTUAL,
    ATON_VIRTUAL_ONPOSITION,
    ATON_VIRTUAL_OFFPOSITION,
    ATON_REAL,
    ATON_REAL_ONPOSITION,
    ATON_REAL_OFFPOSITION

}_ais_nav_status;

typedef struct _RcAISTargetInfo
{
    _RcAISTargetInfo()
    {
        memset(ShipName, 0, 21);
        memset(CallSign, 0, 8);
        memset(Destination, 0, 21);
        memset(ShipNameExtension, 0, 15);
        //b_show_AIS_CPA = false;

        SOG = 1023;     //means invalid
        COG = 3600;     //means invalid
        HDG = 511;      //means invalid
        ROTAIS = 0;
        Lat = 91;       //means invalid
        Lon = 181;      //means invalid

        //wxDateTime now = wxDateTime::Now();
        //now.MakeGMT();
        PositionReportTicks = 0;//now.GetTicks();       // Default is my idea of NOW
        StaticReportTicks = 0;//now.GetTicks();

        IMO = 0;
        mmsi = 666;
        cs = 0;
        aidtype = 0;
        NavStatus = UNDEFINED;
        SyncState = 888;
        SlotTO = 999;
        ShipType = 19;    // "Unknown"

        DimensionA = 0;
        DimensionB = 0;
        DimensionC = 0;
        DimensionD = 0;

        ETA_Mon = 0;
        ETA_Day = 0;
        ETA_Hr = 24;
        ETA_Min = 60;

        Draft = 0.;

        utc_hour = 0;
        utc_min = 0;
        utc_sec = 0;

        Class = AIS_CLASS_A;      // default

        Euro_Length = 0;            // Extensions for European Inland AIS
        Euro_Beam = 0;
        Euro_Draft = 0;
        strncpy(Euro_VIN, "       ", 8);
        UN_shiptype = 0;

        b_isEuroInland = false;
        b_blue_paddle = false;

        AckTime = 0;

        bOwnShip = false;
        bInAckTimeout = false;
        blue_paddle = 0;
        ROTIND = 0;

        blost = false;
        bshow = true;
        bactive = false;
        bdanger = false;
        bselected = false;
        bassociated = false;
        bshowassociated = true;
    }

	~_RcAISTargetInfo(){}

	int                       mmsi;

	ais_transponder_class     Class;
	int						  cs;		// 0=CLASS B SOTDMA unit	1=CLASS B CS unit	https://gpsd.gitlab.io/gpsd/AIVDM.html
	int						  aidtype;	// for msg21, navaid type
	int                       NavStatus;
	int                       SyncState;
	int                       SlotTO;
	int                       ROTAIS;
	int                       ROTIND;

	double                    SOG;
	double                    COG;
	double                    HDG;
	double                    Lon;
	double                    Lat;
	char                      CallSign[8];                // includes terminator
	char                      ShipName[21];
	char                      ShipNameExtension[15];
	unsigned char             ShipType;
	int                       IMO;
	bool                      bOwnShip;

	int                       DimensionA;
	int                       DimensionB;
	int                       DimensionC;
	int                       DimensionD;

	double                    Euro_Length;            // Extensions for European Inland AIS
	double                    Euro_Beam;
	double                    Euro_Draft;
	char                      Euro_VIN[8];
	int                       UN_shiptype;

	bool                      b_isEuroInland;
	bool                      b_blue_paddle;
	int                       blue_paddle;

	int                       ETA_Mon;
	int                       ETA_Day;
	int                       ETA_Hr;
	int                       ETA_Min;

	double                    Draft;

	char                      Destination[21];

	time_t                    PositionReportTicks;
	time_t                    StaticReportTicks;

	int                       utc_hour;
	int                       utc_min;
	int                       utc_sec;

	time_t                    AckTime;
	bool                      bInAckTimeout;

	bool						blost;				//lost status
	bool						bshow;				//show status
	bool						bactive;			//active status
	bool						bdanger;			//danger status
	bool						bselected;			//selected status
	bool						bassociated;	//ais/radar
	bool						bshowassociated;	//ais/radar
}RcAISTargetInfo;

typedef struct _AISTargetStatus
{
	int status;

}AISTargetStatus;

typedef struct _AisDimension
{
	double length;	// length of ais in meters
	double breadth;   // breadth (width) of ais in meters
	double draught;	// draught of ais in meters
	double airDraught;// airdraught (height) of ais in meters
} AisDimension;

typedef enum {PTS_NOLINE_AIS, PTS_LINE_AIS}PastTrackAisStyle;

typedef struct PastTrackAisPoint
{
	// Set recordTime to 0 if you want SDK to set the time stamp for you when
	// adding past track point.
	int64_t recordTime;	// UTC time in seconds since Jan 1, 1970 midnight.
	double lat;
	double lon;
	double speed;
	double heading;
}PastTrackAisInfo;

#pragma pack(pop)

#endif
