#ifndef _RC_ECDIS_SDK_H_
#define _RC_ECDIS_SDK_H_

/*! \file rcecdissdk.h
    \brief Main include file .
*/ 

#include "nmea.h"
#include "commonDef.h"
#include "ownershipDef.h"
#include "AISObjectDefine.h"
#include "TTObjectDefine.h"
#include "S57ManualUpdateDef.h"
#include "S57UserChartDef.h"
#include "MarinerObjDef.h"
#include "Route_WayPointDef.h"
#include "s63defs.h"
#include "importcallbackdefs.h"
#include "s57catalog.h"
#include "alert.h"

#ifdef _WINNT_SOURCE
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
#endif

#if defined __MSW__ 
	#ifdef ECDISSDK_LIB
		#define CORE_EXPORT __declspec(dllexport) 
	#else
		#define CORE_EXPORT __declspec(dllimport) 
	#endif
#else
	#ifdef __LINUX__
		#ifdef ECDISSDK_LIB
			#define CORE_EXPORT __attribute__ ((visibility ("default"))) 
		#else
			#define CORE_EXPORT
		#endif
	#else
		#define CORE_EXPORT
	#endif
#endif

#define ECDISSDK_MAJOR       1
#define ECDISSDK_MINOR       0
#define ECDISSDK_SUBMINOR    0

#ifndef TYPE_HDC

#ifdef __LINUX__
    typedef char * _HDC;
#elif __MSW__
	#include <windows.h>
	typedef HDC _HDC;
#else
	typedef void* _HDC;
#endif

#endif

typedef int _HRESULT;

typedef enum
{
	WPPara_id = 0,
	WPPara_Name,
	WPPara_Position_lat,
	WPPara_Position_lon,
	WPPara_TurningRadius,
	WPPara_XTD,
	WPPara_LeglineType,
	WPPara_Stop,
	WPPara_Speed,
	WPPara_Type,
	WPPara_RudderAngle,
	WPPara_UsrReMark,
	WPPara_WPPointReMark,
}WayPointParameter;

typedef enum
{
	RoutePara_Id,
	RoutePara_Name,
	RoutePara_ActiveWPId,
	RoutePara_Highlight,
	RoutePara_Monitored,
	RoutePara_Usermark,
	RoutePara_WayPoints,
}RouteParameter;

const unsigned int ERROR_SDK001 = 111; /* "Cannot read Exchange Set Catalog" */
const unsigned int ERROR_SDK002 = 112; /*Exchange set not complete*/
const unsigned int ERROR_SDK003 = 113; /*Import failed*/
const unsigned int ERROR_SDK_SUCCESS = 1; /*No error*/

typedef enum _Stabilization
{
	Stab_Ground = 1,
	Stab_Water = 2,
}Stabilization;


class CORE_EXPORT RcSDKInit
{
public:
       static bool Initialize(const char *szMapDataPath);
       static void Deinitialize();
	   static void SetChartRenderDPI(int dpi);
};

class CORE_EXPORT RcChartViewMgr
{
public:
	static RcChartViewMgr* CreateInstance();

	static RcChartViewMgr* GetInstance();

	virtual unsigned int CreateView() = 0;

	virtual bool SetCurrentView(unsigned int viewId) = 0;

	virtual unsigned int GetCurrentViewId() = 0;

	virtual int GetViewCount() = 0;

	virtual int* GetViewIds(int & nCnt) = 0;

	virtual unsigned char * DrawView(_HDC hdc)=0;

	virtual char * GetEngineTypeInfoString()=0;

	virtual bool IsLocationInsideViewPort(double lat, double lon) = 0;

	virtual void SetViewPort(const EnclViewPort & viewPort)=0;

	virtual double GetViewRange() = 0;

	virtual void SetViewRange(double rangeInNm) = 0;

	virtual void ScaleAroundLocation(double lat, double lon, double chartScale)=0;

	virtual void GetViewPort(EnclViewPort & viewPort)=0;

	virtual bool IsViewPortCrossedAntiMeridian() = 0;

	virtual void UnloadChart(char* pszChartName) = 0;

	virtual bool ShowLayer(const char * layerName, bool bShow)=0;

	static void GetS52Color( const char* pszColorName, const char * cs, unsigned char *R, unsigned char* G, unsigned char * B);

	virtual void HighlightAutomaticUpdate(const char * pszChartName, int updateNumber) = 0;

	#ifdef USE_OPENGL 
	virtual void SetCustomRenderCallBack(CustomOGLRenderCallBack  oglRenderCallBack, void * pParam) = 0;
	#endif
	
	virtual void SetShowChartName( bool bShow) = 0;

	virtual void SetS57SymbolScale( double dScale) = 0;

	virtual bool SetChartLoadMode(EnclChartLoadMode mode) = 0;

	virtual EnclChartLoadMode GetChartLoadMode() = 0;

	virtual bool ManualLoadChart(const char * pChartName, bool bLoad) = 0;
	
	virtual void DrawCustomText(const char * str, int x, int y, int size, unsigned char R,
		unsigned char G, unsigned char B) = 0;
	virtual void ClearCustomText() = 0;
protected:
	RcChartViewMgr();
	virtual ~RcChartViewMgr();
};


class CORE_EXPORT RcChartMgr
{
public:
	static 	RcChartMgr* GetInstance();

	virtual bool SetSENCDirectory( const char* pszSENCDir) = 0;

	virtual void GetSENCDirectory( char* pszSENCDir, int maxBufSize ) = 0;

	virtual void GetHWID( char* hwid ) = 0;

	virtual bool DeleteSENCFile(const char* pszChartName)=0;

	virtual int GetSENCFileCount()=0;

    virtual char** GetSENCFileList(int & nFiles,bool forceReload=false)=0;
	
    virtual void FreeSENCFileList(char** pFiles, int nFiles)=0;

	virtual bool S57EncImportFile(const char *encfile, const unsigned int *crc, int *errnum)=0;
	virtual bool S57EncImportTree(const char *encpath,
									  EnclSENCImportErrCallBack callback,
									  int *errnum)=0;

	virtual bool S63EncImportTree(const unsigned char *hw_id,
									  const char *encpath,
									  const char *permitfile,
									  const char *crtfile,
									  EnclSENCImportErrCallBack callback,
									  int *errnum)=0;
	virtual bool S63EncDecryptExt(const unsigned char *hw_id,
									  const char *encpath,
									  const char *permitfile,
									  const char *crtfile,
									  EnclSENCImportErrCallBack callback,
									  int *errnum)=0;

