#ifndef __NMEA_H__
#define __NMEA_H__

#include <time.h>

#pragma pack(push, 8)

typedef struct
{
    char    msgType[4];
    time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
    char    talker[3];		         /* [ talkerId ] */

    int    status1;              /* [ LORAN-C Blink ] */
    int    status2;              /* [ LORAN-C Cycle Lock ] */
    double offTrackDistance;     /* [ XTE in nautical miles ] */
    int    directionToSteer;     /* [ direction to steer (L/R) ] */
    int    status3;              /* [ reached circle of approach ] */
    int    status4;              /* [ passed perpendiculum ] */
    double bearingOriginTrue;          /* [ true bearing origin to destination waypoint ] */
    double bearingOriginMagnetic;      /* [ magnetic bearing origin to destination waypoint ] */
    double bearingPresentTrue;      /*true bearing present to destination*/
    double bearingPresentMagnetic;  /*magnetic bearing present to destination*/
    double headingTrue;          /* [ true heading to destination waypoint ] */
    double headingMagnetic;      /* [ magnetic heading to destination waypoint ] */
    
}NMEASentence_APB;

typedef struct
{
    char    msgType[4];
    time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
    char    talker[3];		/* [ talkerId ] */

    double courseToSteer;        /* [ commanded course to steer, degrees ] */
    double headingTrue;          /* [ actual true heading, degrees ] */
    double headingMagnetic;      /* [ actual magnetic heading, degrees ] */
    double offCourseLimit;       /* [ off course limit, degrees ] */
    int    offCourseStatus;      /* [ off course status, 1=over limit ] */
    double offTrackLimit;        /* [ off track limit, degrees ] */
    int    offTrackStatus;       /* [ off track status, 1=over limit ] */
    double rudderLimit;          /* [ rudder limit, degrees ] */
    int    rudderResponse;       /* [ rudder response 0..9 (0=direct) ] */
    int    gainSetting;          /* [ gain setting 0..9 (0=low) ] */
    int    valid;                /* [ status for all data ] */

    

    /* the following members are proprietary */
    double bearing;              /* [ bearing origin to destination waypoint ] */
    double rudderAngle;          /* [ commanded rudder angle (-=port) ] */
}NMEASentence_ASD;

typedef struct
{
    char    msgType[4];
    time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
    char    talker[3];		/* [ talkerId ] */

    double depth;		/* [ depth (referenced to transducer) in meters ] */
    
}NMEASentence_DBT;

typedef struct 
{
    char msgType[4];
    time_t timestamp;
    char talker[3];

    int Sentences;              /*total number of sentences used for this message 1-9*/
    int SentenceNumber;         /*order of this sentence in msg 1-9*/
    int MsgId;                  /*sequential message identifier provides a message identification 0-9*/
    char AISChannel;            /*AIS Channel A or B*/
    char EncapsulatedMsg[64];   /*Encapsulated ITU-R M.1371 radio message*/

    
}NMEASentence_VDX;


typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	/* [ talkerId ] */

	char    source;               /* [ Source, shaft/engine S/E ] */
	int     number;               /* [ source id (0=midships, even=port, odd=starboard) ] */
	double  rpm;                  /* [ rotations per minute (negative=counterclockwise) ] */
	double  pitch;                /* [ propeller pitch in percent (negative=astern) ] */
	bool    valid;                /* [ data valid indicator ] */
}NMEASentence_RPM;

typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	double  valueStb;             /* [ rudder angle sensor starboard (or single) ] */
	bool    stbValid;             /* [ starboard value valid indicator ] */
	double  valuePort;            /* [ rudder angle sensor port (negative=port) ] */
	bool    portValid;            /* [ port value valid indicator ] */
}NMEASentence_RSA;

typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */
	
	double orgRange;		/* [Origin 1 range , from own ship  ] */
	double orgBearing;		/* [ Origin 1 bearing degrees from 0��] */
	double rangeMarker;		/* [Variable range marker 1 (VRM1), range  ] */
	double bearing;			/* [ Bearing line 1 (EBL1), degrees from 0��] */
	double orgRange2;		/* [Origin 2 range ] */
	double orgBearing2;		/* [ Origin 2 bearing] */
	double VBMrange;			/* [VRM 2, range ] */
	double degrees;			/* [EBL 2, degrees  ] */
	double cursorRange;		/* [ Cursor range, from own ship ] */
	double cursorBearing;	/* [Cursor bearing, degrees clockwise from 0 ] */
	double range;			/* [Range scale in use ] */
	char   rangeUnits;		/* [Range units, K = km N = nautical miles S = statute miles  ] */
	char   displayRotation; /* [Display rotation ] */
}NMEASentence_RSD;

typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	int    steeringModeStatus;   /* [ steering mode status ] */
	int    steeringStatus;       /* [ steering status ] */
	int    switchOverStatus;     /* [ normal operation / end of track ] */
	int    rudderLimitStatus;    /* [ status of rudder limit ] */
	int    steeringMode;         /* [ steering mode (manual, auto or nav) ] */
}NMEASentence_NSD;

typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	bool    arrivalStatus;  /* [arrival circle has not been entered ] */
	bool   perpendicularStatus; /* [perpendicular passed at waypoint] */
	double  circleRadius;
	char    unit;
	char    waypointID[60];
}NMEASentence_AAM;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	int     totalMessageNum;    /*[Total number of messages ]*/
	int     messageNum;			/*[Message Number]*/
	int     prnNum;				/*[Satellite PRNnumber (01 to 32)]*/
	int     gpsWeekNum;			/*[GPS Week Number]*/
	int     svHealth;			/*[SVhealth, bits 17-24 of each almanac page]*/
	double  eccentricity;		/*[Eccentricity]*/
	int     referenceTime;		/*[Almanac Reference Time]*/
	double  inclinationAngle;	 /*[Inclination Angle]*/
	double  semiMajorAxis;      /*[Rate of Right Ascension]*/
	double  perigee;            /*[ Argument of perigee.]*/
	double  longitudeNode;      /*[Longitude of ascension node]*/
	double  meanAnomaly;		/*[Mean anomaly]*/
	double  f0;					/*[F0 Clock Parameter]*/
	double  f1;					/*[ F1 Clock Parameter]*/
}NMEASentence_ALM;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;           /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	int     contentNumber;		 /*[Total number of sentences needed to transfer the message, 1 to 9]*/
	int     sentenceNumber;		 /*[Sentence number, 1 to 9]*/
	int     messageIdentifier;   /*[Sequential message identifier, 0 to 9]*/
	int     AISChannel;          /*[AIS channel for broadcast of the radio message ]*/
	int     messageID;           /*[ITU-R M.1371 Message ID]*/
	char     encapsulatedData[82];    /*[This is the content of the ��binary data�� parameter for ITU-R M.1371 Message 8]*/
	int     numberBits;		/*[Number of fill-bits, 0 to 5]*/
}NMEASentence_BBM;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	char  utcTime[10];			/*[UTC of observation]*/
	double  latitude;			/*[Waypoint latitude, N/S ]*/
	char    NOrS;				/* [(N)orth or (S)outh hemisphere] */
	double  longitude;			/*[Waypoint longitude, E/W ]*/
	char    EOrW;				/* [(E)ast or (W)est from central meridian] */
	double  bearing;			/*[Bearing, degrees true ]*/
	char    trueDegrees;		/* [True bearing indicator] */
	double  magneticBearing;	/*[Bearing, degrees magnetic ]*/
	char    magneticDegrees;	/* [Magnetic bearing indicator] */
	double  distance;			/*[Distance, nautical miles ]*/
	char    milesIndicator;     /* [(N)autical miles indicator] */
	char    waypointID[60];		/*[Waypoint ID]*/
}NMEASentence_BEC;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	double  bearing;			/*[Bearing, degrees true ]*/
	double  magneticBearing;			/*[Waypoint latitude, N/S ]*/
	char    desWpID[60];			/*[Destination waypoint ID  ]*/
	char    originWpID[60];			/*[Origin waypoint ID ]*/
}NMEASentence_BOD;

