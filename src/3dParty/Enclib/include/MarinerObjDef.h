#ifndef __MARINER_OBJ_DEF_H__
#define __MARINER_OBJ_DEF_H__

#include "commonDef.h"

typedef enum _MarinerObjClass
{
	clrlin,					//Clearing line
	dnghlt,					//Danger highlight
	events,					//Event
	marfea,					//Mariner's feature
	marnot,					//Mariner's note
	mnufea,					//Manufacturers' feature
	positn,					//Position
	tidcur,					//Tidal stream or current vector
}MarinerObjClass;

typedef struct _CommonFields
{
	MarinerObjClass cls;
	Coord* pCoords;
	int nCoord;
	int iPRIM;
}CommonFields;

typedef struct _clrlinAttr
{
	int category;			//1,NMT(Not more than);2,NLT(Not less than)
	int scaleMin;
	char author[20];
}clrlinAttr;

typedef struct _clrlinData
{
	CommonFields cf;
	clrlinAttr attr;
}clrlinData;

typedef struct _dnghltAttr
{
	char author[20];
	int scaleMin;
}dnghltAttr;

typedef struct _dnghltData
{
	CommonFields cf;
	dnghltAttr attr;
}dnghltData;

typedef struct _eventsAttr
{
	char objName[20];
	int scaleMin;
	char localTime[5];	//Specific value for time indication in format HHMM or MM
	char author[20];
	char remark[128];
}eventsAttr;

typedef struct _eventsData
{
	CommonFields cf;
	eventsAttr attr;
}eventsData;

typedef struct _marfeaAttr
{
	char objName[20];
	int scaleMin;
	char localTime[5];	//Specific value for time indication in format HHMM or MM
	char author[20];	//inptid
	char remark[128];	//usrmrk
	char recDate[9];
}marfeaAttr;

typedef struct _marfeaData
{
	CommonFields cf;
	marfeaAttr attr;
}marfeaData;

typedef struct _marnotAttr
{
	int category;
	int scaleMin;
	char author[20];
	char remark[128];
	char recDate[9];//RECDAT
}marnotAttr;

typedef struct _marnotData
{
	CommonFields cf;
	marnotAttr attr;
}marnotData;

typedef struct _mnufeaAttr
{
	char objName[20];
	int scaleMin;
	char remark[128];
	char recDate[9];		//RECDAT
}mnufeaAttr;

typedef struct _mnufeaData
{
	CommonFields cf;
	mnufeaAttr attr;
}mnufeaData;

typedef struct _positnAttr
{
	int posFindingMethod;
	int scaleMin;
	char localTime[5];	//Specific value for time indication in format HHMM or MM
	char author[20];
}positnAttr;

typedef struct _positnData
{
	CommonFields cf;
	positnAttr attr;
}positnData;

typedef struct _tidcurAttr
{
	int category;			//0,undefined;1,predicted;2,actual
	double currentStrength;	//Specific value for the strength of the current or tidal stream in knots
	double orient;			//Orientation
	int scaleMin;
	char localTime[5];		//Specific value for time indication in format HHMM or MM
	char author[20];
}tidcurAttr;

typedef struct _tidcurData
{
	CommonFields cf;
	tidcurAttr attr;
}tidcurData;

typedef struct _MarinerObjAttr
{
	MarinerObjClass cls;
	int iPRIM;		//1,Point;2,Line;3,Area
	union
	{
		clrlinAttr Clrlin;
		dnghltAttr Dnghlt;
		eventsAttr Events;
		marfeaAttr Marfea;
		marnotAttr Marnot;
		mnufeaAttr Mnufea;
		positnAttr Positn;
		tidcurAttr Tidcur;
	}objAttr;
}MarinerObjAttr;

#endif 