	virtual EnclS63ProductsCatalog *S63EncCreateProductCatalog()=0;
	virtual void S63EncFreeProductCatalog(EnclS63ProductsCatalog *prodCat)=0;
	virtual bool S63EncReadProductCatalog(const char *prodfile, const char *ds, EnclS63ProductsCatalog *prodcat)=0;
	virtual bool S63EncWriteProductCatalog(const char *prodfile, const EnclS63ProductsCatalog *prodcat)=0;
	virtual bool S63EncGetProductCatalogInfo(const EnclS63ProductsCatalog *prodCat, EnclTimeAndDate *date, int *version, EnclS63ProductsCatalogType *content, EnclS63ProductsCatalogInfo **prodInfo, int *nInfos)=0;
	virtual bool S63EncDelProductCatalogInfo(EnclS63ProductsCatalog *prodcat, const char *cellname, const char *ds)=0;
	virtual EnclS63StatusLst * S63EncReadStatusLst(const char *statusfile)=0;
	virtual void S63EncFreeStatusLst(EnclS63StatusLst *stlst)=0;
	virtual EnclS63MediaLst * S63EncReadMediaLst(const char *mediafile)=0;
	virtual void S63EncFreeMediaLst(EnclS63MediaLst *medialst)=0;
	virtual bool S63EncReadExchangeSetInfo(const char *serialfile, EnclS63ExchangeSetInfo *esInfo)=0;
	virtual bool S63EncCheckBaseCompatible(const char *rootpath, EnclS63CellPermitList * permitlist, char *msg)=0;
	virtual bool S63EncCheckNewer(const char *rootpath, const char *cellname)=0;

    virtual EnclDENCExchangeSet *DENCExchangeSetReadCatalog( const char *catalogName, FILE *errlog, bool sortFlag )=0;
    virtual void DENCExchangeSetFreeCatalog( EnclDENCExchangeSet *exchSet )=0;
    virtual EnclDENCExchangeSetType DENCExchangeSetGetType( const EnclDENCExchangeSet *exchSet )=0;
    virtual const char *DENCExchangeSetGetRootPath( const EnclDENCExchangeSet *exchSet )=0;
    virtual const char *DENCExchangeSetGetComment( const EnclDENCExchangeSet *exchSet )=0;
    virtual int DENCExchangeSetGetNumberOfEntries( const EnclDENCExchangeSet *exchSet, int *numOfVolumes )=0;
    virtual const EnclDENCExchangeSetEntry *DENCExchangeSetGetEntry( const EnclDENCExchangeSet *exchSet, int index )=0;
    virtual EnclDENCExchangeSetType DENCGetExchangeSetType( const char *importPath )=0;
    virtual bool DENCEncGetHeaderInfo(const char * encfile, EnclHeaderRequest *header)=0;
    virtual bool DENCIsStandardENCFileName(const char *fileName)=0;

    virtual int S63PermitListGetPermitsByName(const EnclS63CellPermitList *permList,
                                         const char *nameOrPid,
                                         EnclChartPermitType permType,
                                         EnclS63CellPermitInfo **permitInfos)=0;

	virtual EnclHeaderRequest GetChartHeader(const char* chartName)=0;

	virtual const char * TranslateHeaderRequest(HeaderRequest hr) = 0;

	virtual AutoUpdateRecord* GetAutoUpdateData(const char * chartName,int &nCnt) = 0;

	virtual void FreeAutoUpdateData( AutoUpdateRecord * pRcds) = 0;

	virtual int S57EncReadCell(const char *encFile,
									const unsigned int *checkCrc,
									bool (*percentFunc)(int, const char *))=0;
	virtual int S63EncDecryptCell(const char *pszENCFilePath, const EnclS63CellPermitInfo *permitinfo, const unsigned char *hw_id, const char* destDir)=0;
	virtual int S63EncReadCell(const char *encFile,
									const EnclS63CellPermitInfo *permitinfo,
									const unsigned char *hw_id,
									const unsigned int *checkCrc,
									bool (*percentFunc)(int, const char *))=0;
	virtual void S63EncGetSignatureName(const char *encfile, char *sigfile)=0;

	virtual EnclS63CellPermitList * S63PermitListCreate()=0;
        virtual void S63PermitListFree(EnclS63CellPermitList *permitlist)=0;

	virtual bool S63PermitListDelPermit(EnclS63CellPermitList *permitlist, const EnclS63CellPermitInfo *permitinfo)=0;

        virtual bool S63PermitListAddPermit(EnclS63CellPermitList *permitlist, const EnclS63CellPermitInfo *permitinfo, bool mergedflag)=0;

	virtual int S63PermitCheckHWID(const EnclS63CellPermitInfo *permitinfo, const unsigned char *hw_id)=0;
	
	virtual int S63PermitRead(const char *permitfile, EnclS63CellPermitInfo **permitinfo, int *ninfos, EnclS63PermitCallback callback, void *cbdata)=0;

	virtual int S63PermitListAddedFromFile(EnclS63CellPermitList * permitlist, const char *permitfile, const unsigned char *hw_id, bool replaceflag, EnclS63PermitCallback callback, void *cbdata)=0;

    virtual void S63PermitListWriteToFile(EnclS63CellPermitList * permitlist, const char *permitfile)=0;

	virtual void S63PermitListFreeInfos(EnclS63CellPermitInfo* permitinfo)=0;

	virtual int S63PermitCheckExpiryDate(const EnclS63CellPermitInfo *permitinfo, int *remain)=0;

	virtual int S63PermitCheckExt(const unsigned char *hw_id, const char *permitstring, char *cellname, int *year, int *mon, int *day)=0;

	virtual int S63PermitDecryptExt(const unsigned char *hw_id, const char *permitstring, char *cellname, int *year, int *mon, int *day, char *key1, char *key2)=0;

	virtual bool S63CreateUserPermit(unsigned char* hwId, unsigned char* manKey, unsigned char* manId, char** userPermit)=0;

	virtual void FreeUsrPermit(char* usrPermit)=0;

	virtual bool S63CrtGetString(const char *crtfile, EnclS63CrtAttrType type, char **outstr)=0;

	virtual void S63CrtFreeString(char* str)=0;

