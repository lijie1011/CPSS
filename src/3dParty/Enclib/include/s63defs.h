#ifndef __S63_DEF_H__
#define __S63_DEF_H__

#include <vector>
#include <time.h>

#pragma pack(push, 8)

//!S63 海图许可信息
typedef struct
{
	char permitString[65]; /**<许可内容*//* cell permit string */
	char chartno[8];      /**<图名*//* cell name w/o extension */
	int  expYear;          /**<过期日期，年*//* expiry date, year  */
	int  expMonth;         /**<过期日期，月*//* expiry date, month */
	int  expDay;           /**<过期日期，日*//* expiry date, day   */
	int  service;          /**<服务级别*//* service level indictator */
	int  edition;          /**<版本*//* edition number */
	char dataServer[3];    /**<数据服务商ID*//* data server ID */
} EnclS63CellPermitInfo;

typedef struct
{
    EnclS63CellPermitInfo *permitinfo;
    int ninfos;
}EnclS63CellPermitList;

typedef struct _ErrCellPermitCallbackData
{
    EnclS63CellPermitInfo * cellPermit;
    bool bFormatError;
}EnclErrCellPermitCallbackData;

//additional erro codes
const unsigned int ERROR_SSEN1 = -1; /* parameters invalid" */
const unsigned int ERROR_SSE00 = 0; /* no error */
//
/* error codes defined by S-63 */
const unsigned int ERROR_SSE01 = 1; /* "Self Signed Key is invalid" */
const unsigned int ERROR_SSE02 = 2; /* "Format of Self Signed Key file is incorrect" */
const unsigned int ERROR_SSE03 = 3; /* "SA Signed Data Server Certificate is invalid" */
const unsigned int ERROR_SSE04 = 4; /* "Format of SA Signed Data Server Certificate is incorrect" */
const unsigned int ERROR_SSE05 = 5; /* "SA Digital Certificate (X509) file is not available" */
const unsigned int ERROR_SSE06 = 6; /* "SA Signed Data Server Certificate is invalid" */
const unsigned int ERROR_SSE07 = 7; /* "SA Signed Data Server Certificate file is not available" */
const unsigned int ERROR_SSE08 = 8; /* "SA Digital Certificate (X509) file has incorrect format" */
const unsigned int ERROR_SSE09 = 9; /* "ENC Signature is invalid" */
const unsigned int ERROR_SSE10 = 10; /* "Permits not available for this Data Server" */
const unsigned int ERROR_SSE11 = 11; /* "Cell Permit not found or invalid cell name" */
const unsigned int ERROR_SSE12 = 12; /* "Cell Permit format is incorrect" */
const unsigned int ERROR_SSE13 = 13; /* "Cell Permit is invalid (checksum is incorrect)" */
const unsigned int ERROR_SSE14 = 14; /* "Incorrect system date" */
const unsigned int ERROR_SSE15 = 15; /* "Subscription service has expired" */
const unsigned int ERROR_SSE16 = 16; /* "ENC CRC value is incorrect" */
const unsigned int ERROR_SSE17 = 17; /* "Userpermit is invalid (checksum is incorrect)" */
const unsigned int ERROR_SSE18 = 18; /* "Hardware ID has incorrect format" */
const unsigned int ERROR_SSE19 = 19; /* "Permits are not valid for this system" */
const unsigned int ERROR_SSE20 = 20; /* "Subscription service will expire in less than 30 days" */
const unsigned int ERROR_SSE21 = 21; /* "Decryption failed no valid cell permit found.*/
const unsigned int ERROR_SSE22 = 22; /* "SA Digital Certificate (X509) has expired" */
const unsigned int ERROR_SSE23 = 23; /* "Non sequential update" */
const unsigned int ERROR_SSE24 = 24; /* "ENC signature format incorrect" */
const unsigned int ERROR_SSE25 = 25; /* "The permit has expired" */
const unsigned int ERROR_SSE26 = 26; /* "This ENC is not authenticated by the IHO acting as the Scheme Administrator" */
const unsigned int ERROR_SSE27 = 27; /* "ENC is not up to date" */
const unsigned int ERROR_SSE100 = 100; /* "ENC is cancelled" */
const unsigned int ERROR_SSE101 = 101; /* "Base is not compatible" */
const unsigned int ERROR_SSE102 = 102; /* "Can't find SERIAL.ENC" */
const unsigned int ERROR_SSE103 = 103; /* "Can't find PRODUCTS.TXT" */
const unsigned int ERROR_SSE104 = 104; /* "Can't find CATALOG.031" */
const unsigned int ERROR_SSE105 = 105; /* "Not newer" */
const unsigned int ERROR_SSE106 = 106; /* "Edition is not compatible" */

