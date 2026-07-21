#ifndef __ROUTE_WAY_POINT_DEF_H__
#define __ROUTE_WAY_POINT_DEF_H__

#include <time.h>

#pragma pack(push, 8)

typedef struct _RouteObjData
{
#define MAX_LEN_RT_NAME 1024
#define MAX_LEN_ROUTE_USER_REMARK 2048
	char name[MAX_LEN_RT_NAME];
	char usrMark[MAX_LEN_ROUTE_USER_REMARK];
	time_t createTime;
	double defaultArrivalRadius;	// Default arrival radius in meters.

	inline _RouteObjData & operator = (const _RouteObjData & rtData)
	{
		strncpy(name, rtData.name, MAX_LEN_RT_NAME);
		strncpy(usrMark, rtData.usrMark, MAX_LEN_ROUTE_USER_REMARK);
		defaultArrivalRadius = rtData.defaultArrivalRadius;
		createTime = rtData.createTime;
		return *this;
	}
	inline _RouteObjData()
	{
		strncpy(name,"",MAX_LEN_RT_NAME);
		strncpy(usrMark,"",MAX_LEN_ROUTE_USER_REMARK);
		defaultArrivalRadius = 0.;
		createTime = 0;
	};
}RouteObjData;

// WayPoint Object Interface
typedef enum{ LT_UNDEFINED, LT_RHUMB, LT_GREATCIRCLE}LegType;
typedef struct _WayPointObjData
{
#define MAX_LEN_WP_NAME 1024
#define MAX_LEN_WAPOINT_USER_REMARK 2048
#define MAX_LEN_USER_ID 256
	char m_name[MAX_LEN_WP_NAME];
	double m_wpLat;
	double m_wpLon;
	double m_radius;	// Turn radius in meters.
	double m_distance;	// Distance with last waypoint. (turn radius taken into account)
	double m_speed;
	double m_bearing;	// Bearing to the previous waypoint.
	double m_xtd;
	char m_usrMark[MAX_LEN_WAPOINT_USER_REMARK]; // user remark
	char m_usrId[MAX_LEN_USER_ID];	// user identifier 
	LegType m_leglineType;
	long long int m_stop;
	double m_totalDistance;

	_WayPointObjData(): m_wpLat(0.0),m_wpLon(0.0), m_radius(0.0),m_speed(0.0),m_xtd(0.0),
		m_distance(0.0),m_totalDistance(0.0),m_bearing(0.0)
	{
		m_leglineType = LT_RHUMB;
		memset(m_name, 0, MAX_LEN_WP_NAME);
		memset(m_usrMark, 0, MAX_LEN_WAPOINT_USER_REMARK);
		memset(m_usrId, 0, MAX_LEN_USER_ID);
	}
	inline _WayPointObjData & operator = (const _WayPointObjData & wpoData);
}WayPointObjData;

_WayPointObjData & _WayPointObjData::operator=( const _WayPointObjData & wpoData )
{
	strncpy(m_name,wpoData.m_name, MAX_LEN_WP_NAME);
	m_wpLat = wpoData.m_wpLat;
	m_wpLon = wpoData.m_wpLon;
	m_radius = wpoData.m_radius;
	m_speed = wpoData.m_speed;
	m_xtd = wpoData.m_xtd;
	strncpy(m_usrMark, wpoData.m_usrMark, MAX_LEN_WAPOINT_USER_REMARK);
	strncpy(m_usrId, wpoData.m_usrId, MAX_LEN_USER_ID);
	m_leglineType = wpoData.m_leglineType;
	m_stop = wpoData.m_stop;
	m_distance=wpoData.m_distance;
	m_totalDistance=wpoData.m_totalDistance;
	m_bearing=wpoData.m_bearing;

	return *this;
}

#pragma pack(pop)

#endif