	virtual long S63CrtGetSerial(const char *crtfile)=0;

	virtual long S63CrtGetVersion(const char *crtfile)=0;

	virtual int S63CrtCheckDate(const char *crtfile)=0;

	virtual bool S63CrtGetPublicKey(const char *crtfile, unsigned char *p, unsigned char *q, unsigned char *g, unsigned char *y)=0;

	virtual int S63CrtCheckSignature(const char *crtfile, const char *sigfile, const char *encfile)=0;

    virtual int S57ReadCatalogInfo(const char* catalogName, S57DatasetCatalogueInfo& catalog)=0;

    virtual void S57FreeCatalogInfo(S57DatasetCatalogueInfo& catalog )=0;

    virtual bool CheckENCFileCRC(const char* pszENCFileName, const S57DatasetCatalogueInfo& catalog)=0;
    virtual bool CheckENCFileByCRC(const char* pszENCFileName, unsigned int crc)=0;
    virtual bool GetENCFileEdAndUpdNumber(const char* fileName,	const S57DatasetCatalogueInfo& catalog, int *ed, int *upd)=0;

    virtual bool CheckDatasetIntegrity(const char* pszRoot, const S57DatasetCatalogueInfo& catalog)=0;

	virtual ~RcChartMgr();
protected:
	RcChartMgr();
};


class CORE_EXPORT RcNavigationCalculate
{
public:
	
    static void CalculateRhumblineDistanceAndBearing (double lat1, double lon1, double lat2, double lon2, double *bear, double *dist);

	static void CalculateGreatCircleDistanceAndBearing (double lat1, double lon1, double lat2, double lon2, double *dist, double *bear1, double *bear2);
   
	static void CalculateRhumblinePosition (double lat, double lon, double brg, double dist, double *lat1, double *lon1);

    static void CalculateGreatCirclePosition (double lat1, double lon1, double dist, double bear1, double *lat2, double *lon2, double* bear2);

	static void CalculatePolygonPerimeterAndArea(double *lats,double *lons,int count,double &perimeter,double &area);

	static void CalculateRhumbPolygonPerimeterAndArea(double *lats,double *lons,int count,double &perimeter,double &area);

	static double CalculatePolylineDistance(double *lats,double *lons,int count);

	static double CalculateRhumbPolylineDistance(double *lats,double *lons,int count);

	static void CalculateGreatCircleRoute(double lat0, double lon0, double lat1, double lon1, double * &pwpts, int & nwpts);

	static void FreeWptData(double * pwpts);

	static bool LatLontoXY (double lat, double lon, int *x, int *y, EnclViewPort * vp);

	static void LatLonstoXYs (double* lat, double * lon, int *x, int *y, int n, EnclViewPort * vp);

	static void XYtoLatLon (int x, int y, double *lat, double *lon, EnclViewPort * vp);

	static void XYstoLatLons (int *x, int *y, double *lat, double *lon, int n, EnclViewPort * vp);

    static void CalculateCPA (double ownLat,double ownLon, double ownSpeed, double ownCourse, double otherLat, double otherLon, double otherSpeed, double otherCourse, double *ca, double *tcpa, double *cpaLat, double *cpaLon);

    static double CalculateXTE(double lato, double lono, double lata, double lona, double latb, double lonb);

	static double * GenerateGreatCirclePositions(double lat0, double lon0, double lat1, double lon1, int * npts, double intervalInMeter);

	static void FreePositions(double * pPts);

	static double * GenerateRhumbPositions(double lat0, double lon0, double lat1, double lon1, int * npts, double intervalInMeter);

	static void Calculate3DDistanceAndBearing(double lon1, double lat1, double height1, double lon2, double lat2, double height2, double* bear, double* dist);

protected:
	RcNavigationCalculate();
	~RcNavigationCalculate();
};


class CORE_EXPORT RcRouteMgr
{
public:
    virtual ~RcRouteMgr();

    static RcRouteMgr* GetInstance();

	virtual EnclObjectPtr CreateRoute(const char* pszRouteName) = 0;

	virtual bool SaveRoute(EnclObjectPtr rtId) = 0;

	virtual bool IsRouteSaved(EnclObjectPtr rtId) = 0;

	virtual EnclObjectPtr InsertWayPoint( WayPointObjData & wpData, EnclObjectPtr rtId, int pos ) = 0;

    virtual EnclObjectPtr AppendWayPoint( WayPointObjData & wpData, EnclObjectPtr rtId ) = 0;

	virtual EnclObjectPtr * GetWayPointsByName(EnclObjectPtr rtId, const char* pszWpName, int & nWpts) = 0;

	virtual EnclObjectPtr * GetRoutesByName(const char* pszRouteName, int &nRts) = 0;

	virtual bool DeleteWayPoint(EnclObjectPtr rtId, EnclObjectPtr WptID ) = 0;

	virtual bool DeleteRoute(EnclObjectPtr rtId) = 0;

	virtual EnclObjectPtr ReverseRoute(EnclObjectPtr rtId, bool createNewRoute) = 0;

        virtual EnclObjectPtr * GetAllWayPoints(EnclObjectPtr RtID, int& nWpts) = 0;

	// Get one Way point at specified location and its own route if 'pRtId' is not null. Radius in pixels.
	virtual EnclObjectPtr GetWayPointAtLocation(double lat, double lon, double radius, EnclObjectPtr * pRtId) = 0;

	// Get next way point of specified way point on route .
	virtual EnclObjectPtr GetNextWayPoint(EnclObjectPtr rtId, EnclObjectPtr curWaypoint) = 0;

	// Get previous way point of specified way point on route .
	virtual EnclObjectPtr GetPrevWayPoint(EnclObjectPtr rtId, EnclObjectPtr curWaypoint) = 0;

	virtual EnclObjectPtr * GetAllRoutes(int & nRts) = 0;

	virtual bool SetRouteData(EnclObjectPtr routeId, const RouteObjData & rtData) = 0;

	virtual bool GetRouteData(EnclObjectPtr routeId, RouteObjData & rtData) = 0;

	virtual bool SetWayPointData(EnclObjectPtr routeId, EnclObjectPtr wptID, const WayPointObjData & wpData) = 0;

	virtual bool GetWayPointData(EnclObjectPtr routeId, EnclObjectPtr wptID, WayPointObjData & wpData) = 0;

