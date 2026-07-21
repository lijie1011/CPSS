#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__
#include <cassert>
#include <string>
#include <cstring>
#include <algorithm>

#if defined (__VXWORKS5__) || \
	defined (__VXWORKS6__) || \
	(defined(__GNUC__) && __GNUC__ < 4) || \
	(defined(_MSC_VER) && _MSC_VER < 1600)
#else
#   if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1600)
    #include <cstdint>
#   endif
#endif 

#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif


#if defined(__GNUC__) && !defined(__VXWORKS5__)
#if __GNUC__ < 4 || (__GNUC__==4 && __GNUC_MINOR__ < 6)

	#if (__GNUC__ == 4 && __GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ == 6)
	// For Red Hat compiler
	#else
		typedef long long int int64_t;

		#ifndef UINT64_MAX
		typedef unsigned long long int uint64_t;
		#endif
	#endif

#endif
#endif

#if defined (__VXWORKS5__) || defined (__VXWORKS6__)
#include "stdint.h"
#endif
#include "id.h"

#define CHART_NAME_LEN 8
#define CHART_FILE_NAME_LEN 12
#define INVALID_DEPTH 99999.0f
#define INVALID_HEIGHT 99999.0f

#pragma pack(push, 8)
// Definitions

typedef struct _EnclHeaderRequest
{
    int HR_HDR_ENCT;            //enc type(0:s57    1:s63)
    int HR_HDR_INTU;           //intended usage
                                //1: overview
                                //2: general
                                //3: coastal
                                //4: approach
                                //5: harbour
                                //6: berthing
    char HR_HDR_DSNM[12];            //Original Filename
    char HR_HDR_DSVR[3];            //DataServer
    int HR_HDR_EDTN;           //edition number
    int HR_HDR_UPDN;           //Update number
    char HR_HDR_UADT[9];       //Update application date A(8)
    char HR_HDR_ISDT[9];       //issue date A(8)
    char HR_HDR_STED[4];       //S-57 Version String
    char HR_HDR_PRSP[128];		// Product Specification
    double HR_HDR_PRED;		// Product specification edition number
    int HR_HDR_AGEN;           //Producing agency
    int HR_HDR_DSTR;           //Data structure
    int HR_HDR_AALL;           //ATTF lexical level
    int HR_HDR_NALL;           //NATF lexical level
    int HR_HDR_HDAT;           //Horizontal geodetic datum
                                /*  1 : WGS 72
                                    2 : WGS 84
                                    3 : European 1950
                                    4 : Potsdam Datum
                                    5 : Adindan
                                    6 : Afgooye
                                    7 : Ain el Abd 1970
                                    8 : Anna 1 Astro 1965
                                    9 : Antigua Island Astro 1943
                                    10 : Arc 1950
                                    11 : Arc 1960
                                    12 : Ascension Island 1958
                                    13 : Astro beacon "E" 1945
                                    14 : Astro DOS 71/4
                                    15 : Astro Tern Island (FRIG) 1961
                                    16 : Astronomical Station 1952
                                    17 : Australian Geodetic 1966
                                    18 : Australian Geodetic 1984
                                    19 : Ayabelle Lighthouse
                                    20 : Bellevue (IGN)
                                    21 : Bermuda 1957
                                    22 : Bissau
                                    23 : Bogota Observatory
                                    24 : Bukit Rimpah
                                    25 : Camp Area Astro
                                    26 : Campo Inchauspe 1969
                                    27 : Canton Astro 1966
                                    28 : Cape
                                    29 : Cape Canaveral
                                    30 : Carthage
                                    31 : Chatam Island Astro 1971
                                    32 : Chua Astro
                                    33 : Corrego Alegre
                                    34 : Dabola
                                    35 : Djakarta (Batavia)
                                    36 : DOS 1968
                                    37 : Easter Island 1967
                                    38 : European 1979
                                    39 : Fort Thomas 1955
                                    40 : Gan 1970
                                    41 : Geodetic Datum 1949
                                    42 : Graciosa Base SW 1948
                                    43 : Guam 1963
                                    44 : Gunung Segara
                                    45 : GUX 1 Astro
                                    46 : Herat North
                                    47 : Hjorsey 1955
                                    48 : Hong Kong 1963
                                    49 : Hu-Tzu-Shan
                                    50 : Indian
                                    51 : Indian 1954
                                    52 : Indian 1975
                                    53 : Ireland 1965
                                    54 : ISTS 061 Astro 1968
                                    55 : ISTS 073 Astro 1969
                                    56 : Johnston Island 1961
                                    57 : Kandawala
                                    58 : Kerguelen Island 1949
                                    59 : Kertau 1948
                                    60 : Kusaie Astro 1951
                                    61 : L. C. 5 Astro 1961
                                    62 : Leigon
                                    63 : Liberia 1964
                                    64 : Luzon
                                    65 : Mahe 1971
                                    66 : Massawa
                                    67 : Merchich
                                    68 : Midway Astro 1961
                                    69 : Minna
                                    70 : Montserrat Island Astro 1958
                                    71 : M= Poraloko
                                    72 : Nahrwan
                                    73 : Naparima, BWI
                                    74 : North American 1927
                                    75 : North American 1983
                                    76 : Observatorio Meteorologico 1939
                                    77 : Old Egyptian 1907
                                    78 : Old Hawaiian
                                    79 : Oman
                                    80 : Ordnance Survey of Great Britain 1936
                                    81 : Pico de las Nieves
                                    82 : Pitcairn Astro 1967
                                    83 : Point 58
                                    84 : Pointe Noire 1948
                                    85 : Porto Santo 1936
                                    86 : Provisional South American 1956
                                    87 : Provisional South Chilean 1963 (also known as Hito XVIII 1963)
                                    88 : Puerto Rico
                                    89 : Qatar national
                                    90 : Qornoq
                                    91 : Reunion
                                    92 : Rome 1940
                                    93 : Santo (DOS) 1965
                                    94 : Sao Braz
                                    95 : Sapper Hill 1943
                                    96 : Schwarzeck
                                    97 : Selvagem Grande 1938
                                    98 : South American 1969
                                    99 : South Asia
                                    100 : Tananarive Observatory 1925
                                    101 : Timbalai 1948
                                    102 : Tokyo
                                    103 : Tristan Astro 1968
                                    104 : Viti Levu 1916
                                    105 : Wake-Eniwetok 1960
                                    106 : Wake Island Astro 1952
                                    107 : Yacare
                                    108 : Zanderij
                                    109 : American Samoa 1962
                                    110 : Deception Island
                                    111 : Indian 1960
                                    112 : Indonesian 1974
                                    113 : North Sahara 1959
                                    114 : Pulkovo 1942
                                    115 : S-42 (Pulkovo 1942)
                                    116 : S-JYSK
                                    117 : Voirol 1950
                                    118 : Average Terrestrial System 1977
                                    119 : Compensation Géodésique du Québec 1977
                                    120 : Finnish (KKJ)
                                    121 : Ordnance Survey of Ireland
                                    122 : Revised Kertau
                                    123 : Revised Nahrwan
                                    124 : GGRS 76 (Greece)
                                    125 : Nouvelle Triangulation de France
                                    126 : RT 90 (Sweden)
                                    127 : Geocentric Datum of Australia (GDA)
                                    128 : BJZ54 (A954 Beijing Coordinates)
                                    129 : Modified BJZ54
                                    130 : GDZ80
                                    131 : Local datum*/
    int HR_HDR_VDAT;           //Vertical datum
                                 /* 1 : Mean low water springs
                                    2 : Mean lower low water springs
                                    3 : Mean sea level
                                    4 : Lowest low water
                                    5 : Mean low water
                                    6 : Lowest low water springs
                                    7 : Approximate mean low water springs
                                    8 : Indian spring low water
                                    9 : Low water springs
                                    10 : Approximate lowest astronomical tide
                                    11 : Nearly lowest low water
                                    12 : Mean lower low water
                                    13 : Low water
                                    14 : Approximate mean low water
                                    15 : Approximate mean lower low water
                                    16 : Mean high water
                                    17 : Mean high water springs
                                    18 : High water
                                    19 : Approximate mean sea level
                                    20 : High water springs
                                    21 : Mean higher high water
                                    22 : Equinoctial spring low water
                                    23 : Lowest astronomical tide
                                    24 : Local datum
                                    25 : International Great Lakes Datum 1985
                                    26 : Mean water level
                                    27 : Lower low water large tide
                                    28 : Higher high water large tide
                                    29 : Nearly highest high water
                                    30 : Highest astronomical tide (HAT)*/
    int HR_HDR_SDAT;           //Sounding datum, value same as VDAT
    int HR_HDR_CSCL;           //Compilation scale of data
    int HR_HDR_DUNI;           //Units of depth measurement
                                  /*1 : metres
                                    2 : fathoms and feet
                                    3 : feet
                                    4 : fathoms and fractions*/
    int HR_HDR_HUNI;           //Units of height measurement
                                 /* 1 : metres
                                    2 : feet*/
    int HR_HDR_COMF;           //Coordinate multiplication factor
    int HR_HDR_SOMF;           //3-D (sounding) multiplication factor
    int HR_HDR_PROJ;           //Data Set Projection
                                /*  0: means this field not exist
                                    ALA {1} Albert equal area
                                    AZA {2} Azimuthal equal area
                                    AZD {3} Azimuthal equal distance
                                    GNO {4} Gnonomic
                                    HOM {5} Hotine oblique Mercator (rectified skew orth omorphic)
                                    LCC {6} Lambert conformal conic
                                    LEA {7} Lambert equal area
                                    MER {8} Mercator
                                    OME {9} Oblique Mercator
                                    ORT {10} Orthographic
                                    PST {11} Polar stereo graphic
                                    POL {12} Polyconic
                                    TME {13} Transverse Mercator
                                    OST {14} Oblique stereographic*/
    double HR_HDR_SLAT;        //South west latitude
    double HR_HDR_WLON;        //South west longitude
    double HR_HDR_NLAT;        //North east latitude
    double HR_HDR_ELON;        //North east longitude
    int HR_HDR_PUNI;           //Units of positional accuracy
    int HR_HDR_nObjs;
}EnclHeaderRequest;