/************************************************************************/
/* $--BWC $--BWR */
/************************************************************************/
typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	char     timestampUTC[10];		  /*[Bearing, degrees true ]*/
	double  wpLat;				  /*[Waypoint Latitude]*/
	char    NOrS;     /*[N = North, S = South]*/
	double  wpLong;				  /*[Waypoint Longitude]*/
	char    EOrW;    /*[N = North, S = South]*/
	double  bearing;       /*[true track to waypoint]*/
	double  magneticBearing;      /*[Bearing, Magnetic]*/
	double  nauticalMiles;		  /*[ Nautical Miles ]*/
	char    nauticalMilesUnit;    /*[ N = Nautical Miles]*/
	char    waypointID[60];                 /*[Waypoint ID]*/
}NMEASentence_BWX;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	double  bearing;			/*[Bearing Degrees, Magnetic]*/
	double  magneticBearing;		 /*[Bearing Degrees]*/
	char  toWp[60];				 /*[TO Waypoint]*/
	char  fromWp[60];				 /*[FROM Waypoint]*/
}NMEASentence_BWW;


typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	int     MMSI;				/* [ MMSI] */
	int     msgId;				/* [ Message ID] */
	int     msgIdIndex;			/* [Message ID index ] */
	int     aStartHour;			/* [Start UTC hour, channel A ] */
	int     aStartMinute;		/* [ Start UTC minute, channel A ] */
	int     aStartSlot;			/* [Start slot, channel A ] */
	int     aSlotInterval;		/* [Slot interv al, channel A ] */
	int     tdmaSetup;			/* [ FATDMA or RATDMA/CSTDMA setup] */
	int     bStartHour;			/* [ Start UTC hour, channel B ] */
	int     bStartMinute;		/* [ Start UTC minute, channel B] */
	int     bStartSlot;			/* [ Start slot, channel B] */
	int     bSlotInterval;		/* [Slot interv al, channel B ] */
	char    sentenceFlag;		/* [Sentence status flag ] */
}NMEASentence_CBR;

typedef struct  
{
	char    msgType[4];
	time_t  timestamp;          /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];	

	bool   dataValidity;		/*[  Validity of the data, A = Valid, V = not valid]*/
	int    dataNumber;			/*[  Data set number, 0 to 9 ]*/
	int    layerNumber;			/*[  Layer number]*/
	double currentDepth;		/*[ Current depth in metres ]*/
	double currentDirection;	/*[Current direction in degrees ]*/
	char   TOrR;			/*[Direction reference in use, True/Relativ e T/R ]*/
	double currentSpeed;		/*[Current speed in knots]*/
	double layerDepth;			/*[Reference layer depth in metres]*/
	double head;				/*[Heading]*/
	char   TOrM;			/*[ Heading reference in use, True/Magnetic T/M ]*/
	char   speedRefe;         /*[Speed reference ]*/
}NMEASentence_CUR;

typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	double depth;		/* [ depth (referenced to transducer) in meters ] */
	double rangeScale;           /* [ maximum range scale in use ] */
}NMEASentence_DPT;

typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		/* [ talkerId ] */

	char   dimmPreset;			/*[Display dimming preset ]*/   
	int    brightenPercen;		/*[Brightness percentage 00 to 99]*/   
	char   colourPalette;		/*[Colour palette]*/   
	char   sentenceFlag;		/*[Sentence Status Flag ]*/   
}NMEASentence_DDC;

typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char   talker[3];		/* [ talkerId ] */

	char   _msgType;
	char   eventTime[10];
	char   systemType[2];
	char   firstIndicator[2];
	int    secondIndicator;
	int    doorNumber;
	char   doorStatus;
	char   doorSwitchSet;
	char   msgText[80];
}NMEASentence_DOR;