	virtual EnclObjectPtr GetActiveWayPnt(EnclObjectPtr routeId) = 0;

	virtual bool SetActiveWayPnt(EnclObjectPtr routeId, EnclObjectPtr wptId) = 0;

	virtual int GetWayPntIndexByID(EnclObjectPtr routeId, EnclObjectPtr wptId) = 0;

        virtual EnclObjectPtr GetWayPntIDbyIndex (EnclObjectPtr routeId, int index) = 0;

	virtual bool ActivateRoute(EnclObjectPtr routeId, bool bActivate) = 0;

	virtual EnclObjectPtr GetActiveRoute() = 0;

	virtual void SetRouteVisible(EnclObjectPtr routeId, bool bDisplay) = 0;

	virtual bool GetRouteVisible(EnclObjectPtr routeId) = 0;

	virtual void SetRouteAdvancedDistance(EnclObjectPtr routeId, double distInMeter) = 0;

	virtual void ShowRouteWheelOverLine(EnclObjectPtr routeId, bool bShow) = 0;

	virtual int CheckRoute(EnclObjectPtr objID, double xtdMeter) = 0;

	virtual void ReloadAllRoutes() = 0;

	virtual double CalculateDTGBeforeArriveWaypoint(EnclObjectPtr rtId, EnclObjectPtr wptId, double lonShip, double latShip,
		double course, double * arriveLon, double * arriveLat) = 0;

	virtual EnclObjectPtr CalculateNextWayPoint(EnclObjectPtr rtId, double lat, double lon, double speed, double course) = 0;

	virtual EnclObjectPtr IsAnyWaypointArrived(EnclObjectPtr rtId, double lat, double lon) = 0;

	virtual bool IsWaypointArrived(EnclObjectPtr rtId, EnclObjectPtr wptId, double lat, double lon) = 0;

	virtual char* ExportRouteToXml(EnclObjectPtr rtId, int &nlen) = 0;

	virtual EnclObjectPtr ImportRouteFromXml(char * xml) = 0;
protected:
    RcRouteMgr();
};


class CORE_EXPORT RcNMEA
{
public:
    static RcNMEA* GetInstance();

    virtual bool ReadNMEA(SensorProtocol protocol, const char* msg, RcNMEAData& nmea)=0;

	virtual char* WriteNMEA(const RcNMEAData& nmea) = 0;

	virtual void FreeNMEASentences(char *sentences) = 0;
protected:
    RcNMEA();
	virtual ~RcNMEA();
};

class CORE_EXPORT RcAlertManager
{
public:
    static RcAlertManager* GetInstance();

	virtual EnclObjectPtr NewAlert(int alertId, int alertInst) = 0;

	virtual char* GetAlertALFSentences(EnclObjectPtr alert) = 0;

	virtual char* GetALCSentences() = 0;
	
	virtual void UpdateAlerts() = 0;

	virtual EnclObjectPtr* GetAllAlerts(int & n) = 0;

	virtual void GetAlertInfo(EnclObjectPtr id, EnclAlert& alertInfo) = 0;

	virtual EnclObjectPtr GetAlert(int alertId, int alertInstance) = 0;

	virtual bool AcknowledgeAlert(EnclObjectPtr id, bool remote) = 0;

	virtual void SilenceAlert(EnclObjectPtr id) = 0;
	
	virtual void ClearAlert(EnclObjectPtr id) = 0;

	virtual void RectifyAlert(EnclObjectPtr id) = 0;

	virtual void TransferAlert(EnclObjectPtr id) = 0;

	virtual EnclAlertTableEntry* GetAlertInfo(int alertId, int alertInst) = 0;

	virtual void AddAlertDefinition(const EnclAlertTableEntry & entry) = 0;

protected:
    RcAlertManager();
	virtual ~RcAlertManager();
};

class CORE_EXPORT RcDataQuery
{
public:
    static RcDataQuery* GetInstance();

	static bool IsIDEqual(EnclObjectPtr & ID1, EnclObjectPtr & ID2);

	virtual EnclObjectPtr* QueryObjects(double lat, double lon, double pixradius, const char* ObjCate, int& nObjs)=0;

	virtual EnclObjectPtr* QueryObjectsInPolygon(double *lons, double *lats, int pointCount, int *objCount) = 0;

	virtual EnclObjectPtr QueryChartObject(const char* chartName)=0;

	virtual EnclObjectPtr* QueryAllObjectsInChart(EnclObjectPtr chartId, int * nCnt)=0;

    virtual EnclObjectPtr* QueryFindObjectsInChart(EnclObjectPtr chartId, const char *className,
                                                   const char *attr, int attrLen, int * nCnt)=0;

    virtual EnclObjectPtr* QueryDisplayedChartsAtPos(int pickX, int pickY, int& nObjs)=0;

	virtual EnclObjectPtr* QueryDisplayedChartsAtLonLat(double lon, double lat, int& nObjs)=0;

	virtual EnclObjectPtr* QueryInstalledChartsAtLonLat(double lon, double lat, int& nObjs)=0;

	virtual EnclObjectPtr QueryOwnerChart(EnclObjectPtr pObj) = 0;

    virtual EnclObjectPtr* GetLoadedCharts( int & nObjs) = 0;

    virtual EnclObjectPtr* QueryDisplayedCharts(int & nObjs) = 0;

	virtual EnclObjectPtr* GetChildObjects( const char* objCate, EnclObjectPtr parentObjId, int & nObjs) = 0;

	virtual void SetGuardAreaVisible(const char*dataLayerName, int areaId, bool bVisible) = 0;
	
	virtual int DefineGuardArea(const char* dataLayerName, PolyData &guardArea, int areaId = -1) = 0;

	virtual bool UpdateGuardArea(const char* dataLayerName, PolyData &guardArea, int areaId) = 0;

	virtual void RemoveGuardArea(const char * dataLayerName, int areaId) = 0;

	virtual void ConfigChartAlert(EnclChartAlertType type, bool set) = 0;

	virtual bool GetChartAlert(EnclChartAlertType type) = 0;

	virtual DangerObjInfo* ScanForDangerObjects(int iAreaID, int& nDangerObjs, bool bHighlight, int * highlightGrpId) = 0;

	virtual void DehighlightDangerObjectsInGrp(int groupId) = 0;

	virtual void DehighlightAllDangerObjects() = 0;