typedef struct _Coord
{
	double lat;
	double lon;
}Coord;

typedef enum _ColorScheme
{
	CS_COLOR_DAY,
	CS_COLOR_DUSK,
	CS_COLOR_NIGHT,
	CS_COLOR_GBD
}ColorScheme;

typedef enum _DisplayCategory
{
	ENCL_BASE = 'B',
	ENCL_STANDARD = 'S',
	ENCL_OTHER = 'O',
	ENCL_CUSTOM = 'C',
}EnclDisplayCategory;

typedef enum _ChartLoadMode
{
	ENCL_LOAD_MODE_AUTO,
	ENCL_LOAD_MODE_MANUAL_SINGLE,
	ENCL_LOAD_MODE_MANUAL
}EnclChartLoadMode;

typedef struct _DangerObjInfo
{
	EnclObjectPtr objID;
    int objType;    //0: s57obj     1: manualupdate obj
}DangerObjInfo;

// swap plain old data type array.
template <class T,size_t N>
void SwapPODArray (T(&first)[N], T(&second)[N], int size)
{
    T *t = new T[size];

    memcpy(t, first, size * sizeof(T));
    memcpy(first, second, size * sizeof(T));
    memcpy(second, t, size * sizeof(T));

    if(t)
        delete [] t;
}