typedef struct
{
	char    msgType[4];
	time_t timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char   talker[3];		     /* [ talkerId ] */

	char   localDatum[10];		/*[Local datum]*/
	char   localDatumCode;	     /* [ Local datum subdivision code  ] */
	double latOffset;	     /* [ Lat offset, min, N/S  ] */
	double longOffset;	     /* [ Lon offset, min, E/W  ] */
	double altitudeOffset;	     /* [ Altitude offset, ] */
	char   referenceDatum[4];	     /* [ Reference datum ] */
}NMEASentence_DTM;



typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		     /* [ talkerId ] */

	double  eventTime;			 /*[Event time]*/
	char    _msgType;			 /*[Message type ]*/
	int    positionIndicator; /*[Position indicator of engine telegraph]*/
	int     positionIndicatorSub; /*[Position indication of sub telegrap]*/
	char    operatlocaIndicator;/*[Operating location indicator]*/
	char     shaftNumber[5];			 /*[Number of engine or propeller shaft]*/
}NMEASentence_ETL;


typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		      /* [ talkerId ] */

	char    eventTime[10];		  /*[Event time]*/
	char    tagCode[50];		  /*[Tag code used for identification of source of event ]*/
	char    eventDesc[50];		  /*[Event description ]*/
}NMEASentence_EVE;

typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		      /* [ talkerId ] */

	char    msgTypes;			  /*[Message Type]*/
	char    eventTime[10];		  /*[Event time]*/
	char    systemType[3];		  /*[Type of FSIe detection system]*/
	char    firstIndicator[3];    /*[FSIst division indicator]*/
	int     secondIndicator;	  /*[Second Division indicator]*/
	int     number;			      /*[FSIe detector number or activation detection count]*/
	char    condition;			  /*[Condition ]*/
	char    alarmState;			  /*[Alarm��s acknowledgement state]*/
	char    msgDesc[50];		  /*[Message description text]*/
}NMEASentence_FIR;

typedef struct
{
	char    msgType[4];
	time_t  timestamp;            /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		      /* [ talkerId ] */

	char  transmittFrequency[15];   /*[Transmitting frequency]*/
	char  rcvFrequency[15];		  /*[Receiving frequency ]*/
	char    modeOperation;		  /*[Mode of operation]*/
	int     powerLevel;			  /*[Power level,  0 = standby,  1 = lowest,  9 = highest ]*/
	char    sentenceStatusFlag;   /*[Sentence status flag]*/
}NMEASentence_FSI;


typedef struct
{
	char    msgType[4];
	time_t  timestamp;               /* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	char    utcTime[10];	             /*[UTC time of the GGA or GNS fix associated with this sentence]*/
	double  latExpectedError;	     /*[Expected error in latitude ]*/
	double  longExpectedError;	     /*[Expected error in longitude]*/
	double  altitudeExpectedError;   /*[Expected error in altitude]*/
	int     failedSatelliteIDNumber; /*[ID number]*/
	double  probabilitySatellite;    /*[Probability of missed detection for most likely  failed satellite ]*/
	double  failedSatellite;		 /*[Estimate of bias on most likely failed satellite  ]*/
	double  estimateDeviation;		 /*[Standard deviation of bias estimate ]*/
	char   systemID;
	char   signalID;

}NMEASentence_GBS;


typedef struct  
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	char     utcTime[10];
	double   latitude;
	double   longitude;
	int      gpsQuality;
	int      viewNumber;
	double   horizontalDilutPrecision;  /*[Horizontal dilution of precision]*/
	double   meanSeaLevel;				/*[Antenna altitude]*/
	char     antennaAltitudeUnit;		/*[Units of antenna altitude]*/
	double   geoidalSeparation;			/*[Geoidal separation]*/
	char     geoidalSeparationUnits;	/*[Units of geoidal separation]*/
	int      diffAge;						/*[Age of differential GPS data (see Note 2) ]*/
	int      diffRefID;					/*[Differential reference station ID, 0000-1023  ]*/

}NMEASentence_GGA;