	// All objects in specific data layer of specified Guard area, not just danger objects.
	virtual EnclObjectPtr * GetObjectsInGuardArea(const char *dataLayerName,int& nObjs, int areaId) = 0;

    virtual void FreeDangerObjInfo(DangerObjInfo* dangerInfo) = 0;

	virtual double GetWaterDepthAtPosition(double lat, double lon) = 0;

	virtual double GetLandElevationAtPosition(double lat, double lon) = 0;

	virtual double GetHighestLandElevationInLandArea(double * lons, double *lats, int npoints) = 0;

    static  void FreeObjectIDs(EnclObjectPtr* objIds);

	virtual char* GetObjectAttributes(EnclObjectPtr objID, const char* objType)=0;

	virtual const char* GetObjectTXTDSC(EnclObjectPtr objID, char* NtxtFile, char* txtFile)=0;

	virtual const char* GetObjectPicture(EnclObjectPtr objID, char* picName, int* width, int* height)=0;

	virtual EnclObjectGeoData * GetObjectGeoData(EnclObjectPtr objID, const char* objType)=0;

	virtual void FreeGeoData(EnclObjectGeoData * pData) = 0;
	
	virtual void FreeAttribute(char* data, const char* type)=0;

	virtual void FreeTXTDSC(char* data, const char* type)=0;

	virtual void FreePicture(unsigned char* data, const char* type)=0;

    static int TranslateAttributeValue(char* attrCode, char* buffer, int bufferLen);

	virtual void HighlightObject(EnclObjectPtr objID, const char * objType) = 0;

	virtual void DehighlightObject(EnclObjectPtr objID, const char * objType) = 0;

	virtual void DehighlightAll() = 0;

	virtual EnclS57ClassInfo* S57ClassGetAllClass(int &nCnt )=0;
	virtual void S57ClassFreeAllClass(EnclS57ClassInfo * pInfos) =0;
	virtual EnclS57ClassAttrInfo* S57ClassGetAllAttr(int &nCnt )=0;
	virtual void S57ClassFreeAllAttr(EnclS57ClassAttrInfo * pInfos) =0;
	virtual bool S57ClassGetAttrcodesByClass(int classcode, int **attrcodes, int *attrnum) =0;
	virtual bool S57ClassGetMandatoryAttrcodesByClass(int classcode, int **attrcodes, int *attrnum) =0;
	virtual void S57ClassFreeAttrcodes(int *attrcodes) =0;
	virtual bool S57ClassGetAttrInfo(int attrcode, EnclS57ClassAttrInfo *attrinfo) =0;
	virtual bool S57ClassGetEnumsByAttrcode(int attrcode, int **enums, int *valnum) =0;
	virtual void S57ClassFreeEnums(int *enums) =0;
	virtual bool S57ClassGetEnumInfo(int attrcode, int enumcode, EnclS57ClassEnumInfo *enuminfo) =0;
    virtual double *GetLandAreaCrossPoints(double lat0, double lon0, double lat1, double lon1, int* npts) = 0;
    virtual double *GetLandElevationCrossPoints(double lat0, double lon0, double lat1, double lon1, int* npts) = 0;
	virtual EnclObjectPtr * GetLandObjectsInPolygon(double * lons, double *lats, int npoints, int *nObjects) = 0;
	virtual EnclObjectGeoData * GetLandObjectAreaInsidePolygon(EnclObjectPtr targetObj, double * lons, double *lats, int npoints, int *nAreas) = 0;
	virtual void FreeObjectGeoDataArray(EnclObjectGeoData * dataArray) = 0;
protected:
    RcDataQuery();
    virtual ~RcDataQuery();
};

class CORE_EXPORT RcCustomMarker
{
public:
	virtual ~RcCustomMarker(){}

	static RcCustomMarker* CreateInstance(const char* markerType);

	virtual EnclObjectPtr CreateSymbol(void* pData)=0;

	virtual void DestroySymbol(EnclObjectPtr id)=0;

	virtual const char* GetSymbolsSupported(int & nCnt)=0;

	virtual EnclObjectPtr* GetAllSymbolsID(int& nObjs)=0;

	virtual void GetSymbolAttr(EnclObjectPtr id, void* attr)=0;

	virtual void SetSymbolAttr(EnclObjectPtr id, void* attr)=0;
protected:
    RcCustomMarker(){}
};

class CORE_EXPORT RcAIS
{
public:
	virtual ~RcAIS(){}
	static RcAIS* GetInstance();

	virtual EnclObjectPtr CreateAISTarget(RcNMEAData* data, int *tgtmmsi)=0;

	virtual void SetAISTargetAttr(EnclObjectPtr id, const AISTargetStatus& status)=0;

	virtual EnclObjectPtr* GetAISTargetsID(int& nObjs)=0;

	virtual void GetAISTargetInfo(EnclObjectPtr id, RcAISTargetInfo* targetInfo)=0;

	virtual void GetAISTargetInfoByMMSI(int mmsi, RcAISTargetInfo* targetInfo)=0;

	virtual char * GetAISNavigationStatusString(int status)=0;

	virtual char * GetAISShipTypeString(int type)=0;

	virtual char * GetAISClassString(int classno)=0;

	virtual void GetReportTime(ais_transponder_class type, int navstatus, int cs, double speed, int *imotime, int *losttime, int *autodeltime)=0;

	virtual void SetTargetShow(bool bShow, int mmsi) = 0;

	virtual void DeleteTarget(int mmsi) = 0;

	virtual void SetTargetActive(bool active, int mmsi) = 0;

	virtual void SetTargetDanger(bool danger, int mmsi) = 0;

	virtual void SetTargetLost(bool lost, int mmsi) = 0;

	virtual void SetTargetSelected(bool select, int mmsi, int idx) = 0;

	virtual bool SetTargetAssociated(bool associated, int mmsi) = 0;

	virtual void SetTargetShowAssociated(bool showassociated, int mmsi) = 0;

	virtual void SetTargetLabel(bool bShow, int mmsi) = 0;

	virtual void SetTargetHeadingline(bool bShow, int mmsi) = 0;

	virtual void SetTargetFullScaleSymbol(bool bFullScale, int mmsi) = 0;

	virtual void SetTargetVelocityVector(bool bShow, int mmsi) = 0;

	virtual void SetTargetVelocityVectorTime(double min, int mmsi) = 0;

	virtual void SetTargetVelocityVectoryTimeInc(double min, int mmsi) = 0;