// Class to Store polyline or polygon vertices.
// It should store only one polyline / polygon.
class PolyData
{
public:
	// Take ownership of passed in data.
	// For Polyline and Polygon data.
	// Note: For polyline  mynPolies should be 1.
	// For polygon the first ring should be the exterior ring, others should be interior 
	// rings. For multiple polygons, array of PolyData should be used.
	PolyData(int mynPolies, const int *myaryPtCnts, const double * mypolyData)
	{
		assert(mynPolies);
		nPolyies = mynPolies;
		aryPtCnts = new int [nPolyies];
		memcpy(aryPtCnts, myaryPtCnts, sizeof(int) * nPolyies);
		int nTotalPts = 0;
		for (int i = 0; i < nPolyies; ++ i)
			nTotalPts += aryPtCnts[i];

		ptData = new double[nTotalPts * 2];
		memcpy(ptData, mypolyData, sizeof(double) * nTotalPts * 2);
	}

	// For MultiPoint data only.
	PolyData(int nPts, const double * pPtData)
	{
		assert(nPts);
		nPolyies = 0;
		aryPtCnts = new int(nPts);
		ptData = new double[nPts * 2];
		memcpy(ptData, pPtData, sizeof(double) * nPts * 2);
	}

	// For 2.5d point. For compatible design, we used 2 points 4 coordinates.
	PolyData(const double * pXy, double z)
	{
		nPolyies = 0;
		aryPtCnts = new int(2);
		ptData = new double[4];
		ptData[0] = *pXy ++;
		ptData[1] = *pXy;
		ptData[2] = z;
		ptData[3] = 0;
	}