typedef struct  
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];				   /* [ talkerId ] */

	double   latitude;					/* [ degrees ] */
	double   longitude;					/* [ degrees ] */
	char     timeOfPosition[10];		/* [ milliSec since midnight ] */
	bool     valid;						/* [ shows if received data is valid ] */
}NMEASentence_GLL;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;			/* [ time when parsed: sec since 1.1.1970 ] */
    char     talker[3];		               /* [ talkerId ] */

	char     utcTime[10];                  /* [ UTC of pos. fix HHMMSS ] */
	char     status;               /* [ shows if received data is valid ] */
	double   latitude;             /* [ degrees ] */
	double   longitude;            /* [ degrees ] */
	double   speed;                /* [ speed in knots ] */
	double   courseTrue;           /* [ true course: 0..360 degrees ] */
	int      date;                 /* [ date DDMMYY] */
	double   magVar;               /* [ magnetic variation, degrees ] */
	char     mode;                 /* [ mode indicator ] */
	char     navigationalStatus;	/*[Navigational status]*/
} NMEASentence_RMC;

typedef struct  
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		         /* [ talkerId ] */

	double   heading;                   /*[Heading Degrees, true]*/
	char     tr;							 /*T*/

}NMEASentence_HDT;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		         /* [ talkerId ] */

	double   heading;                   /*[Heading Degrees, magnetic]*/
	char     magnetic;							 /*M*/

}NMEASentence_HDM;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	double heading;                   /*[Magnetic Sensor heading in degrees]*/
	double deviation;							 /*[Magnetic Deviation, degrees]*/
	char   EOrW1;           /*[Magnetic Deviation direction, E = Easterly, W = Westerly*/
	double variationDegrees;             /*[Magnetic Variation degrees*/
	char   EOrW2;           /*[Magnetic Deviation direction, E = Easterly, W = Westerly*/
}NMEASentence_HDG;


typedef struct
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	double   angle;		         /* [ Wind Angle, 0 to 360 degrees ] */
	char     TOrR;		         /* [ Reference, R = Relative, T = True ] */
	double   speed;		         /* [  Wind Speed ] */
	char     speedUnits;		         /* [  Wind Speed Units, K/M/N ] */
	bool     valid;		         /* [ tatus, A = Data Valid ] */
}NMEASentence_MWV;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	double  heading;		  /* [ Heading, degrees true  ] */
	double  magneticHeading;	  /* [ Heading, degrees magnetic  ] */
	double  speed;		      /* [ speed in knots ] */
	char     knotsUnit;		      /* [ N = Knots ] */
	// introduced in newer version of NMEA 0183. 
	double  speedKm;		      /* [ speed in Km/h ] */
	char     kmUnit;		      /* [ K = Km/h ] */
}NMEASentence_VHW;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;					/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		         /* [ talkerId ] */

	double heading;		/* [ true heading: 0..360 degrees ] */
	bool    headingStatus;	/* [ shows if heading is valid ] */
	double course;		/* [ true course: 0..360 degrees ] */
	char    courseRef;		/* [ shows if course is valid ] */
	double speed;		/* [ speed in knots ] */
	char    speedRef;		/* [ type of speed reference ] */
	double set;			/* [ set: 0..360 degrees ] */
	double drift;		/* [ drift in knots ] */
	char   driftUnit;   /*[Speed units, K = km/h; N = knots; S = statute miles/h ]*/
}NMEASentence_OSD;


typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	double   rate_of_turn;			/* [ degrees / minute ] */
	bool	 valid;					/* [ shows if received data is valid ] */
}NMEASentence_ROT;



typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	double  bearing;		/* [ true course: 0..360 degrees ] */
	double  magneticBearing;	/* [ magnetic course: 0..360 degrees ] */
	double  speed;		/* [ speed in knots ] */
	char    speedKnots;  /* [  knots ] */
	char    mode;       /*[Mode indicato]*/
}NMEASentence_VTG;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	char    heading_integrity;       /*Integrity of heading*/
	char    heading_plausibility;       /*Plausibility of heading*/
	char    position_integrity;       /*Integrity of position*/
	char    position_plausibility;       /*Plausibility of position*/
	char    STW_integrity;       /*Integrity of STW*/
	char    STW_plausibility;       /*Plausibility of STW*/
	char    SOG_integrity;       /*Integrity of SOG*/
	char    SOG_plausibility;       /*Plausibility of SOG*/
	char    depth_integrity;       /*Integrity of depth*/
	char    depth_plausibility;       /*Plausibility of depth*/
	char    STW_mode;       /*Mode of STW*/
	char    time_integrity;       /*Integrity of time*/
	char    time_plausibility;       /*Plausibility of time*/
}NMEASentence_NSR;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	double    heading_degrees;       /*degrees of heading*/
	char    mode_indicator;       /*Mode indicator*/
}NMEASentence_THS;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	char    utcTime[10];		/* [ true course: 0..360 degrees ] */
	int     utcDay;		        /* [ Day, 01 to 31 (UTC) ] */
	int     utcMonth;		    /* [ Month, 01 to 12 (UTC)  ] */
	int     utcYear;	     	/* [Year (UTC)  ] */
	int     localHours;		    /* [ Local zone hours(see Note), 00 h to ��13 h  ] */
	int     localMinutes;		/* [Local zone minutes 00 to +59 ] */
}NMEASentence_ZDA;


typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

   int		 targetNum;			    /* [ Target number, 00 to 99  ] */
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
}NMEASentence_TTM;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	int		 targetNum;			    /* [ Target number, 00 to 99  ] */
	double	 targetLatitude;		/* [ distance in nautical miles ] */
	//char	 TOrR1;            /* [ Bearing from own ship, degrees true/relativ e (T/R)  ] */
	double	 targetLongitude;		/* [ Target course, degrees true/relative (T/R)  ] */
	//char	 TOrR2;			/* [Target course, degrees true/relative (T/R) ] */
	char	 targetName[16];        /* [ target Name ] */
	char	 UTCtime[10];           /* [Time of data (UTC)] */
	char	 targetStatus;		    /* [Target statusL = Lost, tracked target has been lost; Q = Query, target in the process of acquisition; T = racking]*/
	//char	 refTarget;				/* [ Reference target ] */
}NMEASentence_TLL;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	int		 targetNum;			    /* [ Target number, 00 to 99  ] */
	char	 globalLabel[16];        /* [ target label --> mmsi ] */
}NMEASentence_TLB;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	double   waterSpeedL;		/* [ longitudinal water speed in knots ] */
	double   waterSpeedT;		/* [ transverse water speed in knots ] */
	bool     waterSpeedValid;	/* [ shows if received water speed is valid ] */
	double   groundSpeedL;		/* [ longitudinal ground speed in knots ] */
	double   groundSpeedT;		/* [ transverse ground speed in knots ] */
	bool     groundSpeedValid;	/* [ shows if received ground speed is valid ] */
	double   sternWaterSpeed;	/* [ stern transverse water speed in knots ] */
	bool     sternWaterSpeedValid;	/* [ shows if stern water speed is valid ] */
	double   sternGroundSpeed;	   /* [ stern transverse ground speed in knots ] */
	bool     sternGroundSpeedValid; /* [ shows if stern ground speed is valid ] */
}NMEASentence_VBW;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	double   rudderOrde;		    /* [ Starboard (or single) rudder order (see Notes 1 and 2) ] */
	bool     rudderOrdeValid;		/* [ Status A = v alid,V = data inv alid ] */
	double   PortRudderOrder;		/* [Status A = data valid, V = data inv alid   ] */
	bool     statusValid;			/* [Port rudder order  ] */
	char     locationSource;		/* [Command source location] */
	
}NMEASentence_ROR;


typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	char   timeOfPosition[10];	/* [ milliSec since midnight ] */
	double latitude;		/* [ degrees ] */
	char   latUnit;
	double longitude;		/* [ degrees ] */
	char   longUnit;
	char   mode[20];			/* [ mode indicator ] */
	int    satInUse;		/* [ no. of satellites in use ] */
	double horDilut;		/* [ horizontal dilution of prec. ] */
	double altitude;		/* [ altitude of antenna ] */
	double geoSep;			/* [ geoidal separation ] */
	double diffAge;			/* [ age of differential data ] */
	double refId;			/* [ reference station id ] */
	char  statusIndicator;   /*[Nav igational status indicator]*/
}NMEASentence_GNS;

typedef struct
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	int      sentencesTotal;
	int      sentenceNumber;
	int     identifierText;
	char     msgText[64];
}NMEASentence_TXT;

typedef struct						/*Heartbeat supervision sentence*/
{
	char     msgType[4];
	time_t   timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char     talker[3];		        /* [ talkerId ] */

	int		 repeatInterval;		/*autonomous repeat interval in seconds*/
	char	 status;				/*equipment status A=yes, V=no*/
	int		 msgId;					/*sequential sentence identifier 0~9periodic*/
}NMEASentence_HBT;

typedef struct						/*Cyclic alert list*/
{
	char    msgType[4];
	time_t  timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		        /* [ talkerId ] */

	int		numberOfSentences;		/*total number of sentences for current msg*/
	int		sentenceNumber;			/*order of sentence in the message 01~99*/
	int		msgId;					/*sequential sentence identifier 00~99*/
	int		currentEntryIdx;		/*auxiliary var used in sentence construction*/
	int		numberAlertEntries;		/*number of alert entries,
									each entry consists of 4 fields listed bellow.
									This field is the total number of alert entries
									when passed in to constructing nmea sentences. 
									It is the number of alert entries within current 
									sentence when parsing nmea sentence however*/
	char*	manufacturerCodes;		/*manufacture mnemonic codes array,comma separated, 
									the code is a fixed 3 char combination,null
									for standard alert id*/
	int*	alertIds;				/*alert identifier array*/
	int*	alertInstances;			/*alert instance array, 1~999999, 0 is for null
									when there is only one alert of that type*/
	int*	revisionCounters;		/*revision counter array*/
}NMEASentence_ALC;

typedef struct						/*Alert sentence*/
{
	char    msgType[4];
	time_t  timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		        /* [ talkerId ] */

	int		numberOfSentences;		/*total number of sentences for current msg 1~2*/
	int		sentenceNumber;			/*order of sentence in the message 1~2*/
	int		msgId;					/*sequential sentence identifier 0~9*/
	char	lastChangeTime[10];		/*Time of last change,format: hhmmsss.ss*/
	char	alertCategory;			/*alert category. A,B,or C.*/
	char	alertPriority;			/*alert priority. E,A,W,or C .*/
	char	alertState;				/*alert state. A,S,N,O,U or V.*/
	char	manufacturerCode[4];	/*manufacture mnemonic codes,the code is a 
									fixed 3 char ,null for standard alert id*/
	int		alertId;				/*alert identifier, 7-digit integer*/
	int		alertInstances;			/*alert instance, 1~999999, 0 is for null
									when there is only one alert of that type*/
	int		revisionCounter;		/*revision counter */
	int		escalationCounter;		/*Escalation counter*/
	char	alertText[100];			/*alert text*/
	/*The first ALF sentence transmits the Alert title. An Alert title is 
	maximum 16 characters short form of the alert
	text.
	The optional second ALF sentence transmits the additional alert description.
	Additional alert description is the long description of the alert. The 
	additional alert description contains more information for decision making 
	(i.e.  alert description text).
	The second ALF sentence uses null fields for Time of last change, Alert 
	category, Alert priority, and Alert state to allow longer text. The actual 
	number of valid characters should be such that the total number of characters
	in a sentence does not exceed the “82”-character limit.
	Some equipment standards specify alert text longer than 16 characters 
	(for example the AIS standard has defined some alerts to be coded with 
	ALR-sentence and with text longer than 16 characters). In such cases,
	the first ALF sentence is used for the first 16 characters of the alert 
	text as alert title and the second ALFsentence
	to carry the full alert text.*/
}NMEASentence_ALF;