	virtual void SetTargetTimeIncMarker(bool bShow, int mmsi) = 0;

	virtual void SetTargetTurnIndicator(bool bShow, int mmsi) = 0;

	virtual void SetTargetPastTrack(bool bShow, int mmsi) = 0;

	virtual void SetTargetPastTrackLength(double tInMin, int mmsi) = 0;

	virtual void SetTargetPastTrackTimeLabelInterval(double tInMin, int mmsi) = 0;

	virtual void SetTargetPaskTrackStyle(PastTrackAisStyle pts, int mmsi) = 0;

protected:
	RcAIS(){}
};

class CORE_EXPORT RcTT
{
public:
	virtual ~RcTT(){}
	static RcTT* GetInstance();

	virtual EnclObjectPtr CreateTTTarget(RcNMEAData* data, int *tgtnum)=0;

	virtual void SetTTTargetAttr(EnclObjectPtr id, const TTTargetStatus& status)=0;

	virtual EnclObjectPtr* GetTTTargetsID(int& nObjs)=0;

	virtual void GetTTTargetInfo(EnclObjectPtr id, RcTTTargetInfo* targetInfo)=0;

	virtual void GetTTTargetInfoByNum(int num, RcTTTargetInfo* targetInfo)=0;

	virtual void SetTargetShow(bool bShow, int num) = 0;

	virtual void DeleteTarget(int num) = 0;

	virtual void SetTargetDanger(bool danger, int num) = 0;

	virtual void SetTargetLost(bool lost, int num) = 0;

	virtual void SetTargetSelected(bool select, int num, int idx) = 0;

	virtual bool SetTargetAssociated(bool associated, int num) = 0;

	virtual void SetTargetShowAssociated(bool showassociated, int num) = 0;

	virtual void SetTargetLabel(bool bShow, int num) = 0;

	virtual void SetTargetVelocityVector(bool bShow, int num) = 0;

	virtual void SetTargetVelocityVectorTime(double min, int num) = 0;

	virtual void SetTargetVelocityVectoryTimeInc(double min, int num) = 0;

	virtual void SetTargetTimeIncMarker(bool bShow, int num) = 0;

	virtual void SetTargetPastTrack(bool bShow, int num) = 0;

	virtual void SetTargetPastTrackLength(double tInMin, int num) = 0;

	virtual void SetTargetPastTrackTimeLabelInterval(double tInMin, int num) = 0;

	virtual void SetTargetPaskTrackStyle(PastTrackTtStyle pts, int num) = 0;

protected:
	RcTT(){}
};

class CORE_EXPORT RcOwnship
{
public:
	virtual ~RcOwnship(){}

	static RcOwnship* GetInstance();
public:
	virtual void CreateOwnship() = 0;

	virtual void DeleteOwnship() = 0;

	virtual void SetShipPosition(double lat, double lon) = 0;

	virtual void GetShipPosition(double *lat, double *lon) = 0;

	virtual void SetShipHeading(double heading) = 0;

	virtual double GetShipHeading() = 0;

	virtual void SetShipDimension(ShipDimension sd) = 0;

	virtual ShipDimension GetShipDimension() = 0;

	virtual void SetShipCCRPOffset(double dx, double dy) = 0;

	virtual void GetShipCCRPOffset(double &dx, double &dy) = 0;

	virtual void SetShipCOG(double cog) = 0;

	virtual double GetShipCOG() = 0;
	
	virtual void SetShipSOG(double sog) = 0;

	virtual double GetShipSOG() = 0;

	virtual void SetShipCTW(double ctw) = 0;

	virtual double GetShipCTW() = 0;

	virtual void SetShipSTW(double stw) = 0;

	virtual double GetShipSTW() = 0;

	virtual void ShowBeamLine(bool bShow) = 0;

	virtual void ShowHeadingLine(bool bShow) = 0;

	virtual void ShowFullScaleSymbol(bool bFullScale) = 0;

	virtual void ShowVelocityVector(bool bShow) = 0;

	virtual void SetShipVelocityVectorTime(double min) = 0;

	virtual void SetShipVelocityVectoryTimeInc(double min) = 0;

	virtual void ShowShipTimeIncMarker(bool bShow) = 0;

	virtual void ShowShipStabilizeIndicator(bool bShow) = 0;

	virtual void ShowShipWaterRefVector(bool bShow) = 0;

	virtual void ShowShipGroundRefVector(bool bShow) = 0;

	virtual void AddPastTrackPosition(PastTrackInfo & pti) = 0;

	virtual void ShowOwnshipPastTrack(bool bShow) = 0;

	virtual void SetPastTrackLength(double tInMin) = 0;

	virtual void SetPastTrackTimeLabelInterval(double tInMin) = 0;

	virtual void SetShowPastTrackTimeLabel(bool bShow) = 0;

	virtual void SetPaskTrackStyle(PastTrackStyle pts) = 0;

	virtual PastTrackInfo * GetAllPastTrackData(int &size) = 0;

	virtual PastTrackInfo * GetPastTrackByTime(int64_t start, int64_t end, int & size, 
		PositionSource ps) = 0;
	
	virtual void SetShowPrimaryTrack(bool bShow) = 0;

	virtual void SetShowSecondaryTrack(bool bShow) = 0;

	virtual void FreePastTrackData(PastTrackInfo * data) = 0;

	virtual void DeleteAllPastTrack() = 0;

	virtual void SetShipMovePara(const ShipMovePara& movePara) = 0;

	virtual void GetShipMovePara(ShipMovePara& movePara)=0;

    virtual void DestroyPastTrack(int64_t sec) = 0;

	virtual PastTrackInfo* GetAllPastTracks(long &cont) = 0;

	virtual void FreePastTracks(PastTrackInfo* info)=0;

    virtual void SetPastTrackTimeSpan(int64_t timeBegin, int64_t timeEnd) = 0;

	virtual void SetAnchorWatchPosition(double lat, double lon) = 0;

	virtual void SetAnchorWatchRange(double rangeInMeters) = 0;

	virtual void EnableAnchorWatch() = 0;

	virtual void DisableAnchorWatch() = 0;

	virtual bool QueryAnchorWatch(double * dist, double *bearing) = 0;
protected:
	RcOwnship(){}
};

class CORE_EXPORT RcRouteMonitor
{
public:
	virtual~ RcRouteMonitor(){}