	~PolyData()
	{
		if ( aryPtCnts) delete [] aryPtCnts;
		if ( ptData) delete [] ptData;
	}

	PolyData(): nPolyies(0), aryPtCnts(0),ptData(0){}

	PolyData(const PolyData & mp)
	{
		nPolyies = mp.nPolyies;
		if ( mp.nPolyies != 0)
		{
			nPolyies = mp.nPolyies;
			aryPtCnts = new int[mp.nPolyies];
			memcpy(aryPtCnts, mp.aryPtCnts, mp.nPolyies * sizeof(int));

			int nTotalPts = 0;
			for ( int i = 0; i < mp.nPolyies; i ++)
				nTotalPts += mp.aryPtCnts[i];

			ptData = new double[nTotalPts * 2];
			memcpy(ptData, mp.ptData, sizeof(double) * nTotalPts * 2 );
		}
		else
		{
			if(mp.aryPtCnts && mp.aryPtCnts[0] != 0)
			{
				nPolyies = 0;
				aryPtCnts = new int(mp.aryPtCnts[0]);
				ptData = new double[mp.aryPtCnts[0] * 2];
				memcpy(ptData, mp.ptData,
					sizeof(double) * mp.aryPtCnts[0] * 2 );
			}
			else
			{
				if (!mp.aryPtCnts)
				{
					aryPtCnts = 0;
					ptData = 0;
				}
				else
				{
					aryPtCnts = new int(0);
					ptData = 0;
				}
			}
		}
	}

	PolyData & operator= (PolyData mp)
	{
		swap_(*this, mp);
		return * this;
	}

	friend void swap_(PolyData & first, PolyData & second)
	{
		std::swap(first.nPolyies, second.nPolyies);
		std::swap(first.ptData, second.ptData);
		std::swap(first.aryPtCnts, second. aryPtCnts);
	}
	int GetNumPolies() const {return nPolyies;}
	const int * GetPointCounts()const { return aryPtCnts;}
	const double * GetPolyData() { return ptData;}
	const double * GetPolyData()const { return ptData;}

	int GetTotalNumPoints() const
	{
		int n = 0;
		for (int i = 0; i < nPolyies; ++ i)
			n += aryPtCnts[i];
		return n;
	}

private:
	int nPolyies;
	int * aryPtCnts;
	double * ptData;
};

class EnclObjectGeoData{
public:
	typedef enum _GeoType
	{
		GT_POINT,
		GT_25D_POINT,
		GT_MULT_POINT,
		GT_LINE,
		GT_AREA,
		GT_INVALID
	} GeoType;

	typedef union _ptData
	{
		double singlePtData[2];
		// For GT_25D_POINT type, set point count = 2, set the first 3 coordinate
		// value of all 4 values.
		PolyData *multiPtData;
	}ptData;

	EnclObjectGeoData(){memset(&data, 0, sizeof(double) * 2); type = GT_INVALID;}