typedef struct						/*Alert command refused*/
{
	char    msgType[4];
	time_t  timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		        /* [ talkerId ] */

	char	releaseTime[10];		/*total number of sentences for current msg*/
	char	manufacturerCode[4];	/*manufacture mnemonic codes,the code is a 
									fixed 3 char ,null for standard alert id*/
	int		alertId;				/*alert identifier, 7-digit integer*/
	int		alertInstances;			/*alert instance, 1~999999, 0 is for null
									when there is only one alert of that type*/
	char	command;				/*refuse alert command. A, Q, Q or S*/
}NMEASentence_ARC;

typedef struct						/*Alert command*/
{
	char    msgType[4];
	time_t  timestamp;				/* [ time when parsed: sec since 1.1.1970 ] */
	char    talker[3];		        /* [ talkerId ] */

	char	releaseTime[10];		/*total number of sentences for current msg*/
	char	manufacturerCode[4];	/*manufacture mnemonic codes,the code is a 
									fixed 3 char ,null for standard alert id*/
	int		alertId;				/*alert identifier, 7-digit integer*/
	int		alertInstances;			/*alert instance, 1~999999, 0 is for null
									when there is only one alert of that type*/
	char	command;				/*refuse alert command. A, Q, Q or S*/
	char	flag;					/*Sentence status flag.should be C*/
}NMEASentence_ACM;


typedef union _NMEA0183Data
{
    char msgType[4];
    NMEASentence_APB APB;
    NMEASentence_ASD ASD;
    NMEASentence_DBT DBT;
	NMEASentence_BBM BBM;
	NMEASentence_BEC BEC;
	NMEASentence_BOD BOD;
	NMEASentence_BWX BWX;
	NMEASentence_BWW BWW;
	NMEASentence_CBR CBR;
	NMEASentence_CUR CUR;
	NMEASentence_DDC DDC;
	NMEASentence_DOR DOR;
	NMEASentence_DPT DPT;
	NMEASentence_DTM DTM;
	NMEASentence_ETL ETL;
	NMEASentence_EVE EVE;
	NMEASentence_FIR FIR;
	NMEASentence_FSI FSI;
	NMEASentence_GBS GBS;
	NMEASentence_GGA GGA;
	NMEASentence_GLL GLL;
	NMEASentence_HDT HDT;
	NMEASentence_HDM HDM;
	NMEASentence_HDG HDG;
	NMEASentence_MWV MWV;
	NMEASentence_VHW VHW;
	NMEASentence_OSD OSD;
	NMEASentence_ROT ROT;
	NMEASentence_VTG VTG;
	NMEASentence_NSR NSR;
	NMEASentence_THS THS;
	NMEASentence_ZDA ZDA;
	NMEASentence_TTM TTM;
	NMEASentence_TLL TLL;
	NMEASentence_TLB TLB;
	NMEASentence_VBW VBW;
	NMEASentence_ROR ROR;
	NMEASentence_GNS GNS;
	NMEASentence_TXT TXT;
	NMEASentence_RPM RPM;
	NMEASentence_RSA RSA;
	NMEASentence_RSD RSD;
	NMEASentence_RMC RMC;
	NMEASentence_NSD NSD;
	NMEASentence_AAM AAM;
	NMEASentence_ALM ALM;
    NMEASentence_VDX VDX;

	// For BAM
    NMEASentence_HBT HBT;
    NMEASentence_ALC ALC;
    NMEASentence_ALF ALF;
    NMEASentence_ARC ARC;
    NMEASentence_ACM ACM;
}NMEA0183Data;

typedef struct _NMEA2000Data
{

}NMEA2000Data;

typedef enum _SensorProtocol
{
    PROTO_NMEA0183=0,
    PROTO_NMEA2000,
}SensorProtocol;

typedef struct _RcNMEAData
{
    SensorProtocol protocol;
    union
    {
        NMEA0183Data nmea0183;
        NMEA2000Data nmea2000;
    };
}RcNMEAData;

#pragma pack(pop)

#endif