typedef enum _EnclS63CrtAttrType
{
	ENCL_S63CRTATTR_ISSUER,
	ENCL_S63CRTATTR_SUBJECT,
	ENCL_S63CRTATTR_PUBLIC_KEY_ALGO,
	ENCL_S63CRTATTR_SIG_ALOGRITHM,
	ENCL_S63CRTATTR_TAKEEFFECT,
	ENCL_S63CRTATTR_EXPIRYDATE,
}EnclS63CrtAttrType;

/* enumerations and types for handling PRODUCTS.TXT */
typedef enum
{
	EnclS63ProductsCatalogType_Unknown = 0,
	EnclS63ProductsCatalogType_Full    = 1,
	EnclS63ProductsCatalogType_Partial = 2,
	EnclS63ProductsCatalogType_Merged  = 3
} EnclS63ProductsCatalogType;

typedef enum
{
	EnclS63ProductsCatalogInfoType_Unknown = 0,
	EnclS63ProductsCatalogInfoType_ENC     = 1,
	EnclS63ProductsCatalogInfoType_ECS     = 2,
	EnclS63ProductsCatalogInfoType_Other   = 3
} EnclS63ProductsCatalogInfoType;

typedef struct
{
	EnclS63ProductsCatalogInfoType type; /* product type (ENC or ECS) */
	char cellName[32];                 /* product (cell) name */
	int  cellIssueDate;                /* base cell issue date */
	int  editionNo;                    /* base cell edition */
	int  updateIssueDate;              /* issue date latest update */
	int  updateNo;                     /* latest update number */
	int  fileSize;                     /* total size for all files of the product */
	double southLat, westLon, northLat, eastLon; /* cell limits */
	double coverage[20];         /* cell coverage coordinates */
	int  nCoverage;                    /* number of coverage coordinate pairs */
	int  compression;                  /* compression flag */
	int  encryption;                   /* encryption flag */
	int  reIssueUpdateNo;              /* re-issued base cell update number */
	int  prevEditionUpdateNo;          /* latest update number for previous edition */
	char location[16];                 /* base cell location */
	char cellReplacements[160];        /* cancelled cell replacements */
	char dataServer[3];    /**<数据服务商ID*//* data server ID */
} EnclS63ProductsCatalogInfo;

typedef struct EnclTimeAndDateStruct
{
    int year;        /* 1970..2037 */
    int month;       /* 1..12 */
    int day;         /* 1..31 */
    int hour;        /* 0..23 */
    int minute;      /* 0..59 */
    int second;      /* 0..59 */
    int millisecond; /* 0..999 */
} EnclTimeAndDate;

typedef struct
{
    EnclTimeAndDate date;
    int version;
    EnclS63ProductsCatalogType type;
    EnclS63ProductsCatalogInfo *prodinfo;
    int ninfos;
}EnclS63ProductsCatalog;

/* enumerations and types for handling SERIAL.ENC */
typedef enum
{
	EnclS63ExchangeSetType_Unknown = 0,
	EnclS63ExchangeSetType_Base    = 1,
	EnclS63ExchangeSetType_Update  = 2
} EnclS63ExchangeSetType;

typedef struct
{
	char dataServer[4];        /* data server ID */
	char issueWeek[12];        /* week of issue */
	int  publicationDate;      /* date of publication */
	EnclS63ExchangeSetType type; /* CD type */
	char version[8];           /* format version */
	char exSetNo[8];           /* exchange set number */
} EnclS63ExchangeSetInfo;

typedef struct
{
	char base_media_number[4];
	char dataserver[3];
	char week_of_issue[8];
	char user_info[101];
	char base_issue_date[9];
} EnclS63StatusInfo;

typedef struct
{
	char dataserver[3];
	char week_of_issue[11];
	char media_type[11];
	char issue_date[9];
	EnclS63StatusInfo *stinfos;
	int ninfos;
}EnclS63StatusLst;

typedef struct
{
	char location[8];
	char issue_date[9];
	char long_name[101];
	char regional_info[101];
	char reserved[101];
	char comments[101];
} EnclS63MediaInfo;

typedef struct
{
	char dataserver[3];
	char week_of_issue[11];
	char media_type[11];
	char issue_date[9];
	char media_lable_id[7];
	char media_id[4];
	char media_name[101];
	char regional_info[101];
	EnclS63MediaInfo *mdinfos;
	int ninfos;
}EnclS63MediaLst;

#pragma pack(pop)

#endif