	// Construct point geometry data.
	EnclObjectGeoData(const double * mydata)
	{
		type = GT_POINT;
		data.singlePtData[0] = mydata[0];
		data.singlePtData[1] = mydata[1];
	}

	EnclObjectGeoData(double x, double y)
	{
		type = GT_POINT;
		data.singlePtData[0] = x;
		data.singlePtData[1] = y;
	}
	// Construct line/area/2.5d point geometry data. Data is owned by current 
	// object.
	EnclObjectGeoData(GeoType t, const PolyData * mydata) : type(t)
	{
		data.multiPtData = new PolyData(*mydata);
	}

	EnclObjectGeoData(const EnclObjectGeoData & other)
	{
		type = other.type;
		if (type == GT_POINT)
		{
			data.singlePtData[0] = other.data.singlePtData[0];
			data.singlePtData[1] = other.data.singlePtData[1];
		}
		else
		{
			data.multiPtData = new PolyData(*(other.data.multiPtData));
		}
	}

	~EnclObjectGeoData()
	{
		if (type != GT_POINT && data.multiPtData)
			delete data.multiPtData;
	}

	GeoType GetType(){return type;}
	GeoType GetType() const {return type;}
	ptData GetData(){return data;}
	ptData GetData() const {return data;}

	friend void swap_(EnclObjectGeoData & first, EnclObjectGeoData & second)
	{
		std::swap(first.type, second.type);
		if (first.type == GT_POINT)
		{
			first.data.singlePtData[0] = second.data.singlePtData[0];
			first.data.singlePtData[1] = second.data.singlePtData[1];
		}
		else
		{
			std::swap(first.data.multiPtData, second.data.multiPtData);
		}
	}

	EnclObjectGeoData & operator= (EnclObjectGeoData other)
	{
		swap_(*this, other);
		return *this;
	}

public:
	GeoType type;
	ptData data;
};

typedef struct _AutoUpdateRecord
{
    _AutoUpdateRecord()
    {
        memset(dataSetName, 0, 9);
        memset(updApplicationDate, 0, 9);
        memset(issueDate, 0, 9);
        memset(className, 0, 7);
        cnt = 0;
        pos = 0;
    }
    ~_AutoUpdateRecord()
    {
        if(pos){
            delete [] pos;
            pos = 0;
        }
    }

	_AutoUpdateRecord(const _AutoUpdateRecord & aur)
	{
		strncpy(dataSetName, aur.dataSetName, 9);			// Data Set name
		updateNumber = aur.updateNumber;				// Update number
		editionNumber = aur.editionNumber;				// Data set edition number
		strncpy(updApplicationDate, aur.updApplicationDate, 9);		// Update application date
		strncpy(issueDate, aur.issueDate, 9);				// Data set Issue date
		versionNumber = aur.versionNumber;				// Update feature record version number
		updateInstruction = aur.updateInstruction;			// Update instruction 1: insert 2: Delete 3: Modify
		recordID = aur.recordID;					// Update feature identity
		geoPrimitive = aur.geoPrimitive;				// Geometric primitive 1 : Point 2: Line 3: Area
		cnt = aur.cnt;						// counts of position points
		if(cnt > 0){
			pos = new double[cnt*2];		// pointer to postions(lon1 lat1 lon2 lat2...)
			memcpy(pos, aur.pos, cnt*2*sizeof(double));
		}else{
			pos = 0;
		}
		strncpy(className, aur.className, 7);				// Update feature object class name
	}

	_AutoUpdateRecord & operator= (_AutoUpdateRecord aur)
	{
		swap_(*this, aur);
		return * this;
	}

	friend void swap_(_AutoUpdateRecord & first, _AutoUpdateRecord & second)
	{
		SwapPODArray(first.dataSetName, second.dataSetName, sizeof(first.dataSetName)); // Data Set name
		SwapPODArray(first.updApplicationDate, second.updApplicationDate,
					 sizeof(first.updateInstruction)); // Update application date
		SwapPODArray(first.issueDate, second.issueDate,sizeof(first.issueDate)); // Data set Issue date
		SwapPODArray(first.className, second.className,sizeof(first.className));
		std::swap(first.updateNumber, second.updateNumber);				// Update number
		std::swap(first.editionNumber, second.editionNumber);				// Data set edition number
		std::swap(first.versionNumber, second.versionNumber);				// Update feature record version number
		std::swap(first.updateInstruction, second.updateInstruction);			// Update instruction 1: insert 2: Delete 3: Modify
		std::swap(first.recordID, second.recordID);					// Update feature identity
		std::swap(first.geoPrimitive, second.geoPrimitive);				// Geometric primitive 1 : Point 2: Line 3: Area
		std::swap(first.cnt, second.cnt);						// counts of position points
		std::swap(first.pos, second.pos);					// pointer to postions(lon1 lat1 lon2 lat2...)
	}

	char dataSetName[9];			// Data Set name
	int updateNumber;				// Update number
	int editionNumber;				// Data set edition number
	char updApplicationDate[9];		// Update application date
	char issueDate[9];				// Data set Issue date
	int versionNumber;				// Update feature record version number
	int updateInstruction;			// Update instruction 1: insert 2: Delete 3: Modify
	int recordID;					// Update feature identity
	int geoPrimitive;				// Geometric primitive 1 : Point 2: Line 3: Area
	int cnt;						// counts of position points
	double* pos;					// pointer to postions(lon1 lat1 lon2 lat2...)
	char className[7];				// Update feature object class name
}AutoUpdateRecord;