	static RcRouteMonitor* GetInstance();

	virtual double CalculateXTE(EnclObjectPtr RtID) = 0;

	virtual double CalculateBTW_DTW(EnclObjectPtr RtID, double * BTW ) = 0;

	virtual void CalculatePrediction(double time, double& predictLat, double& predictLon) = 0;
protected:
	RcRouteMonitor(){}
};

class CORE_EXPORT RcMarinerSettings
{
public:
    static RcMarinerSettings* GetInstance();

	virtual void ResetToDefault() = 0;

	virtual void SetShowOutlinedShipSymbol(bool bFlag)=0;

    virtual void SetShowOneMinTimeMark(bool bFlag)=0;

	virtual void SetStabilizationType(Stabilization stabilize)=0;

	virtual void SetPredictionVectorLength(int minutes)=0;

    virtual void SetShowPasttrack(bool bFalg)=0;

	virtual void SetShipDimension(double length, double beam, int draught, int airdraught)=0;

    virtual bool SetColorScheme(const char* scheme)=0;

    virtual void SetPasttrackTimemarkInterval(int nMinutes)=0;

	virtual void SetRouteDistanceTag(int distanceInNM)=0;

    virtual void SetPointSymbolStyle(EnclPointSymbolType type)=0;

    virtual void SetDisplayStyle(EnclDisplayStyle type)=0;

    virtual void SetDisplayNationalLanguage(bool bFlag)=0;

    virtual void SetDisplayChineseLanguage(bool bFlag)=0;

	virtual void SetShowSoundings(bool bFlag)=0;

	virtual void SetSafetyDepth(double value)=0;

	virtual void SetSafetyContour(double value)=0;

	virtual void SetShallowContour(double value)=0;

	virtual void SetDeepContour(double value)=0;

	virtual void EnableSCAMIN(bool bFlag)=0;

    virtual void SetShowTwoDepthShades(bool bFlag)=0;

    virtual void SetSymbolizedAreaBoundaries(EnclBoundarySymbolType type)=0;

    virtual void SetLineSymbolStyle(EnclLineSymbolStyle style)=0;

    virtual void SetDisplayCategory(EnclDisplayCategory disCat)=0;

    virtual bool SetViewGroups(int* piGrp, int nGrp, EnclViewingGroupAction vga)=0;

    virtual bool SetViewGroupLayers(int* piLyrs, int nlyrs, EnclViewingGroupAction vga) = 0;

    virtual bool SetTextGroups(int* piGrp, int nGrp, EnclTextGroupAction tga) = 0;

    virtual bool SetTextGroupLayers(int* piLyrs, int nlyrs, EnclTextGroupAction vga) = 0;

    virtual void SetShowFullText(bool bShow) = 0;

    virtual void SetShowText(bool bShow) = 0;

    virtual void SetShowDateCheck(bool bCheck) = 0;

	virtual void SetShowChartBoundary( bool bShow) = 0;

	virtual void SetShowAccuracy(bool bShow) = 0;

    virtual void SetShowGrid(bool bFlag)=0;

    virtual void SetShowShallowPattern(bool bShow)=0;

	virtual void SetShowOverScale(bool bShow) = 0;

	virtual void SetShowUnknownObjects(bool bShow) = 0;

	virtual void SetMonoView(int state) = 0;

	virtual void SetS57TextScaleFactor(float factor) = 0;

    virtual bool GetShowGrid()=0;

    virtual bool GetViewGroupState(int grp)=0;

	virtual int GetViewGroupLayerState(int lyr)=0;

	virtual int* GetViewGroupLayersList(int & nLyrs) = 0;

	virtual int* GetViewGroupsList(int & nGrps) = 0;

	virtual void FreeViewGroupsList(int * pGrp) = 0;

    virtual int* GetTextGroupsList(int& nGrp)=0;

    virtual void FreeTextGroupsList(int * pGrp) = 0;

	virtual bool GetTextGroupLayerState(int lyr)=0;

	virtual bool GetTextGroupState(int grp) = 0;

	virtual int* GetTextGroupLayersList(int & nLyrs) = 0;

	virtual void FreeTextGroupLayersList(int * pLyr) = 0;

	virtual bool GetShowFullText() = 0;

	virtual bool GetShowText() = 0;

    virtual bool GetShowDateCheck() = 0;

    virtual EnclDisplayCategory GetDisplayCategory()=0;

    virtual EnclBoundarySymbolType GetSymbolizedAreaBoundaries()=0;
	
	virtual EnclLineSymbolStyle GetLineSymbolStyle()=0;

    virtual void SetShowOtherVesselSymbolOnly(bool bFlag)=0;

    virtual void SetShowContourLabels(bool bFlag)=0;

    virtual void SetShowOtherContour(bool bFlag)=0;

    virtual void SetShowScaleBoundaries(bool bFlag)=0;

	virtual void SetS57ObjCustomSCAMIN(const char * classAcronym, int iSCAMIN) = 0;

	virtual void SetShowFullLightSectorLines(bool bFull) = 0;

    virtual void SetViewDate(unsigned long date) = 0;

    virtual void SetViewDateRange(unsigned long dateStart, unsigned long dateEnd) = 0;

    virtual void SetViewDateAuto(bool autoflag) = 0;

	virtual void SetHighlightDateDependentObjs(bool bHighlight) = 0;

	virtual void SetHighlightAdditionalChartInfo(bool bHighlight) = 0;

	virtual void SetHighlightAdditionalChartInfoDoc(bool bHighlight) = 0;

	virtual void SetShowIsolatedDangerObjects(bool bShow) = 0;

	virtual void SetShowShallowWarterIsolatedDangerObjects(bool bShow) = 0;

    virtual bool GetShowContourLabels()=0;

    virtual bool GetShowOtherContour()=0;

    virtual bool GetShowScaleBoundaries()=0;

    virtual bool GetShowTwoDepthShades()=0;

    virtual bool GetSCAMINEnabledStatus()=0;

    virtual double GetSafetyDepth()=0;

	virtual double GetSafetyContour()=0;

    virtual double GetShallowContour()=0;

	virtual double GetDeepContour()=0;

	virtual bool GetShowSoundings()=0;

	virtual bool GetDisplayNationalLanguage()=0;

    virtual bool GetDisplayChineseLanguage()=0;

	virtual bool GetShowOtherVesselSymbolOnly()=0;