typedef enum _ViewGroupAction
{
	ENCL_VGA_SET,
	ENCL_VGA_CLEAR,
	ENCL_VGA_SET_ALL,
	ENCL_VGA_CLEAR_ALL,
	ENCL_VGA_CLEAR_ALL_EXCEPT_BASE
}EnclViewingGroupAction;

typedef enum _TextGroupAction
{
    ENCL_TGA_SET,
    ENCL_TGA_CLEAR,
    ENCL_TGA_SET_ALL,
    ENCL_TGA_CLEAR_ALL
}EnclTextGroupAction;

typedef enum _HeaderRequest
{	
	HR_HDR_ENCT,
	HR_HDR_INTU,
	HR_HDR_DSNM,
	HR_HDR_DSVR,
	HR_HDR_EDTN,
	HR_HDR_UPDN,
	HR_HDR_UADT,
	HR_HDR_ISDT,
	HR_HDR_STED,
	HR_HDR_PRSP,
	HR_HDR_PRED,
	HR_HDR_AGEN,
	HR_HDR_DSTR,
	HR_HDR_AALL,
	HR_HDR_NALL,
	HR_HDR_HDAT,
	HR_HDR_VDAT,
	HR_HDR_SDAT,
	HR_HDR_CSCL,
	HR_HDR_DUNI,
	HR_HDR_HUNI,
	HR_HDR_COMF,
	HR_HDR_SOMF,
        HR_HDR_PROJ,
        HR_HDR_SLAT,
	HR_HDR_WLON,
	HR_HDR_NLAT,
	HR_HDR_ELON,
	HR_HDR_PUNI,
	HR_HDR_NOBJ,
	HR_HDR_END
}HeaderRequest;

typedef enum _DisplayStyle
{
        ENCL_DS_STANDARD,
        ENCL_DS_PAPER,
}EnclDisplayStyle;

typedef enum _PointSymbolType
{
        ENCL_SIMPLIFIED_POINT,
        ENCL_TRADITIONAL_POINT,
}EnclPointSymbolType;

typedef enum _AreaBoundaryType
{
        ENCL_PLAIN_BOUNDARIES,
        ENCL_SYMBOLIZED_BOUNDARIES,
}EnclBoundarySymbolType;

typedef enum _LineSymbolType
{
        ENCL_IHO_LINE,
}EnclLineSymbolStyle;

typedef enum _Datum
{
	ENCL_DATUM_WGS84,
	ENCL_DATUM_CGCS2000
}EnclDatum;

typedef enum _ProjectionMethod
{
	ENCL_PROJECTION_MERCATOR,
	ENCL_PROJECTION_GNOMONIC,
	ENCL_PROJECTION_UPS,
	ENCL_PROJECTION_RADAR,
	ENCL_PROJECTION_GNOMONIC_NOUSE,
	ENCL_PROJECTION_TRANSVERSE_MERCATOR
}EnclProjection;

#define VP_CLAT			0x80000000L 
#define VP_CLON			0x40000000L 
#define VP_ROTATION		0x20000000L 
#define VP_PIXWIDTH		0x10000000L 
#define VP_PIXHEIGHT		0x08000000L 
#define VP_TOP		    0x04000000L
#define VP_LEFT 			0x02000000L
#define VP_SCALE			0x01000000L
#define VP_SCALEPPM		0x00800000L 
#define VP_PROJECTION		0x00400000L
#define VP_ALL			0xFFC00000L 

typedef struct _EnclViewPort
{
	unsigned int nMask;
	double centerLat;
	double centerLon;
	double viewScale;
	double scaleppm;
	int width;
	int height;
	int left;
	int top;
	double bearing;
	EnclProjection projection;
}EnclViewPort;

typedef enum _AgenType
{
	AGEN_IHO_MEMBER_STATES,		//IHO Member States
	AGEN_OTHER_STATES,			//Other States
	AGEN_OTHER_ENTITIES,		//Other Organizations/Entities
	AGEN_UNDEF					//undefined
}AgenType;

typedef struct _NotifiViewGroup
{
	int ngroups;			// 0: all n: groups total numbers
	int *groups;			// pointer to groups, 0: all groups
}NotifiViewGroup;

// For DMS encode / decode

typedef enum {DMS_LATITUDE = 1, DMS_LONGITUDE = 2}DmsFlag;
typedef enum {DMS_DEGREE = 0, DMS_MINUTE = 1, DMS_SECOND = 2}DmsComponent;

/* exchange set file flags */
#define ENCL_EXSETFLAG_COMPRESSED	1
#define ENCL_EXSETFLAG_ENCRYPTED	(1<<1)
#define ENCL_EXSETFLAG_HASCRC		(1<<2)
#define ENCL_EXSETFLAG_HASBBOX	(1<<3)

/* enumeration for exchange set types */
typedef enum
{
	ExSetType_Unknown	= 0, /* normal, search all enc in current dir */
	ExSetType_S57		= 1, /* plain S-57, suchas: <Power>(include ENC_ROOT) */
	ExSetType_S63		= 2, /* standard S-63, such as: <V01X01>(include ENC_ROOT)*/
	ExSetType_S63Large	= 3,  /* S-63 large media, such as: <M01X01>(include MEDIA.TXT) */
	ExSetType_S63LargeSub	= 4,  /* S-63 large media sub, suchas: <M01X01/B1> */
} EnclDENCExchangeSetType;

/* enumeration for file types */
typedef enum
{
	ExSetFileType_Unknown	=  0,
	ExSetFileType_Catalog	=  1, /* exchange set catalog */
	ExSetFileType_DataSet	=  2, /* base cell or update */
	ExSetFileType_Picture	=  3, /* referenced picture (TIFF) file */
	ExSetFileType_Text		=  4, /* referenced text (TXT) file */
	ExSetFileType_PDF		=  5, /* referenced PDF (PDF) file */
	ExSetFileType_HTM		=  6, /* referenced HTML (HTM) file */
	ExSetFileType_JPG		=  7, /* referenced JPEG (JPG) file */
	ExSetFileType_AVI		=  8, /* referenced AVI (AVI) file */
	ExSetFileType_MPG		=  9, /* referenced MPEG (MPG) file */
	ExSetFileType_Signature	= 16, /* signature file */
	ExSetFileType_SENCUpdate  = 30, /* SENC update file */
} EnclDENCFileType;