    virtual EnclPointSymbolType GetPointSymbolStyle()=0;

    virtual EnclDisplayStyle GetDisplayStyle()=0;

	virtual int GetRouteDistanceTag()=0;

	virtual int GetPasttrackTimemarkInterval()=0;

	virtual const char* GetColorScheme()=0;

	virtual void GetShipDimension(double& length, double& beam, int& draught, int& airdraught)=0;

	virtual bool GetShowPasttrack()=0;

	virtual int GetPredictionVectorLength()=0;

	virtual Stabilization GetStabilizationType()=0;

	virtual bool GetShowOneMinTimeMark()=0;

	virtual bool GetShowOutlinedShipSymbol()=0;

	virtual void GetS57ObjCustomSCAMIN(const char * classAcronym, int &iSCAMIN) = 0;

	virtual bool GetShowFullLightSectorLines() = 0;	

    virtual bool GetShowShallowPattern()=0;

    virtual void GetViewDateRange(unsigned long * dateStart, unsigned long * dateEnd) = 0;

    virtual bool GetViewDateAuto() = 0;

	virtual bool GetHighlightDateDependentObjs() = 0;
	
	virtual bool GetHighlightAdditionalChartInfo() = 0;

	virtual bool GetHighlightAdditionalChartInfoDoc() = 0;

	virtual bool GetShowOverScale() = 0;

	virtual bool GetShowChartBoundary() = 0;

	virtual bool GetShowAccuracy() = 0;

	virtual bool GetShowUnknownObjects() = 0;

	virtual int GetMonoView() = 0;

	virtual bool GetShowIsolatedDangerObjects() = 0;

	virtual bool GetShowShallowWarterIsolatedDangerObjects() = 0;

	virtual float GetS57TextScaleFactor() = 0;
protected:
    RcMarinerSettings(){}
};

class CORE_EXPORT RcManualUpdates
{
public:
	virtual ~RcManualUpdates(){}

	static RcManualUpdates* GetInstance();

	virtual EnclObjectPtr ManualUpdAddObject(const ManUpdObjData & info,
		const ManUpdObjAttrs & attrs, const EnclObjectGeoData & data, 
		ManUpdState mus = MUS_ADD_NEW)=0;

	virtual EnclObjectPtr ManualUpdAddObject(const EnclManualUpdateObject & obj, 
		ManUpdState mus = MUS_ADD_NEW, EnclObjectPtr prevObjId=0)=0;

	virtual EnclObjectPtr ManualUpdRemoveEncObject(EnclObjectPtr objID) = 0;
	virtual EnclObjectPtr* ManualUpdMoveEncObject(EnclObjectPtr objID,
		const EnclObjectGeoData & data, int &nObjs) = 0;

    virtual bool ManualUpdRemoveObject(EnclObjectPtr objID)=0;

    virtual bool ManualUpdRestoreObject(EnclObjectPtr objID)=0;

    virtual bool ManualUpdDestroyObject(EnclObjectPtr objID)=0;

    virtual void ManualUpdReload()=0;

	virtual unsigned char * GetManualUpdateSymbolImage(
		const EnclS57ClassInfo & info, int imgWidth, int imgHeight) = 0;

	virtual EnclObjectPtr* GetManualUpdateObjectsList(int& nObjs)=0;

	virtual bool GetManualUpdObjectAttr(EnclObjectPtr objID, EnclManualUpdateObject& attr)=0;

	virtual void FreeManualUpdObjectAttr(EnclManualUpdateObject & attr)=0;

	virtual void ModifyManualUpdateObjectAttr(EnclObjectPtr objID, const ManUpdObjAttrs& attr)=0;

	virtual EnclObjectPtr ModifyManualUpdateObjectAttr(EnclObjectPtr objID, const EnclManualUpdateObject& obj)=0;
protected:
	RcManualUpdates(){}
};

class CORE_EXPORT RcUserChart
{
public:
	virtual ~RcUserChart(){}

	static RcUserChart* GetInstance();

	virtual EnclObjectPtr UserChartAddObject(const UsrChrtObjData & info,
		const UsrChrtObjAttrs & attrs, const EnclObjectGeoData & data,
		UsrChrtState ucs = UCS_ADD_NEW)=0;

	virtual EnclObjectPtr UserChartAddObject(const EnclUserChartObject & obj,
		UsrChrtState ucs = UCS_ADD_NEW, EnclObjectPtr prevObjId=0)=0;

	virtual bool UserChartRemoveObject(EnclObjectPtr objID)=0;

	virtual bool UserChartRestoreObject(EnclObjectPtr objID)=0;

	virtual bool UserChartDestroyObject(EnclObjectPtr objID)=0;

	virtual void UserChartReload()=0;

	virtual unsigned char * GetUserChartSymbolImage(
		const EnclS57ClassInfo & info, int imgWidth, int imgHeight) = 0;

	virtual EnclObjectPtr* GetUserChartObjectsList(int& nObjs)=0;

	virtual bool GetUserChartObjectAttr(EnclObjectPtr objID, EnclUserChartObject& attr)=0;

	virtual void FreeUserChartObjectAttr(EnclUserChartObject & attr)=0;

	virtual void ModifyUserChartObjectAttr(EnclObjectPtr objID, const UsrChrtObjAttrs& attr)=0;

	virtual EnclObjectPtr ModifyUserChartObjectAttr(EnclObjectPtr objID, const EnclUserChartObject& obj)=0;
protected:
	RcUserChart(){}
};

class CORE_EXPORT RcUtility
{
public:
	static void DMSEncode(double angle, DmsFlag flag, DmsComponent comp, int precision, char * output);
	static double DMSDecode(const char * string);
};

class CORE_EXPORT RcTime
{
public:
	// Set SDK reference time in seconds since 1970.1.1
	static void SetTime(time_t secs);
	// Get current SDK reference UTC time in seconds since 1970.1.1
	static time_t GetTime(time_t * t);
};

class RcEagleEye
{
public:
	static RcEagleEye* GetInstance();
	virtual unsigned char* GetEagleEyeImage(int width, int height) = 0;
	virtual void GeoToPix(double lon, double lat, int& x, int& y) = 0;
	virtual void PixToGeo(int x, int y, double& lon, double& lat) = 0;
};

#ifdef _WINNT_SOURCE
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#endif


#endif //_RC_ECDIS_SDK_H_