/* structure for exchange set entry */
typedef struct _EcDENCExchangeSetEntry
{
	EnclDENCFileType	fileType;
	char *				fileName;
	char *				subDir;
	int					volume;
	int					flags;    /* compressed, encrypted, hasCRC, hasBoundingBox */
	unsigned int		crcValue; /* CRC of file */
	double				minLat, minLon, maxLat, maxLon; /* bounding box */
	char *				comment;
} EnclDENCExchangeSetEntry;

typedef struct
{
	EnclDENCExchangeSetType type;
	char *rootpath;
	int nentries;
	EnclDENCExchangeSetEntry ** entryinfo;
	int nvols;
	char *comment;
}EnclDENCExchangeSet;

typedef enum _EnclChartPermitType
{
	ENCL_S63_PERMIT  = 1,
	ENCL_SENC_PERMIT = 2,
	ENCL_ARCS_PERMIT = 3
} EnclChartPermitType;

typedef enum
{
	// The sequence of the value matters. DO NOT change it.
	ENCL_CAT_NAVI_HAZARDS = 0,			// Navigational hazards, must be 0.
	ENCL_CAT_SAFETY_CONTOUR,		// Safety contour
	ENCL_CAT_TRAFFIC_SEPARATION_ZONE,	// Traffic separation zone
	ENCL_CAT_INSHORE_TRAFFIC_ZONE,		// Inshore traffic zone
	ENCL_CAT_RESTRICTED_AREA,			// Restricted area
	ENCL_CAT_CAUTION_AREA,				// Caution area
	ENCL_CAT_OFFSHORE_PRODUCTION_AREA,	// Offshore production area
	ENCL_CAT_AREAS_TO_BE_AVOIDED,		// Areas to be avoided
	ENCL_CAT_MILITARY_PRACTICE_AREA,	// Military practice area
	ENCL_CAT_SEAPLANE_LAND_AREA,		// Seaplane landing area
	ENCL_CAT_SUBMARINE_TRANSIT_LANE,	// Submarine transit lane
	ENCL_CAT_ANCHORAGE_AREA,			// Anchorage area
	ENCL_CAT_MARINE_FARM_CULTURE,		// Marine farm/aquaculture
	ENCL_CAT_PSSA						// Particularly Sensitive Sea Area
}EnclChartAlertType;

typedef enum _EnclPrimitiveType
{
	ENCL_P_PRIM,
	ENCL_L_PRIM,
	ENCL_A_PRIM,
	ENCL_NONE
}EnclPrimitiveType;

typedef enum _EnclS57ClassAttrType
{
	ENCL_SAT_ENUM = 'E',
	ENCL_SAT_LIST = 'L',
	ENCL_SAT_FLOAT = 'F',
	ENCL_SAT_INT = 'I',
	ENCL_SAT_CODE_STRING = 'A',
	ENCL_SAT_FREE_TEXT = 'S',
}EnclS57ClassAttrType;

typedef struct _EnclS57ClassAttrInfo
{
	int attrcode;
	char e_desc[128];
	char c_desc[256];
	char acronym[7];
	EnclS57ClassAttrType type;	//E(enum), L(list), I(int), F(float), A(string), S(free string)
	char classtype;		//F: Feature Object Attributes
						//N: National Language Attributes
						//S: Spatial and Meta Object Attributes
	char unit[16];		//m, degree...
	char resolution[16];	//0.1, 0.01...
}EnclS57ClassAttrInfo;

typedef struct _EnclS57ClassEnumInfo
{
	int attrcode;
	int	enumcode;
	char e_desc[256];
	char c_desc[256];
	char abbr[16];
}EnclS57ClassEnumInfo;

typedef struct _EnclS57ClassInfo
{
	int classcode;
	EnclPrimitiveType primitivetype[3];
	char acronym[7];
	char e_desc[128];
	char c_desc[256];
}EnclS57ClassInfo;

typedef void (*CustomOGLRenderCallBack)(void * param);

typedef struct _EnclRoute
{
	int ptCount;				// way points numbers
	double *cords;			// way points: lon1, lat1, lon2, lat2...
}EnclRoute;

#pragma pack(pop)

#endif
