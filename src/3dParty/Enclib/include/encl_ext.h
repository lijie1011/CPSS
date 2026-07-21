#ifndef ECDISINTERFACE_H
#define ECDISINTERFACE_H
// This is not a pure c api header since there are some c++ classes include in this header.
// e.g. PolyData EnclObjectPtr etc. We might turn this header into pure c header in the 
// future so DO NOT ADD MORE C++ feature to this header.
// To compress the c and C++ linkage incompatible warning the extern "C" indication will
// be commented out for now.

/*! \file encl.h
    \brief Enclib 库头文件.
*/ 

#include "commonDef.h"
#include "S57ManualUpdateDef.h"
#include "S57UserChartDef.h"
#include "Route_WayPointDef.h"
#include "nmea.h"
#include "ownershipDef.h"
#include "importcallbackdefs.h"
#include "alert.h"
#include "AISObjectDefine.h"
#include "TTObjectDefine.h"
#include "global.h"

#pragma execution_character_set("utf-8")

typedef  void*  EnclChartHeader;
typedef unsigned char * EnclImgDataPtr;

/** @defgroup sdkbase 基本配置 
* 库基本参数配置及初始化
* @{
*/

/*! \brief 初始化 Enclib 库
* \param encLibPath Enclib库数据文件存放路径，若设置为0或其他不存在的路径，则使用系统环境变量“Enclib”
* 所指路径，此时要求 Enclib 环境变量必须设置，否则会导致初始化失败。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	EnclInitialize("/usr/local/Enclib");
*	EnclInitialize(0);
* @endcode
*/
ENCL_API bool EnclInitialize(const char *encLibPath);
//obsolete
ENCL_API bool EnclSENCInit(const char *encLibPath);

/*! \brief 释放 Enclib 库
* \return 成功返回true，否则返回false。
*/
ENCL_API bool EnclDeinitialize();
//obsolete
ENCL_API bool EnclSENCFree();

/*! \brief 设置海图安装路径
* \param dataPath 海图安装路径，若设置为0，则使用默认路径。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	bool done;
*	if(m_params.size() == 0){
*		done = EnclSENCSetDataDir(0);
*	}else{
*		std::string spath = m_params[0];
*		done = EnclSENCSetDataDir(spath.c_str());
*	}
* @endcode
*/
ENCL_API bool EnclSENCSetDataDir(const char *dataPath);

/*! \brief 获取海图安装路径.
* \param dataPath 用于存放安装路径的内存指针，需要提前分配好空间。
* \param maxBufSize dataPath的字节数.
* \return 无.
*
* @par 示例:
* @code
*	char pszSENCDir[512];
*	memset(pszSENCDir, 0, 512);
*	EnclSENCGetDataDir(pszSENCDir, 512);
* @endcode
*/
ENCL_API void EnclSENCGetDataDir(char* dataPath, int maxBufSize);

/*! \brief 获取本机HWID.
* 获取本机HWID，用于安装加密海图，生成UserPermit等
* \param hwId 用于存放HWID的内存指针, 需要提前分配好至少 6 Bytes
* \return 无.
*
* @par 示例:
* @code
*	char hwid[6];
*	memset(hwid, 0, 6);
*	EnclGetHWID(hwid);
* @endcode
*/
ENCL_API void EnclGetHWID(char* hwId);
	
/*! \brief 获取海图库版本字符串.
* \return 海图库版本字符串.
*/
ENCL_API const char* EnclSoftwareGetVersion();

/*! \brief 设置系统时间.
* \param secs 需要设置的系统时间戳（从1970.1.1开始的秒数)
* \return 无.
*
* @par 示例:
* @code
*	struct tm nowdate;
*	time_t nowsecs;
*	nowdate.tm_year = 2022 - 1900;
*	nowdate.tm_mon = 2 - 1;
*	nowdate.tm_mday = 22;
*	nowdate.tm_hour = 8;
*	nowdate.tm_min = 0;
*	nowdate.tm_sec = 0;
*	nowsecs = mktime(&nowdate);
*	EnclTimeSetTime(nowsecs);
* @endcode
*/
ENCL_API void EnclTimeSetTime(time_t secs);

/*! \brief 获取系统时间.
* \param secs 用于存放系统当前时间戳，可以为null
* \return 系统当前时间戳.
*
* @par 示例:
* @code
*	long nowsecs = EnclTimeGetTime(0);
* @endcode
*/
ENCL_API time_t EnclTimeGetTime(time_t * secs);
/** @} */

/** @defgroup chartmng 海图管理 
* 海图安装，卸载，更新，查询等
* @{
*/
/*! \brief 卸载海图
* \param chartName 海图名字（8位字符）。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	char chartno[1024];
*	memset(chartno, 0, 1024);
*	strcpy(chartno, m_params[0].c_str());
*	bool done = EnclSENCDelete(chartno);
* @endcode
*/
ENCL_API bool EnclSENCDelete(char * chartName);

/*! \brief 安装MENC海图（单个文件方式）
* \param encFile 待安装海图文件路径名称。
* \param crc CRC校验码，如果为null，则不进行校验。
* \param errNum 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
* \return 成功返回true，否则返回false。
*/
ENCL_API bool EnclMENCEncImportFile(const char *encFile, const unsigned int *crc, int *errNum);
//obsolete
ENCL_API bool EnclSENCMENCImportFile(const char * encFile);

/*! \brief 安装MENC海图（目录方式）
* \param encPath 目录路径。
* \param callback 回调函数，用于输出安装过程中出现的错误码。
* \param errNum 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
* \return 成功返回true，否则返回false。
*/
ENCL_API bool EnclMENCEncImportTree(const char *encPath, EnclSENCImportErrCallBack callback, int *errNum);

//obsolete
ENCL_API int EnclSENCMENCImportTree(const char * encPath);

/*! \brief 安装S57海图（单个文件方式）
* \param encFile 待安装海图文件路径名称。
* \param crc CRC校验码，如果为null，则不进行校验。
* \param errNum 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int errnum;
*	unsigned int *pcrc;
*	unsigned int crc;
*	if(!m_cellcrcs.at(i).isEmpty()){
*		crc = m_cellcrcs.at(i).toLongLong();
*		pcrc = &crc;
*	}else{
*		pcrc = NULL;
*	}
*	bool ret = EnclS57EncImportFile(cellfpath.toStdString().c_str(), (const unsigned int *)pcrc, &errnum);
* @endcode
*/
ENCL_API bool EnclS57EncImportFile(const char *encFile, const unsigned int *crc, int *errNum);
//obsolete
ENCL_API bool EnclSENCS57ImportFile(const char * encFile);

/*! \brief 安装S57海图（目录方式）
* \param encPath 目录路径。
* \param callback 回调函数，用于输出安装过程中出现的错误码。
* \param errNum 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	bool S57InstallByTree(const char * rootpath)
*	{
*		int errnum;
*		bool bret = EnclS57EncImportTree(rootpath, g_errcallback, &errnum);
*		return bret;
*	}
* @endcode
*/
ENCL_API bool EnclS57EncImportTree(const char *encPath, EnclSENCImportErrCallBack callback, int *errNum);

//obsolete
ENCL_API int EnclSENCS57ImportTree(const char * encPath);

/*! \brief 安装S63海图（目录方式）
* \param hwId HWID。
* \param encPath 目录路径。
* \param permitFile 许可证文件路径。
* \param crtFile 安全证书路径。
* \param callback 回调函数，用于输出安装过程中出现的错误码。
* \param errNum 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	bool S63InstallByTree(const char* rootPath, const char* permitFilePath, const unsigned char* hw_id, const char* ihoCrtFilePath)
*	{
*		int errnum;
*		bool bret = EnclS63EncImportTree(hw_id, rootPath, permitFilePath, ihoCrtFilePath, g_errcallback, &errnum);
*		return bret;
*	}
* @endcode
*/
ENCL_API bool EnclS63EncImportTree(const unsigned char *hwId, const char *encPath, const char *permitFile, const char *crtFile, EnclSENCImportErrCallBack callback, int *errNum);

/*! \brief 安装S63海图（单个文件方式）
* \param encFile 待安装海图文件路径名称。
* \param permitInfo 许可证信息。
* \param hwId HWID。
* \param crc CRC校验码，如果为null，则不进行校验。
* \param percentFunc 回调函数，用于输出安装进度。
* \return 返回错误码，0 表示成功，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	errnum = EnclS63EncReadCell(encfile.toStdString().c_str(), &(pmtinfos[pmtidx]), (const unsigned char *)m_acthwid.toStdString().c_str(), pcrc, NULL);
* @endcode
*/
ENCL_API int EnclS63EncReadCell(const char *encFile, const EnclS63CellPermitInfo *permitInfo, const unsigned char *hwId, const unsigned int *crc, bool (*percentFunc)(int, const char *));

/*! \brief 根据待安装海图文件的路径名称获取其签名文件的路径名称，这两个文件总是在同一目录下。
* \param encFile 待安装海图文件路径名称。
* \param sigFile 返回的签名文件的路径名称，需要提前分配好足够空间，建议1024 bytes
* \return 无
*
* @par 示例:
* @code
*	char sigf[1024]={0};
*	bool sigpass = true;
*	EnclS63EncGetSignatureName(cellfpath.toStdString().c_str(), sigf);
*	QString sigfile = QString(sigf);
*	QString encfile = cellfpath;
*	errnum = EnclS63CrtCheckSignature(m_installedcrtfile.toStdString().c_str(), sigfile.toStdString().c_str(), encfile.toStdString().c_str());
*	if(errnum != ERROR_SSE00)
*	{
*	    sigpass = false;
*	}
* @endcode
*/
ENCL_API void EnclS63EncGetSignatureName(const char *encFile, char *sigFile);

/*! \brief 创建EnclS63ProductsCatalog数据结构对象，用于处理 PRODUCTS.TXT
* \return 返回对象指针
*
* @par 示例:
* @code
*	EnclS63ProductsCatalog *prdcata = EnclS63EncCreateProductCatalog();
*	EnclS63EncReadProductCatalog(m_parent->GetInstalledPrdPath().toStdString().c_str(), NULL, prdcata);
*	
*	for(int i = 0; i < selcells.size(); i++){
*	    QString cellname = QString(cellitems[0]);
*	    QString cellds = QString(cellitems[1]);
*		EnclS63EncDelProductCatalogInfo(prdcata, cellname.toStdString().c_str(), cellds.toStdString().c_str());
*	}
*	
*	EnclS63EncWriteProductCatalog(m_parent->GetInstalledPrdPath().toStdString().c_str(), prdcata);
*	
*	EnclTimeAndDate date;
*	int version;
*	EnclS63ProductsCatalogType type;
*	EnclS63ProductsCatalogInfo *prodcatinfo;
*	int ninfos;	
*	EnclS63EncGetProductCatalogInfo(prdcata, &date, &version, &type, &prodcatinfo, &ninfos);
*	
*	EnclS63EncFreeProductCatalog(prdcata);
* @endcode
*/
ENCL_API EnclS63ProductsCatalog *EnclS63EncCreateProductCatalog();

/*! \brief 释放EnclS63ProductsCatalog数据结构对象
* \param prodCat 待释放对象指针。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclS63EncCreateProductCatalog() 示例
* @endcode
*/
ENCL_API void EnclS63EncFreeProductCatalog(EnclS63ProductsCatalog *prodCat);

/*! \brief 读取PRODUCTS.TXT信息到创建好的数据结构对象中
* \param prodFile PRODUCTS.TXT文件路径。
* \param dataServer DataServer名称（两个字符）。
* \param prodCat 已创建好的数据结构对象指针。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63EncCreateProductCatalog() 示例
* @endcode
*/
ENCL_API bool EnclS63EncReadProductCatalog(const char *prodFile, const char *dataServer, EnclS63ProductsCatalog *prodCat);

/*! \brief 保存EnclS63ProductsCatalog数据结构对象中的信息为文件
* \param prodFile 保存为文件路径，文件名字一般为PRODUCTS.TXT
* \param prodCat EnclS63ProductsCatalog数据结构对象指针。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63EncCreateProductCatalog() 示例
* @endcode
*/
ENCL_API bool EnclS63EncWriteProductCatalog(const char *prodFile, const EnclS63ProductsCatalog *prodCat);

/*! \brief 读取EnclS63ProductsCatalog数据结构对象中的信息
* \param prodCat EnclS63ProductsCatalog数据结构对象指针。
* \param date 读取出来的日期
* \param version 读取出来的版本信息
* \param content 读取出来的状态类型信息
* \param prodInfo 指向读取出来的记录，实际上直接指向EnclS63ProductsCatalog数据结构对象中的记录，故不需要释放。
* \param nInfos 读取出来的记录个数
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63EncCreateProductCatalog() 示例
* @endcode
*/
ENCL_API bool EnclS63EncGetProductCatalogInfo(const EnclS63ProductsCatalog *prodCat, EnclTimeAndDate *date, int *version, EnclS63ProductsCatalogType *content, EnclS63ProductsCatalogInfo **prodInfo, int *nInfos);

/*! \brief 依据海图名称以及DataServer名称删除EnclS63ProductsCatalog数据结构对象中相应信息
* \param prodCat EnclS63ProductsCatalog数据结构对象指针。
* \param cellName 海图名称（8个字符）
* \param dataServer DataServer名称（2个字符）
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63EncCreateProductCatalog() 示例
* @endcode
*/
ENCL_API bool EnclS63EncDelProductCatalogInfo(EnclS63ProductsCatalog *prodCat, const char *cellName, const char *dataServer);

/*! \brief 读取SERIAL.ENC信息到EnclS63ExchangeSetInfo数据结构对象中
* \param serialFile SERIAL.ENC文件路径。
* \param esInfo EnclS63ExchangeSetInfo数据结构对象指针。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	EnclS63ExchangeSetInfo esinfo;
*	EnclS63EncReadExchangeSetInfo(m_essrlfile.toStdString().c_str(), &esinfo);
* @endcode
*/
ENCL_API bool EnclS63EncReadExchangeSetInfo(const char *serialFile, EnclS63ExchangeSetInfo *esInfo);

/*! \brief 读取STATUS.LST中的信息并返回
* \param statusFile STATUS.LST文件路径。
* \return 返回EnclS63StatusLst数据结构指针
*
* @par 示例:
* @code
*	EnclS63StatusLst *stlst = EnclS63EncReadStatusLst(m_esstsfile.toStdString().c_str());
*	if(stlst){
*		for(int i = 0; i < stlst->ninfos; i++){
*			QString basename = stlst->stinfos[i].base_media_number;
*			QString baseinfo = stlst->stinfos[i].user_info;
*			m_esbasemsg[basename] = baseinfo;
*		}
*		EnclS63EncFreeStatusLst(stlst);
*	}
* @endcode
*/
ENCL_API EnclS63StatusLst * EnclS63EncReadStatusLst(const char *statusFile);

/*! \brief 释放EnclS63StatusLst数据结构对象指针
* \param stLst 待释放对象指针。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclS63EncReadStatusLst() 示例
* @endcode
*/
ENCL_API void EnclS63EncFreeStatusLst(EnclS63StatusLst *stLst);

/*! \brief 读取MEDIA.TXT中的信息并返回
* \param mediaFile MEDIA.TXT文件路径。
* \return 返回EnclS63MediaLst数据结构指针
*
* @par 示例:
* @code
*	EnclS63MediaLst *mdlst = EnclS63EncReadMediaLst(m_esmdifile.toStdString().c_str());
*	if(mdlst){
*		m_esds = mdlst->dataserver;
*		for(int i = 0; i < mdlst->ninfos; i++){
*			QString basename = mdlst->mdinfos[i].location;
*			QString baseinfo = mdlst->mdinfos[i].long_name;
*			m_esbasemsg[basename] = baseinfo;
*		}
*		EnclS63EncFreeMediaLst(mdlst);
*	}
* @endcode
*/
ENCL_API EnclS63MediaLst * EnclS63EncReadMediaLst(const char *mediaFile);

/*! \brief 释放EnclS63MediaLst数据结构对象指针
* \param mediaLst 待释放对象指针。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclS63EncReadMediaLst() 示例
* @endcode
*/
ENCL_API void EnclS63EncFreeMediaLst(EnclS63MediaLst *mediaLst);


//obsolete
ENCL_API bool EnclS63EncCheckBaseCompatible(const char *rootPath, EnclS63CellPermitList * permitList, char *msg);
ENCL_API bool EnclS63EncCheckNewer(const char *rootPath, const char *cellName);
ENCL_API bool EnclS63EncResetProdCat();
ENCL_API int EnclSENCS63ImportTree(const char* rootPath, const char* permitFile, const unsigned char* hwId, const char* crtFile);
ENCL_API void EnclSENCSetImportErrorCallback(EnclSENCImportErrCallBack iec);

/*! \brief 读取CATALOG.031
* \param catFile CATALOG.031 文件路径。
* \param errlog 目前没用，给null即可。
* \param sortFlag true表示排序，false不进行排序。
* \return 返回EnclDENCExchangeSet数据结构指针
*
* @par 示例:
* @code
*	EnclDENCExchangeSet *exchset = EnclDENCExchangeSetReadCatalog(m_escatfile.toStdString().c_str(), NULL, true);
*	if(exchset != NULL)
*	{
*	    int volnums;
*	    int nentries = EnclDENCExchangeSetGetNumberOfEntries(exchset, &volnums);
*	    const char *rootpath = EnclDENCExchangeSetGetRootPath(exchset);
*	    for(int i = 0; i < nentries; i++)
*	    {
*	        const EnclDENCExchangeSetEntry *entry = EnclDENCExchangeSetGetEntry(exchset, i);
*	        if (entry->fileType == ExSetFileType_DataSet)
*	        {
*	            QString fpath = QString(rootpath) + QString(entry->subDir) + QString(entry->fileName);
*	            QString cellfname = entry->fileName;
*	            QString cellfpath = fpath;
*	
*	            if(!QFile::exists(cellfpath)){
*	                continue;
*	            }
*	        }
*	    }
*	}
*	EnclDENCExchangeSetFreeCatalog(exchset);
* @endcode
*/
ENCL_API EnclDENCExchangeSet *EnclDENCExchangeSetReadCatalog( const char *catFile, FILE *errlog, bool sortFlag );

/*! \brief 释放EnclDENCExchangeSet对象
* \param exchSet 待释放对象指针，该对象指针由EnclDENCExchangeSetReadCatalog()生成
* \return 无
*
* @par 示例:
* @code
*	参考 EnclDENCExchangeSetReadCatalog() 示例
* @endcode
*/
ENCL_API void EnclDENCExchangeSetFreeCatalog( EnclDENCExchangeSet *exchSet );

/*! \brief 获取数据集类型
* \param exchSet EnclDENCExchangeSet对象指针
* \return 	ExSetType_Unknown		= 0, normal, search all enc in current dir
* \return	ExSetType_S57			= 1, plain S-57, suchas: <Power>(include ENC_ROOT)
* \return	ExSetType_S63			= 2, standard S-63, such as: <V01X01>(include ENC_ROOT)
* \return	ExSetType_S63Large		= 3, S-63 large media, such as: <M01X01>(include MEDIA.TXT)
* \return	ExSetType_S63LargeSub	= 4, S-63 large media sub, suchas: <M01X01/B1>
*/
ENCL_API EnclDENCExchangeSetType EnclDENCExchangeSetGetType( const EnclDENCExchangeSet *exchSet );

/*! \brief 获取数据集根目录路径
* \param exchSet EnclDENCExchangeSet 对象指针，该对象指针由EnclDENCExchangeSetReadCatalog()生成
* \return 返回数据集根目录路径
*
* @par 示例:
* @code
*	参考 EnclDENCExchangeSetReadCatalog() 示例
* @endcode
*/
ENCL_API const char *EnclDENCExchangeSetGetRootPath( const EnclDENCExchangeSet *exchSet );

//nouse
ENCL_API const char *EnclDENCExchangeSetGetComment( const EnclDENCExchangeSet *exchSet );

/*! \brief 获取数据集中海图安装文件的个数
* \param exchSet EnclDENCExchangeSet 对象指针，该对象指针由EnclDENCExchangeSetReadCatalog()生成
* \param numOfVolumes 存放返回的卷标个数，这个参数目前用不到。
* \return 返回数据集中海图安装文件的个数
*
* @par 示例:
* @code
*	参考 EnclDENCExchangeSetReadCatalog() 示例
* @endcode
*/
ENCL_API int EnclDENCExchangeSetGetNumberOfEntries( const EnclDENCExchangeSet *exchSet, int *numOfVolumes );

/*! \brief 根据索引号获取数据集中海图安装文件信息
* \param exchSet EnclDENCExchangeSet 对象指针，该对象指针由EnclDENCExchangeSetReadCatalog()生成
* \param index 索引号。
* \return 根据索引号返回数据集中海图安装文件信息
*
* @par 示例:
* @code
*	参考 EnclDENCExchangeSetReadCatalog() 示例
* @endcode
*/
ENCL_API const EnclDENCExchangeSetEntry *EnclDENCExchangeSetGetEntry( const EnclDENCExchangeSet *exchSet, int index );

/*! \brief 根据所给目录路径自动判断获取数据集类型
* \param importPath 数据集根目录路径
* \return 	ExSetType_Unknown		= 0, normal, search all enc in current dir
* \return	ExSetType_S57			= 1, plain S-57, suchas: <Power>(include ENC_ROOT)
* \return	ExSetType_S63			= 2, standard S-63, such as: <V01X01>(include ENC_ROOT)
* \return	ExSetType_S63Large		= 3, S-63 large media, such as: <M01X01>(include MEDIA.TXT)
* \return	ExSetType_S63LargeSub	= 4, S-63 large media sub, suchas: <M01X01/B1>
*
* @par 示例:
* @code
*	void ChartMngCharts::OnRootChanged(QString root)
*	{
*	    m_esroot = root;
*	    m_estype = EnclDENCGetExchangeSetType(m_esroot.toStdString().c_str());
*	}
* @endcode
*/
ENCL_API EnclDENCExchangeSetType EnclDENCGetExchangeSetType( const char *importPath );

/*! \brief 从尚未安装的海图原始文件中获取海图相关信息
* \param encFile 海图原始文件路径
* \param header 用于存放海图信息的 EnclHeaderRequest对象指针
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	int celledtn;
*	int cellupdn;
*	QString celluadt;
*	QString cellisdt;
*	EnclHeaderRequest header;
*	EnclDENCEncGetHeaderInfo(cellfpath.toStdString().c_str(), &header);
*	celledtn = header.HR_HDR_EDTN;
*	cellupdn = header.HR_HDR_UPDN;
*	celluadt = header.HR_HDR_UADT;
*	cellisdt = header.HR_HDR_ISDT;
* @endcode
*/
ENCL_API bool EnclDENCEncGetHeaderInfo(const char * encFile, EnclHeaderRequest *header);

/*! \brief 根据文件名判断是否是标准S57海图文件
* \param fileName 文件名
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	QDir dir(m_esroot);
*	dir.setSorting(QDir::Name);
*	QStringList filelist;
*	filelist=dir.entryList();
*	for(int i = 0; i < filelist.count(); i++)
*	{
*	    QString filename = filelist.at(i);
*	    QString fpath = dir.absoluteFilePath(filename);
*	    if(EnclDENCIsStandardENCFileName(filename.toStdString().c_str())){
*	        QString cellfname = filename;
*	        QString cellfpath = fpath;
*	
*	        if(!QFile::exists(cellfpath)){
*	            continue;
*	        }
*	    }
*	}
* @endcode
*/
ENCL_API bool EnclDENCIsStandardENCFileName(const char *fileName);
/** @} */

/** @defgroup crtmng 安全证书管理 
* 安全证书查询，检测等
* @{
*/

/*! \brief 检测海图安装文件及其签名文件的合法性，该接口总是配合EnclS63EncGetSignatureName一起来进行检测。
* \param crtFile IHO证书文件路径名称。
* \param sigFile 海图签名文件路径名称。
* \param encFile 海图安装文件路径名称。
* \return 返回错误码，0 表示成功，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	参考 EnclS63EncGetSignatureName() 示例
* @endcode
*/
ENCL_API int EnclS63CrtCheckSignature(const char *crtFile, const char *sigFile, const char *encFile);

/*! \brief 从IHO证书文件中获取公钥，一般用于展示证书信息
* \param crtFile IHO证书文件路径名称。
* \param p 公钥参数 p，需提前分配好 129 bytes 空间。
* \param q 公钥参数 q，需提前分配好 41 bytes 空间。
* \param g 公钥参数 g，需提前分配好 129 bytes 空间。
* \param y 公钥参数 y，需提前分配好 129 bytes 空间。
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	unsigned char p[128+1];
*	unsigned char q[40+1];
*	unsigned char g[128+1];
*	unsigned char y[128+1];
*	
*	EnclS63CrtGetPublicKey(m_parent->GetInstalledCrtPath().toStdString().c_str(), p, q, g, y);
* @endcode
*/
ENCL_API bool EnclS63CrtGetPublicKey(const char *crtFile, unsigned char *p, unsigned char *q, unsigned char *g, unsigned char *y);

/*! \brief 检测IHO证书文件时效性
* \param crtFile IHO证书文件路径名称。
* \return 返回错误码，0 表示成功，22 表示已过期，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	int errnum;
*	errnum = EnclS63CrtCheckDate(m_installedcrtfile.toStdString().c_str());
* @endcode
*/
ENCL_API int EnclS63CrtCheckDate(const char *crtFile);

/*! \brief 从IHO证书文件中获取序列号，一般用于展示证书信息，以两位一组的十六进制形式呈现。
* \param crtFile IHO证书文件路径名称。
* \return 返回long型序列号
*
* @par 示例:
* @code
*	long series = EnclS63CrtGetSerial(m_parent->GetInstalledCrtPath().toStdString().c_str());
*	QString qseries = QString::number(series, 16);
*	int len = qseries.length();
*	len += len % 2;
*	qseries = QString("%1").arg(series, len, 16, QLatin1Char('0')).toUpper();
*	FormatString(qseries, 2, ' ');
*	sainfo += tr("Serial Number: ");
*	sainfo += qseries + QString("\n");
* @endcode
*/
ENCL_API long EnclS63CrtGetSerial(const char *crtFile);

/*! \brief 从IHO证书文件中获取版本号，一般用于展示证书信息。
* \param crtFile IHO证书文件路径名称。
* \return 返回long型版本号
* \return 0: V1     1: V2       2: V3
*
* @par 示例:
* @code
*	long version = EnclS63CrtGetVersion(m_parent->GetInstalledCrtPath().toStdString().c_str());
*	QString qversion = QString("V") + QString::number(version + 1, 10);
*	sainfo = tr("Version: ");
*	sainfo += qversion + QString("\n");
* @endcode
*/
ENCL_API long EnclS63CrtGetVersion(const char *crtFile);

/*! \brief 从IHO证书文件中获取属性信息，一般用于展示证书信息。
* \param crtFile IHO证书文件路径名称。
* \param type 属性类型，详见示例。
* \param outStr 返回的属性信息，注意需要释放。
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	char *issuer;
*	char *subject;
*	char *pubalgo;
*	char *sigalgo;
*	char *takeeffect;
*	char *expirydate;
*	
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_ISSUER, &issuer);
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_SUBJECT, &subject);
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_PUBLIC_KEY_ALGO, &pubalgo);
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_SIG_ALOGRITHM, &sigalgo);
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_TAKEEFFECT, &takeeffect);
*	EnclS63CrtGetString(m_parent->GetInstalledCrtPath().toStdString().c_str(), ENCL_S63CRTATTR_EXPIRYDATE, &expirydate);
*	
*	QString qissuer = QString(issuer);
*	QString qsubject = QString(subject);
*	QString qpubalgo = QString(pubalgo);
*	QString qsigalgo = QString(sigalgo);
*	QString qtakeeffect = QString(takeeffect);
*	QString qexpirydate = QString(expirydate);
*	
*	EnclS63CrtFreeString(issuer);
*	EnclS63CrtFreeString(subject);
*	EnclS63CrtFreeString(pubalgo);
*	EnclS63CrtFreeString(sigalgo);
*	EnclS63CrtFreeString(takeeffect);
*	EnclS63CrtFreeString(expirydate);
*	
*	sainfo += tr("Issuer: ");
*	sainfo += qissuer + QString("\n");
*	sainfo += tr("Not Valid Before: ");
*	sainfo += qtakeeffect + QString("\n");
*	sainfo += tr("Not Valid After: ");
*	sainfo += qexpirydate + QString("\n");
* @endcode
*/
ENCL_API bool EnclS63CrtGetString(const char *crtFile, EnclS63CrtAttrType type, char **outStr);

/*! \brief 释放IHO证书属性信息。
* \param str 属性信息指针，由 EnclS63CrtGetString() 产生。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclS63CrtGetString() 示例
* @endcode
*/
ENCL_API void EnclS63CrtFreeString(char* str);
/** @} */


/** @defgroup pmtmng 许可证管理 
* 许可证安装，删除，更新，查询，检测等
* @{
*/

/*! \brief 检测海图许可证时效性。
* \param permitInfo 海图许可证信息 EnclS63CellPermitInfo 指针。
* \param remain 剩余有效天数，可以是0或者负数。
* \return 返回错误码，0 表示还有30天以上有效期，20 表示不到30天， 15 表示已过期，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	int remain;
*	errcode = EnclS63PermitCheckExpiryDate(permitinfo, &remain);
* @endcode
*/
ENCL_API int EnclS63PermitCheckExpiryDate(const EnclS63CellPermitInfo *permitInfo, int *remain);

/*! \brief 检测海图许可证有效性并返回海图名称，有效日期等信息。
* \param hwId HWID。
* \param permitString 海图许可证信息字符串（64字符）。
* \param cellName 返回的海图名称。
* \param year 返回的有效期截止日期-年。
* \param mon 返回的有效期截止日期-月。
* \param day 返回的有效期截止日期-日。
* \return 返回错误码，0 表示成功，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	int errcode;
*	char cellname[9];
*	int year, mon, day;
*	memset(cellname, 0, 9);
*	errcode = EnclS63PermitCheckExt((const unsigned char *)actHwid, permitinfo->permitString, cellname, &year, &mon, &day);
* @endcode
*/
ENCL_API int EnclS63PermitCheckExt(const unsigned char *hwId, const char *permitString, char *cellName, int *year, int *mon, int *day);

//nouse
ENCL_API int EnclS63PermitCheckHWID(const EnclS63CellPermitInfo *permitInfo, const unsigned char *hwId);
//ENCL_API int EnclS63PermitCreateCellPermit(const unsigned char *hwId, const char *cellName, int year, int mon, int day, const char *key1, const char *key2, char *permitString);

/*! \brief 生成UserPermit。
* \param hwId HWID。
* \param manKey MANKEY。
* \param manId MANID。
* \param userPermit 存放返回的UserPemit字串指针的地址，使用完后需要释放。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	char *pmtstr =NULL;
*	bool ret = EnclS63CreateUserPermit(hwid, mkey, mid, &pmtstr);
*	if (pmtstr != NULL)
*	{
*	    EnclS63FreeUserPermit(pmtstr);
*	}
* @endcode
*/
ENCL_API bool EnclS63CreateUserPermit(unsigned char *hwId, unsigned char *manKey, unsigned char *manId, char **userPermit);

/*! \brief 释放UserPermit字串。
* \param userPermit 待释放的UserPemit字串指针。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclS63CreateUserPermit() 示例
* @endcode
*/
ENCL_API void EnclS63FreeUserPermit(char* userPermit);

//nouse
ENCL_API int EnclS63PermitDecryptExt(const unsigned char *hwId, const char *permitString, char *cellName, int *year, int *mon, int *day, char *key1, char *key2);

/*! \brief 创建EnclS63CellPermitList数据结构对象，用于处理PERMIT.TXT。
* \return 返回对象指针。
*
* @par 示例:
* @code
*	//load installed permitlist
*	EnclS63CellPermitList *permitlist = EnclS63PermitListCreate();
*	if(QFile::exists(m_parent->GetInstalledPmtPath())){
*	    EnclS63PermitListAddedFromFile(permitlist, m_parent->GetInstalledPmtPath().toStdString().c_str(), (const unsigned char *)actHwid, true, NULL, NULL);
*	}
*
*	QVector<EnclS63CellPermitInfo> selinfos;
*	for(int i = 0; i < selids.size(); i++){
*	    EnclS63CellPermitInfo oneinfo = permitlist->permitinfo[selids[i]];
*	    selinfos.push_back(oneinfo);
*	}
*
*	//delete selections
*	for(int i = 0; i < selinfos.size(); i++){
*	    EnclS63PermitListDelPermit(permitlist, &selinfos[i]);
*	}
*
*	//save the deleted install permitlist
*	EnclS63PermitListWriteToFile(permitlist, m_parent->GetInstalledPmtPath().toStdString().c_str());
*
*	//free
*	EnclS63PermitListFree(permitlist);
* @endcode
*/
ENCL_API EnclS63CellPermitList * EnclS63PermitListCreate();

//nouse
ENCL_API int EnclS63PermitRead(const char *permitFile, EnclS63CellPermitInfo **permitInfo, int *nInfos, EnclS63PermitCallback callback, void *cbData);

/*! \brief 从PERMIT.TXT读取信息到EnclS63CellPermitList数据结构对象中。
* \param permitList 对象指针。
* \param permitFile permit文件路径名称，一般为PERMIT.TXT。
* \param hwId HWID
* \param mergeFlag true表示如果待读入permit信息在对象中已存在（海图名称和DataServer名称均相同），则保留有效期最新的那一个。false表示无条件追加。
* \param callback 回调函数，该回调函数会逐条得到每条permit信息，用于检测有效性，可以为null
* \param cbData 如果调用者需要传递一些信息给回调函数，则需要用到该参数，一般情况用不到，null即可。
* \return 返回错误码，0 表示成功，其他错误码可以查阅相关文档。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListCreate() 示例
* @endcode
*/
ENCL_API int EnclS63PermitListAddedFromFile(EnclS63CellPermitList * permitList, const char *permitFile, const unsigned char *hwId, bool mergeFlag, EnclS63PermitCallback callback, void *cbData);

/*! \brief 将EnclS63CellPermitList数据结构对象中的信息写入到文件。
* \param permitList 对象指针。
* \param permitFile 文件路径名称。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListCreate() 示例
* @endcode
*/
ENCL_API void EnclS63PermitListWriteToFile(EnclS63CellPermitList * permitList, const char *permitFile);

//nouse
/*! \brief 将单个permit信息添加到数据结构对象中。
* \param permitList 对象指针。
* \param permitInfo 单个permit信息指针。
* \param mergeFlag true表示如果待读入permit信息在对象中已存在（海图名称和DataServer名称均相同），则保留有效期最新的那一个。false表示无条件追加。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListDelPermit() 示例
* @endcode
*/
ENCL_API bool EnclS63PermitListAddPermit(EnclS63CellPermitList *permitList, const EnclS63CellPermitInfo *permitInfo, bool mergedflag);

/*! \brief 从数据结构对象中删除相匹配的单个permit信息。
* \param permitList 对象指针。
* \param permitInfo 单个permit信息指针。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListCreate() 示例
* @endcode
*/
ENCL_API bool EnclS63PermitListDelPermit(EnclS63CellPermitList *permitList, const EnclS63CellPermitInfo *permitInfo);

/*! \brief 从数据结构对象中筛选并返回相应的permit信息。
* \param permitList 对象指针。
* \param cellName 海图名称。
* \param permType 目前只支持一种类型 ENCL_S63_PERMIT。
* \param permitInfos 返回符合要求的permit信息指针。
* \return 返回符合要求的permit个数。
*
* @par 示例:
* @code
*	EnclS63CellPermitInfo *pmtinfos = NULL;
*	int np = EnclS63PermitListGetPermitsByName(pmtlist, cellname.toStdString().c_str(), ENCL_S63_PERMIT, &pmtinfos);
*   EnclS63PermitListFreeInfos(pmtinfos);
* @endcode
*/
ENCL_API int EnclS63PermitListGetPermitsByName(const EnclS63CellPermitList *permList, const char *cellName, EnclChartPermitType permType, EnclS63CellPermitInfo **permitInfos); 

/*! \brief 释放 EnclS63CellPermitList 数据结构对象。
* \param permitList 对象指针，通常由 EnclS63PermitListCreate() 产生。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListCreate() 示例
* @endcode
*/
ENCL_API void EnclS63PermitListFree(EnclS63CellPermitList *permitList);

/*! \brief 释放 EnclS63CellPermitInfo 信息
* \param permitInfo EnclS63CellPermitInfo 指针，通常由 EnclS63PermitListGetPermitsByName 产生。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclS63PermitListGetPermitsByName() 示例
* @endcode
*/
ENCL_API void EnclS63PermitListFreeInfos(EnclS63CellPermitInfo* permitInfo);
/** @} */

/** @defgroup chartdraw 海图显示
* 海图显示控制等
* @{
*/

/*! \brief 绘制海图
* \return 返回包含有海图绘制信息的内存指针。
*
* @par 示例:
* @code
*	unsigned char * pPixBuf = EnclDrawChart();
*	if(pPixBuf)
*	{
*	    QImage img = QImage (pPixBuf, w, h, QImage::Format_RGB32);
*	    painter.drawImage(0,0, img);
*	}
* @endcode
*/
ENCL_API EnclImgDataPtr EnclDrawChart();

ENCL_API void EnclDrawSetDPI(int dpi);

/*! \brief 手动加载指定海图
* \param chartName 指定海图名称。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	EnclDrawManualLoadChart(chartname.toStdString().c_str());
* @endcode
*/
ENCL_API bool EnclDrawManualLoadChart(const char* chartName);

/*! \brief 设置海图加载模式（自动/手动）
* \param mode ENCL_LOAD_MODE_AUTO 自动， ENCL_LOAD_MODE_MANUAL 手动。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	EnclDrawSetLoadMode(ENCL_LOAD_MODE_AUTO);
*	EnclDrawSetLoadMode(ENCL_LOAD_MODE_MANUAL);
* @endcode
*/
ENCL_API bool EnclDrawSetLoadMode(EnclChartLoadMode mode);

/*! \brief 获取海图加载模式
* \return 返回海图加载模式，ENCL_LOAD_MODE_AUTO 自动， ENCL_LOAD_MODE_MANUAL 手动。
*
* @par 示例:
* @code
    EnclChartLoadMode lm = EnclDrawGetLoadMode();
* @endcode
*/
ENCL_API EnclChartLoadMode EnclDrawGetLoadMode();

/*! \brief 设置颜色模式
* \param colorScheme "DAY" 白天，"DUSK" 黄昏，"NIGHT" 晚上。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	case 17:
*	    EnclDrawSetColorScheme("DAY");
*	    break;
*	case 18:
*	    EnclDrawSetColorScheme("DUSK");
*	    break;
*	case 19:
*	    EnclDrawSetColorScheme("NIGHT");
*	    break;
* @endcode
*/
ENCL_API bool EnclDrawSetColorScheme(const char* colorScheme);

/*! \brief 获取颜色模式
* \return 返回颜色模式字符串，参考 EnclDrawSetColorScheme。
*
* @par 示例:
* @code
*	int rs = 3;
*	QString scheme = QString(EnclDrawGetColorScheme());
*	if (scheme.compare("DAY") == 0)
*		rs = 3;
*	else if (scheme.compare("DUSK") == 0)
*		rs = 2;
*	else if (scheme.compare("NIGHT") == 0)
*		rs = 1;
* @endcode
*/
ENCL_API const char* EnclDrawGetColorScheme();

/*! \brief 设置显示类别
* \param cat ENCL_BASE 基本，ENCL_STANDARD 标准，ENCL_OTHER 全部。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetDisplayCategory(ENCL_OTHER);
* @endcode
*/
ENCL_API void EnclDrawSetDisplayCategory(EnclDisplayCategory cat);

/*! \brief 获取显示类别
* \return 返回显示类别，参考 EnclDrawSetDisplayCategory。
*
* @par 示例:
* @code
*	EnclDisplayCategory dc = EnclDrawGetDisplayCategory();
*	switch (dc)
*	{
*	case ENCL_BASE:
*	    m_viewClassBtn->setText(tr("BASE"));
*	    break;
*	case ENCL_STANDARD:
*	    m_viewClassBtn->setText(tr("STANDARD"));
*	    break;
*	case ENCL_OTHER:
*	    m_viewClassBtn->setText(tr("OTHER"));
*	    break;
*	default:
*	    break;
*	}
* @endcode
*/
ENCL_API EnclDisplayCategory EnclDrawGetDisplayCategory();

/*! \brief 设置安全等深线
* \param value 安全等深线值（米）。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetSafetyContour(10);
* @endcode
*/
ENCL_API void EnclDrawSetSafetyContour(double value);

/*! \brief 获取安全等深线
* \return 返回安全等深线值（米）。
*
* @par 示例:
* @code
*	double ret = EnclDrawGetSafetyContour();
* @endcode
*/
ENCL_API double EnclDrawGetSafetyContour();

/*! \brief 设置浅水等深线
* \param value 浅水等深线值（米）。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShallowContour(10);
* @endcode
*/
ENCL_API void EnclDrawSetShallowContour(double value);

/*! \brief 获取浅水等深线
* \return 返回浅水等深线值（米）。
*
* @par 示例:
* @code
*	double ret = EnclDrawGetShallowContour();
* @endcode
*/
ENCL_API double EnclDrawGetShallowContour();

/*! \brief 设置深水等深线
* \param value 深水等深线值（米）。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetDeepContour(10);
* @endcode
*/
ENCL_API void EnclDrawSetDeepContour(double value);

/*! \brief 获取深水等深线
* \return 返回深水等深线值（米）。
*
* @par 示例:
* @code
*	double ret = EnclDrawGetDeepContour();
* @endcode
*/
ENCL_API double EnclDrawGetDeepContour();

/*! \brief 设置安全水深
* \param value 安全水深值（米）。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetSafetyDepth(10);
* @endcode
*/
ENCL_API void EnclDrawSetSafetyDepth(double value);

/*! \brief 获取安全水深
* \return 返回安全水深值（米）。
*
* @par 示例:
* @code
*	double ret = EnclDrawGetSafetyDepth();
* @endcode
*/
ENCL_API double EnclDrawGetSafetyDepth();

//obsolete
ENCL_API void EnclDrawSetDisplayStyle(EnclDisplayStyle style);
ENCL_API EnclDisplayStyle EnclDrawGetDisplayStyle();

/*! \brief 设置点符号模式
* \param type ENCL_SIMPLIFIED_POINT 简化符号，ENCL_TRADITIONAL_POINT 传统符号。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetPointSymbolType(ENCL_TRADITIONAL_POINT);
* @endcode
*/
ENCL_API void EnclDrawSetPointSymbolType(EnclPointSymbolType type);

/*! \brief 获取点符号模式
* \return 返回点符号模式，参考 EnclDrawSetPointSymbolType。
*
* @par 示例:
* @code
*	EnclPointSymbolType ret = EnclDrawGetPointSymbolType();
* @endcode
*/
ENCL_API EnclPointSymbolType EnclDrawGetPointSymbolType();

/*! \brief 设置面边线模式
* \param type ENCL_PLAIN_BOUNDARIES 平滑边线，ENCL_SYMBOLIZED_BOUNDARIES 符号化边线。
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetBoundarySymbolType(ENCL_SYMBOLIZED_BOUNDARIES);
* @endcode
*/
ENCL_API void EnclDrawSetBoundarySymbolType(EnclBoundarySymbolType type);

/*! \brief 获取面边线模式
* \return 返回面边线模式，参考 EnclDrawSetBoundarySymbolType。
*
* @par 示例:
* @code
*	EnclBoundarySymbolType style  = EnclDrawGetBoundarySymbolType();
* @endcode
*/
ENCL_API EnclBoundarySymbolType EnclDrawGetBoundarySymbolType();

/*! \brief 设置最小比例尺开关。某些物标在小于某个比例尺时可以隐藏。这个开关就是用于控制是否隐藏。
* \param flag true表示隐藏，false表示不隐藏
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetUseAutoScamin(true);
* @endcode
*/
ENCL_API void EnclDrawSetUseAutoScamin(bool flag);

/*! \brief 获取最小比例尺开关状态
* \return 返回最小比例尺开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetUseAutoScamin();
* @endcode
*/
ENCL_API bool EnclDrawGetUseAutoScamin();

/*! \brief 设置浅水图案开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowShallowPattern(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowShallowPattern(bool flag);

/*! \brief 获取浅水图案开关状态
* \return 返回浅水开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowShallowPattern();
* @endcode
*/
ENCL_API bool EnclDrawGetShowShallowPattern();

/*! \brief 设置等深线数字开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowContourLabels(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowContourLabels(bool flag);

/*! \brief 获取等深线数字开关状态
* \return 返回等深线数字开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowContourLabels();
* @endcode
*/
ENCL_API bool EnclDrawGetShowContourLabels();

/*! \brief 设置水深点开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowSoundings(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowSoundings(bool flag);

/*! \brief 获取水深点开关状态
* \return 返回水深点开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowSoundings();
* @endcode
*/
ENCL_API bool EnclDrawGetShowSoundings();

/*! \brief 设置某些groups的物标显示或隐藏
* \param action ENCL_VGA_SET 显示，ENCL_VGA_CLEAR 隐藏，ENCL_VGA_SET_ALL 全部显示，ENCL_VGA_CLEAR_ALL 全部隐藏
* \param groups 待操作的group数组指针
* \param nGroups group个数
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int * pGrps = new int[m_params.size() - 1];
*	for(int i = 1; i < m_params.size(); ++ i)
*	    pGrps[i - 1] = toInt(m_params[i]);
*	
*	EnclDrawSetViewingGroup(ENCL_VGA_SET, pGrps, m_params.size() - 1);
*	
*	if(pGrps)
*	    delete [] pGrps;
* @endcode
*/
ENCL_API bool EnclDrawSetViewingGroup(EnclViewingGroupAction action, int* groups, int nGroups);

/*! \brief 获取某个group物标的开关状态
* \param group group值。
* \return 返回true 表示显示状态，返回false表示隐藏状态。
*
* @par 示例:
* @code
*	bool bStatus = EnclDrawGetViewingGroup(20010);
* @endcode
*/
ENCL_API bool EnclDrawGetViewingGroup(int group);

/*! \brief 设置某些layers的物标显示或隐藏
* \param layers 待操作的layers数组指针
* \param nLayers layers个数
* \param action ENCL_VGA_SET 显示，ENCL_VGA_CLEAR 隐藏，ENCL_VGA_SET_ALL 全部显示，ENCL_VGA_CLEAR_ALL 全部隐藏
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int * layers = new int[m_params.size() - 1];
*	for(int i = 1; i < m_params.size(); ++ i)
*	    layers[i - 1] = toInt(m_params[i]);
*	
*	EnclDrawSetViewGroupLayer(ENCL_VGA_SET, layers, m_params.size() - 1);
*	
*	if(layers)
*	    delete [] layers;
* @endcode
*/
ENCL_API bool EnclDrawSetViewGroupLayer(int * layers, int nLayers, EnclViewingGroupAction action);

/*! \brief 获取某个layer物标的开关状态
* \param layer layer值。
* \return 返回true 表示显示状态，返回false表示隐藏状态。
*
* @par 示例:
* @code
*	bool bStatus = EnclDrawGetViewGroupLayer(7);
* @endcode
*/
ENCL_API bool EnclDrawGetViewGroupLayer(int layer);

/*! \brief 设置某些groups的文本显示或隐藏
* \param action ENCL_TGA_SET 显示，ENCL_TGA_CLEAR 隐藏，ENCL_TGA_SET_ALL 全部显示，ENCL_TGA_CLEAR_ALL 全部隐藏
* \param groups 待操作的group数组指针
* \param nGroups group个数
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int * pGrps = new int[m_params.size() - 1];
*	for(int i = 1; i < m_params.size(); ++ i)
*	    pGrps[i - 1] = toInt(m_params[i]);
*	
*	EnclDrawSetTextGroup(ENCL_TGA_SET, pGrps, m_params.size() - 1);
*	
*	if(pGrps)
*	    delete [] pGrps;
* @endcode
*/
ENCL_API bool EnclDrawSetTextGroup(EnclTextGroupAction action, int* groups, int nGroups);

/*! \brief 获取某个group文本的开关状态
* \param group group值。
* \return 返回true 表示显示状态，返回false表示隐藏状态。
*
* @par 示例:
* @code
*	bool bStatus = EnclDrawGetTextGroup(11);
* @endcode
*/
ENCL_API bool EnclDrawGetTextGroup(int group);

/*! \brief 设置某些layers的文本显示或隐藏
* \param layers 待操作的layers数组指针
* \param nLayers layers个数
* \param action ENCL_TGA_SET 显示，ENCL_TGA_CLEAR 隐藏，ENCL_TGA_SET_ALL 全部显示，ENCL_TGA_CLEAR_ALL 全部隐藏
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int * layers = new int[m_params.size() - 1];
*	for(int i = 1; i < m_params.size(); ++ i)
*	    layers[i - 1] = toInt(m_params[i]);
*	
*	EnclDrawSetTextGroupLayer(ENCL_TGA_SET, layers, m_params.size() - 1);
*	
*	if(layers)
*	    delete [] layers;
* @endcode
*/
ENCL_API bool EnclDrawSetTextGroupLayer(int * layers, int nLayers, EnclTextGroupAction action);

/*! \brief 获取某个layer文本的开关状态
* \param layer layer值。
* \return 返回true 表示显示状态，返回false表示隐藏状态。
*
* @par 示例:
* @code
*	bool bStatus = EnclDrawGetViewGroupLayer(1);
* @endcode
*/
ENCL_API bool EnclDrawGetTextGroupLayer(int layer);

/*! \brief 设置文本全写（简写）开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowFullText(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowFullText(bool flag);

/*! \brief 获取文本全写（简写）开关状态
* \return 返回文本全写（简写）开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowFullText();
* @endcode
*/
ENCL_API bool EnclDrawGetShowFullText();

/*! \brief 设置文本开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowText(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowText(bool flag);

/*! \brief 获取文本开关状态
* \return 返回文本开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowText();
* @endcode
*/
ENCL_API bool EnclDrawGetShowText();

/*! \brief 设置灯光线全画（简画）开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowFullLightSectorLines(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowFullLightSectorLines(bool flag);

/*! \brief 获取灯光线全画（简画）开关状态
* \return 返回灯光线全画（简画）开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowFullLightSectorLines();
* @endcode
*/
ENCL_API bool EnclDrawGetShowFullLightSectorLines();

/*! \brief 设置超比例尺开关，用于控制在超比例尺状态下是否显示相关标记。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowOverScale(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowOverScale(bool flag);

/*! \brief 获取超比例尺开关状态，参考 EnclDrawSetShowOverScale
* \return 返回超比例尺开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowOverScale();
* @endcode
*/
ENCL_API bool EnclDrawGetShowOverScale();

/*! \brief 设置精度符号开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowAccuracy(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowAccuracy(bool flag);

/*! \brief 获取精度符号开关状态
* \return 返回精度符号开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowAccuracy();
* @endcode
*/
ENCL_API bool EnclDrawGetShowAccuracy();

/*! \brief 设置海图边界线开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowChartBoundary(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowChartBoundary(bool flag);

/*! \brief 获取海图边界线开关状态
* \return 返回海图边界线开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowChartBoundary();
* @endcode
*/
ENCL_API bool EnclDrawGetShowChartBoundary();

/*! \brief 设置两色水深开关。
* \param flag true表示打开，显示两色水深，false表示关闭，显示四色水深
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowTwoShades(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowTwoShades(bool flag);

/*! \brief 获取两色水深开关状态，参考 EnclDrawSetShowTwoShades
* \return 返回两色水深开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowTwoShades();
* @endcode
*/
ENCL_API bool EnclDrawGetShowTwoShades();

/*! \brief 设置浅水危险物标志开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowShallowWaterDangers(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowShallowWaterDangers(bool flag);

/*! \brief 获取浅水危险物标志开关状态
* \return 返回浅水危险物标志开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowShallowWaterDangers();
* @endcode
*/
ENCL_API bool EnclDrawGetShowShallowWaterDangers();

/*! \brief 设置经纬网格线开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowGrid(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowGrid(bool flag);

/*! \brief 获取经纬网格线开关状态
* \return 返回经纬网格线开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowGrid();
* @endcode
*/
ENCL_API bool EnclDrawGetShowGrid();

/*! \brief 设置未知物标开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowUnknownObject(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowUnknownObject(bool flag);

/*! \brief 获取未知物标开关状态
* \return 返回未知物标开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowUnknownObject();
* @endcode
*/
ENCL_API bool EnclDrawGetShowUnknownObject();

/*! \brief 设置附加信息标志开关。用于控制拥有附加信息的物标是否显示附加信息标志。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetHighlightAdditionalChartInfo(true);
* @endcode
*/
ENCL_API void EnclDrawSetHighlightAdditionalChartInfo(bool flag);

/*! \brief 获取附加信息标志开关状态，参考 EnclDrawSetHighlightAdditionalChartInfo
* \return 返回附加信息标志开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetHighlightAdditionalChartInfo();
* @endcode
*/
ENCL_API bool EnclDrawGetHighlightAdditionalChartInfo();

/*! \brief 设置附加文件标志开关。用于控制拥有附加文件的物标是否显示附加信息标志。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetHighlightAdditionalChartInfoDoc(true);
* @endcode
*/
ENCL_API void EnclDrawSetHighlightAdditionalChartInfoDoc(bool flag);

/*! \brief 获取附加文件标志开关状态，参考 EnclDrawSetHighlightAdditionalChartInfoDoc
* \return 返回附加文件标志开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetHighlightAdditionalChartInfoDoc();
* @endcode
*/
ENCL_API bool EnclDrawGetHighlightAdditionalChartInfoDoc();

/*! \brief 设置检视日期范围
* \param start 起始时间戳。
* \param end 结束时间戳。
* \return 无。
*
* @par 示例:
* @code
*	unsigned long start = Str2Epoch(m_params[0]);
*	unsigned long end = Str2Epoch(m_params[1]);
*	EnclDrawSetViewingDateRange(start, end);
*	
*	EnclDrawGetViewingDateRange(&start, &end);
*	std::string sStart = Epoch2Str(start);
*	std::string sEnd = Epoch2Str(end);
* @endcode
*/
ENCL_API void EnclDrawSetViewingDateRange(unsigned long start, unsigned long end);

/*! \brief 获取检视日期范围
* \param start 返回的起始时间戳。
* \param end 返回的结束时间戳。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclDrawSetViewingDateRange() 示例
* @endcode
*/
ENCL_API void EnclDrawGetViewingDateRange(unsigned long *start, unsigned long *end);

/*! \brief 设置检视日期
* \param date 检视时间戳。
* \return 无。
*
* @par 示例:
* @code
*	unsigned long tm = Str2Epoch(m_params[0]);
*	EnclDrawSetViewingDate(tm);
*	
*	unsigned long date;
*	date = EnclDrawGetViewingDate();
*	std::string sDate = Epoch2Str(date);
* @endcode
*/
ENCL_API void EnclDrawSetViewingDate(unsigned long date);

/*! \brief 获取检视日期
* \return 返回检视时间戳
*
* @par 示例:
* @code
*	参考 EnclDrawSetViewingDate() 示例
* @endcode
*/
ENCL_API unsigned long EnclDrawGetViewingDate();

/*! \brief 设置日期依赖标志开关。用于控制拥有日期依赖的物标是否显示日期依赖标志。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetHighlightDateDependents(true);
* @endcode
*/
ENCL_API void EnclDrawSetHighlightDateDependents(bool flag);

/*! \brief 获取日期依赖标志开关状态，参考 EnclDrawSetHighlightDateDependents
* \return 返回日期依赖标志开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetHighlightDateDependents();
* @endcode
*/
ENCL_API bool EnclDrawGetHighlightDateDependents();

/*! \brief 设置中文显示开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetDisplayChineseLanguage(true);
* @endcode
*/
ENCL_API void EnclDrawSetDisplayChineseLanguage(bool flag);

/*! \brief 获取中文显示开关状态
* \return 返回中文显示开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetDisplayChineseLanguage();
* @endcode
*/
ENCL_API bool EnclDrawGetDisplayChineseLanguage();

/*! \brief 设置本地语言开关。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetDisplayNationalLanguage(true);
* @endcode
*/
ENCL_API void EnclDrawSetDisplayNationalLanguage(bool bSet);

/*! \brief 获取本地语言开关状态
* \return 返回本地语言开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetDisplayNationalLanguage();
* @endcode
*/
ENCL_API bool EnclDrawGetDisplayNationalLanguage();

/*! \brief 获取显示库版本
* \return 返回显示库版本号字符串指针。
*
* @par 示例:
* @code
*	const char* ver = EnclDrawGetPresLibVersion();
* @endcode
*/
ENCL_API const char* EnclDrawGetPresLibVersion();

/*! \brief 设置自动检视日期开关，用于控制是否自动将检视日期设定为当天
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetViewingDateAuto(true);
* @endcode
*/
ENCL_API void EnclDrawSetViewingDateAuto(bool flag);

/*! \brief 获取自动检视日期开关状态
* \return 返回自动检视日期开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetViewingDateAuto();
* @endcode
*/
ENCL_API bool EnclDrawGetViewingDateAuto();

/*! \brief 获取物标layer列表
* \param nLayers 返回的layer个数。
* \return 返回layer数组指针。
*
* @par 示例:
* @code
*	int nlyrs;
*	int *viewlyrs = EnclDrawGetViewGroupLayerList(nlyrs);
*	EnclDrawFreeViewGroupLayerList(viewlyrs);
* @endcode
*/
ENCL_API int *EnclDrawGetViewGroupLayerList(int &nLayers);

/*! \brief 获取物标group列表
* \param nGroups 返回的group个数。
* \return 返回group数组指针。
*
* @par 示例:
* @code
*	int ngrps;
*	int *viewgrps = EnclDrawGetViewGroupList(ngrps);
*	EnclDrawFreeViewGroupList(viewgrps);
* @endcode
*/
ENCL_API int *EnclDrawGetViewGroupList(int &nGroups);

/*! \brief 获取文本layer列表
* \param nLayers 返回的layer个数。
* \return 返回layer数组指针。
*
* @par 示例:
* @code
*	int nlyrs;
*	int *textlyrs = EnclDrawGetTextGroupLayerList(nlyrs);
*	EnclDrawFreeTextGroupLayerList(textlyrs);
* @endcode
*/
ENCL_API int *EnclDrawGetTextGroupLayerList(int &nLayers);

/*! \brief 获取文本group列表
* \param nGroups 返回的group个数。
* \return 返回group数组指针。
*
* @par 示例:
* @code
*	int ngrps;
*	int *textgrps = EnclDrawGetTextGroupList(ngrps);
*	EnclDrawFreeTextGroupList(textgrps);
* @endcode
*/
ENCL_API int *EnclDrawGetTextGroupList(int &nGroups);

/*! \brief 释放物标layer列表
* \param layers layer数组指针。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclDrawGetViewGroupLayerList() 示例
* @endcode
*/
ENCL_API void EnclDrawFreeViewGroupLayerList(int *layers);

/*! \brief 释放物标group列表
* \param groups group数组指针。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclDrawGetViewGroupList() 示例
* @endcode
*/
ENCL_API void EnclDrawFreeViewGroupList(int *groups);

/*! \brief 释放文本layer列表
* \param layers layer数组指针。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclDrawGetTextGroupLayerList() 示例
* @endcode
*/
ENCL_API void EnclDrawFreeTextGroupLayerList(int *layers);

/*! \brief 释放文本group列表
* \param groups group数组指针。
* \return 无。
*
* @par 示例:
* @code
*	参考 EnclDrawGetTextGroupList() 示例
* @endcode
*/
ENCL_API void EnclDrawFreeTextGroupList(int *groups);

//internal
ENCL_API int EnclDrawGetEngineType();

/*! \brief 手动卸载（并非删除）指定海图
* \param chartName 海图名称。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	EnclDrawManualUnloadChart(chartname.toStdString().c_str());
* @endcode
*/
ENCL_API bool EnclDrawManualUnloadChart(const char* chartName);

//obsolete
ENCL_API void EnclDrawSetLineSymbolType(EnclLineSymbolStyle value);
ENCL_API EnclLineSymbolStyle EnclDrawGetLineSymbolStyle();

/*! \brief 设置安全等深线以外的等深线开关，用于控制除了安全等深线以外的等深线是否显示。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowOtherContour(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowOtherContour(bool flag);

/*! \brief 获取安全等深线以外的等深线开关状态，参考 EnclDrawSetShowOtherContour
* \return 返回安全等深线以外的等深线开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowOtherContour();
* @endcode
*/
ENCL_API bool EnclDrawGetShowOtherContour();

/*! \brief 设置单色模式。
* \param monoState 0：非单色模式，-1：背景黑前景白，1：背景白前景黑
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetMonoView(0);
* @endcode
*/
ENCL_API void EnclDrawSetMonoView(int monoState);

/*! \brief 获取单色模式状态，参考 EnclDrawSetMonoView
* \return 单色模式状态值。0：非单色模式，-1：背景黑前景白，1：背景白前景黑
*
* @par 示例:
* @code
*	int mode = EnclDrawGetMonoView();
* @endcode
*/
ENCL_API int EnclDrawGetMonoView();

/*! \brief 恢复默认设置
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawResetToDefaultSettings();
* @endcode
*/
ENCL_API void EnclDrawResetToDefaultSettings();

/*! \brief 设置不同比例尺海图分界线开关，同时也和EnclDrawSetShowOverScale 配合用于控制scale pattern的显示。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowScaleBoundaries(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowScaleBoundaries(bool flag);

/*! \brief 获取不同比例尺海图分界线开关状态，参考 EnclDrawSetShowScaleBoundaries
* \return 返回不同比例尺海图分界线开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowScaleBoundaries();
* @endcode
*/
ENCL_API bool EnclDrawGetShowScaleBoundaries();

/*! \brief 获取日期检测开关状态，参考 EnclDrawSetShowDateCheck
* \return 返回日期检测开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowDateCheck();
* @endcode
*/
ENCL_API bool EnclDrawGetShowDateCheck();

/*! \brief 设置日期检测开关，用于控制日期依赖物标的显示。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowDateCheck(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowDateCheck(bool flag);

//nouse
ENCL_API void EnclDrawText(const char * str, int x, int y, int size, unsigned char R, 
	unsigned char G, unsigned char B);
ENCL_API void EnclDrawClearText();

/*! \brief 设置孤立危险物开关，用于控制所有孤立危险物的显示。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowIsolatedDangerObjects(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowIsolatedDangerObjects(bool bShow); //EXT

/*! \brief 获取孤立危险物开关状态
* \return 返回孤立危险物开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowIsolatedDangerObjects();
* @endcode
*/
ENCL_API bool EnclDrawGetShowIsolatedDangerObjects(); //EXT
/** @} */

/** @defgroup viewport 视口设置 
* 视口尺寸设置，旋转，漫游等
* @{
*/

/*! \brief 设置视口大小
* \param width 宽（像素）
* \param height 高（像素）
* \return 无。
*
* @par 示例:
* @code
*	EnclViewSetSize(1916, 929);
* @endcode
*/
ENCL_API void EnclViewSetSize(int width, int height);

/*! \brief 视口平移，从(x1, y1)到（x2, y2)
* \param x1 起始屏幕坐标 x
* \param y1 起始屏幕坐标 y
* \param x2 结束屏幕坐标 x
* \param y2 起始屏幕坐标 y
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int fromX = toInt(m_params[0]);
*	int fromY = toInt(m_params[1]);
*	int toX = toInt(m_params[2]);
*	int toY = toInt(m_params[3]);
*	EnclViewPan(fromX, fromY, toX, toY);
* @endcode
*/
ENCL_API bool EnclViewPan(int x1, int y1, int x2, int y2);

/*! \brief 视口缩放
* \param x 缩放中心点屏幕坐标 x
* \param y 缩放中心点屏幕坐标 y
* \param modulus 缩放系数
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	int posX = toInt(m_params[0]);
*	int posY = toInt(m_params[1]);
*	double zoomFactor = toDouble(m_params[2]);
*	EnclViewZoom(posX, posY, zoomFactor);
* @endcode
*/
ENCL_API bool EnclViewZoom(int x, int y, double modulus);

//obsolete
ENCL_API bool EnclViewRotate(double angle);

/*! \brief 获取视口中心经纬度
* \param lon 返回中心点经度
* \param lat 返回中心点纬度
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	double lon;
*	double lat;
*	EnclViewGetCenter(&lon, &lat);
* @endcode
*/
ENCL_API bool EnclViewGetCenter(double *lon, double *lat);

//obsolete
ENCL_API void EnclViewCenter(double lon, double lat);

/*! \brief 设置视口比例尺
* \param scale 待设置的比例尺
* \return 无
*
* @par 示例:
* @code
*	EnclViewSetScale(52000);
* @endcode
*/
ENCL_API void EnclViewSetScale(double scale);

/*! \brief 获取视口比例尺
* \return 返回视口当前比例尺
*
* @par 示例:
* @code
*	double scale = EnclViewGetScale();
* @endcode
*/
ENCL_API double EnclViewGetScale();

/*! \brief 设置视口中心
* \param lon 中心点经度
* \param lat 中心点纬度
* \return 无。
*
* @par 示例:
* @code
*	EnclViewSetCenter(104.321, 32.1);
* @endcode
*/
ENCL_API void EnclViewSetCenter(double lon, double lat);

/*! \brief 设置视口旋转角度
* \param rotation 旋转角度（度），正北为0，顺时针为正
* \return 无。
*
* @par 示例:
* @code
*	EnclViewSetRotation(30);
* @endcode
*/
ENCL_API void EnclViewSetRotation(double rotation);

/*! \brief 获取视口信息
* \param vp 存放返回信息的结构指针
* \return 无。
*
* @par 示例:
* @code
*	EnclViewPort vp;
*	EnclViewGetViewPort(&vp);
* @endcode
*/
ENCL_API void EnclViewGetViewPort(EnclViewPort * vp);

/*! \brief 设置视口范围
* \param range 视口高度的海里数
* \return 无。
*
* @par 示例:
* @code
*	EnclViewSetRange(8.5);
* @endcode
*/
ENCL_API void EnclViewSetRange(double range);

/*! \brief 获取视口范围
* \return 返回视口高度的海里数
*
* @par 示例:
* @code
*	double range = EnclViewGetRange();
* @endcode
*/
ENCL_API double EnclViewGetRange();

/*! \brief 判断180度经线是否在视口内
* \return 是返回true，否返回false。
*
* @par 示例:
* @code
*	bool cross = EnclViewCrossAntiMeridian();
* @endcode
*/
ENCL_API bool EnclViewCrossAntiMeridian();

/*! \brief 创建视口
* \return 返回视口id。
*
* @par 示例:
* @code
*	int vid = EnclViewCreateView();
* @endcode
*/
ENCL_API unsigned int EnclViewCreateView();

/*! \brief 获取当前视口id
* \return 返回当前视口id。
*
* @par 示例:
* @code
*	int vid = EnclViewGetCurrentView();
* @endcode
*/
ENCL_API unsigned int EnclViewGetCurrentView();

/*! \brief 设置当前视口
* \param viewId 待设置的视口id, 通常由EnclViewCreateView()产生
* \return 成功返回true, 否则返回false。
*
* @par 示例:
* @code
*	bool flag = EnclViewSetCurrentView(2);
* @endcode
*/
ENCL_API bool EnclViewSetCurrentView(unsigned int viewId);

/*! \brief 获取视口个数
* \return 返回视口个数。
*
* @par 示例:
* @code
*	int vcnt = EnclViewGetViewCount();
* @endcode
*/
ENCL_API int EnclViewGetViewCount();
ENCL_API int* EnclViewGetViews(int * nCnt);  //EXT
/** @} */

/** @defgroup query 信息查询 
* 查询海图信息等
* @{
*/

/*! \brief 查询海图头信息
* \param chartName 待查询海图名称（8位字符）
* \return 返回海图头信息EnclHeaderRequest结构
*
* @par 示例:
* @code
*	EnclHeaderRequest header = EnclQueryGetHeaderInfo("GB4X0000");
* @endcode
*/
ENCL_API EnclHeaderRequest EnclQueryGetHeaderInfo(const char * chartName);

/*! \brief 拾取物标
* \param lon 拾取中心点经度
* \param lat 拾取中心点纬度
* \param pickRad 拾取半径（像素）
* \param count 返回拾取的物标个数
* \return 返回拾取物标id
*
* @par 示例:
* @code
*	int nCnt;
*	EnclObjectPtr * pObjs = EnclQueryPickObjects(60.12345, 31.23456, 20, &nCnt);
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*		int len;
*		char * pAttrs = EnclQueryGetObjectAttributes(pObjs[i], &len);
*		EnclQueryFreeAttributes(pAttrs);
*   	
*   	EnclQueryHighlightObject(pObjs[i]);
*   	EnclQueryDehighlightObject(pObjs[i]);
*		EnclQueryDehighlightAll();
*   	
*		int len;
*		char ntxt[128];
*		char txt[128];
*		char * txtdsc = (char *)EnclQueryGetObjectTXTDSC(pObjs[i], ntxt,
*			 txt, &len);
*		EnclQueryFreeObjectTXTDSC((const char *)txtdsc);
*   	
*		int width;
*		int height;
*		char picname[128];
*		char * data = (char *)EnclQueryGetObjectPicture(pObjs[i], picname, &width, &height);
*		EnclQueryFreeObjectPicture((EnclImgDataPtr)data);
*   	
*		EnclObjectGeoData* pdata = EnclQueryObjectGeoData(pObjs[i]);
*		EnclObjectGeoData::GeoType t = pdata->GetType();
*		if(t == EnclObjectGeoData::GT_POINT)
*		{
*		    unsigned char * p = (unsigned char*)pdata->GetData().singlePtData;
*		}
*		else if(t == EnclObjectGeoData::GT_MULT_POINT)
*		{
*		    //GT_MULT_POINT for sounding: point(x, y) + point(z, 0) -> point(x, y, z)
*		    const double * p = pdata->GetData().multiPtData->GetPolyData();
*		    const int* pcnts = pdata->GetData().multiPtData->GetPointCounts();
*		    int npoints = pcnts[0] / 2;
*		    for(int i = 0; i < npoints; ++ i){
*		        double x = p[i * 4];
*		        double y = p[i * 4 + 1];
*		        double z = p[i * 4 + 2];
*		        double nouse = p[i * 4 + 3];
*		    }
*		}
*		else if(t == EnclObjectGeoData::GT_LINE || t == EnclObjectGeoData::GT_AREA)
*		{
*		    int np  = pdata->GetData().multiPtData->GetNumPolies();
*		    const int* pcnts = pdata->GetData().multiPtData->GetPointCounts();
*		    const double * p = pdata->GetData().multiPtData->GetPolyData();
*		}
*		EnclQueryFreeGeoData(pdata);
*	}
*	
*	EnclFreeObjectIDs(pObjs);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclQueryPickObjects(double lon, double lat,
	int pickRad, int * count);

/*! \brief 获取物标属性
* \param id 物标id
* \param len 返回属性字串长度
* \return 返回属性字串
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API char *EnclQueryGetObjectAttributes(EnclObjectPtr id, int* len);

/*! \brief 释放物标属性字串
* \param data 物标属性字串指针，由 EnclQueryGetObjectAttributes() 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeAttributes(char * data);

/*! \brief 获取物标地理坐标数据
* \param id 物标id
* \return 返回属性地理坐标数据结构指针
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API EnclObjectGeoData * EnclQueryObjectGeoData(EnclObjectPtr id);

/*! \brief 释放物标地理坐标数据
* \param data 物标地理坐标数据结构指针
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeGeoData(EnclObjectGeoData * data);

/*! \brief 获取已安装海图列表
* \param nCnt 返回的海图个数
* \param bForceReload 是否强制刷新，通常为true
* \return 返回海图列表字串指针
*
* @par 示例:
* @code
*	int nCharts = 0;
*	char ** pChartNos = EnclQueryGetSENCList(&nCharts, true);
*	for(int i = 0; i < nCharts; ++ i)
*	    EnclSENCDelete(pChartNos[i]);
*	EnclQueryFreeSENCList(pChartNos, nCharts);
* @endcode
*/
ENCL_API char** EnclQueryGetSENCList(int * nCnt, bool bForceReload);

/*! \brief 释放已安装海图列表
* \param plist 已安装海图列表指针，由 EnclQueryGetSENCList() 产生
* \param nCnt 海图个数，由 EnclQueryGetSENCList() 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryGetSENCList() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeSENCList(char ** plist, int nCnt);

/*! \brief 将指定海图的指定自动更新版本的内容进行高亮
* \param chartName 指定的海图名称（8位字符）
* \param updNo 指定的自动更新版本
* \return 无
*
* @par 示例:
* @code
*	EnclQueryHighlightAutoUpdates("GB5X01SW", 2);
* @endcode
*/
ENCL_API void EnclQueryHighlightAutoUpdates(const char * chartName, unsigned int updNo);

/*! \brief 获取指定海图的自动更新记录
* \param chartName 指定的海图名称（8位字符）
* \param nCnt 返回的自动更新记录个数
* \return 返回自动更新记录指针
*
* @par 示例:
* @code
*	int nCnt;
*	AutoUpdateRecord * pRcds = EnclQueryGetAutoUpdateData("GB5X01SW", &nCnt);
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*	    co << "\r\nclassName: " << pRcds[i].className
*	       << "\r\ndtaSetName: " << pRcds[i].dataSetName
*	       << "\r\neditionNumber: " << pRcds[i].editionNumber
*	       << "\r\ngeoPrimitive: " << pRcds[i].geoPrimitive
*	       << "\r\nissueDate: " << pRcds[i].issueDate
*	       << "\r\ncnt: " << pRcds[i].cnt;
*	
*	    if(pRcds[i].cnt > 0){
*	        for(int j = 0; j < pRcds[i].cnt; j++){
*	            co << "\r\nlon: " << pRcds[i].pos[j*2]
*	               << "\r\nlat: " << pRcds[i].pos[j*2+1];
*	        }
*	    }else{
*	        co << "\r\nlon: "
*	           << "\r\nlat: ";
*	    }
*	
*	    co << "\r\nrecordID: " << pRcds[i].recordID
*	       << "\r\nupdApplicationDate: " << pRcds[i].updApplicationDate
*	       << "\r\nupdateInstruction: " << pRcds[i].updateInstruction
*	       << "\r\nupdateNumber: " << pRcds[i].updateNumber
*	       << "\r\nversionNumber: " << pRcds[i].versionNumber << "\r\n";
*	}
*	
*	EnclQueryFreeAutoUpdateData(pRcds);
* @endcode
*/
ENCL_API AutoUpdateRecord * EnclQueryGetAutoUpdateData(const char * chartName,int *nCnt);

/*! \brief 释放自动更新记录
* \param pRcds 自动更新记录指针，由 EnclQueryGetAutoUpdateData() 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryGetAutoUpdateData() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeAutoUpdateData(AutoUpdateRecord * pRcds);

/*! \brief 将指定的物标进行高亮
* \param id 指定的物标id
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryHighlightObject(EnclObjectPtr id);

/*! \brief 清除所有物标高亮状态,对应于 EnclQueryHighlightObject()
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryDehighlightAll();

/*! \brief 获取指定的物标的文本附件信息
* \param id 指定的物标id
* \param NtxtFile 返回的本地语言文本文件路径
* \param txtFile 返回的文本文件路径
* \param len 返回的文本内容字节数
* \return 返回的文本内容，其实就是本地语言文本文件内容 + 0x1f + 文本文件内容
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API const char *EnclQueryGetObjectTXTDSC(EnclObjectPtr id, char* NtxtFile, char* txtFile, int* len);

/*! \brief 释放物标的文本附件信息
* \param data 文本内容指针，由 EnclQueryGetObjectTXTDSC() 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeObjectTXTDSC(const char * data);

/*! \brief 获取指定的物标的图片附件信息
* \param id 指定的物标id
* \param picName 返回的图片文件路径
* \param width 返回的图片宽度
* \param height 返回的图片高度
* \return 返回的图片内容指针
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API EnclImgDataPtr EnclQueryGetObjectPicture(EnclObjectPtr id, char* picName, int* width, int* height);

/*! \brief 释放物标的图片附件信息
* \param data 图片内容指针，由 EnclQueryGetObjectPicture() 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeObjectPicture(EnclImgDataPtr data);

/*! \brief 查询指定多边形区域内海图物标
* \param[in] lons  多边形顶点的经度坐标数组
* \param[in] lats  多边形顶点的纬度坐标数组
* \param[in] pointCount 多边形顶点个数
* \param[out] objCount  查询结果物标个数
* \return 查询结果物标ID数组 通过调用\ref EnclFreeObjectIDs 释放数组内存资源
*/
ENCL_API EnclObjectPtr* EnclQueryGetObjectInPolygon(double *lons, double *lats, int pointCount, int *objCount);

/*! \brief 查询指定多边形区域内陆地要素
* \param[in] lons  多边形顶点的经度坐标数组
* \param[in] lats  多边形顶点的纬度坐标数组
* \param[in] pointCount 多边形顶点个数
* \param[out] objCount  查询结果物标个数
* \return 查询结果物标ID数组 通过调用\ref EnclFreeObjectIDs 释放数组内存资源
*/
ENCL_API EnclObjectPtr * EnclQueryGetLandObjectsInPolygon(double * lons, double *lats, int npoints, int *nObjects); //EXT

/*! \brief 查询指定多边形区域与指定陆地要素的相交区域（相交计算可产生多个独立多边形区域）
* \param[in] targetObj 目标陆地要素
* \param[in] lons  多边形顶点的经度坐标数组
* \param[in] lats  多边形顶点的纬度坐标数组
* \param[in] pointCount 多边形顶点个数
* \param[out] nArea 相交区域多边形个数 
* \return 多边形数据结构数组。 通过调用\ref EnclQueryFreeGeoDataArray 释放数组内存资源
*/
ENCL_API EnclObjectGeoData * EnclQueryGetLandObjectAreaInsidePolygon(EnclObjectPtr targetObj, double * lons, double *lats, int npoints, int *nAreas); //EXT

/*! \brief 释放 \ref EnclQueryGetLandObjectAreaInsidePolygon 返回的多边形结构数组
* \param[in] geoDataArray 多边形结构数组
*/
ENCL_API void EnclQueryFreeGeoDataArray(EnclObjectGeoData * geoDataArray); //EXT

/*! \brief 查询指定地理位置水深。 若无有效数据则返回99999.0
* \param[in] lons  指定查询位置经度
* \param[in] lats  指定查询位置纬度
* \return 水深 单位（米）
*/
ENCL_API double EnclQueryGetDepthAtPosition(double lon, double lat);

/*! \brief 查询指定地理位置水深。 若无有效数据则返回99999.0
* \param[in] lons  指定查询位置经度
* \param[in] lats  指定查询位置纬度
* \return 水深 单位（米）
*/
ENCL_API double EnclQueryGetLandElevationAtPosition(double lon, double lat);

/*! \brief 查询指定区域内最高点高程。 若无有效数据则返回99999.0
* \param[in] lons  多边形顶点的经度坐标数组
* \param[in] lats  多边形顶点的纬度坐标数组
* \param[in] npoints 多边形顶点个数
* \return 最高点高程 单位（米）
*/
ENCL_API double EnclQueryGetHighestElevationInLandArea(double * lons, double *lats, int npoints);

/*! \brief 查询线段与陆地要素边界交点
* \param[in] lon0  线段起始点经度
* \param[in] lat0  线段起始点纬度
* \param[in] lon1  线段结束点经度
* \param[in] lat2  线段结束点纬度
* \param[out] npts 交点个数
* \return 交点数组 （lon, lat, lon,lat ...）
*/
ENCL_API double * EnclQueryLandAreaCrossPoints(double lat0, double lon0, double lat1, double lon1, int *npts); //EXT

/*! \brief 释放 \ref EnclQueryLandAreaCrossPoints 返回的数组内存资源
* \param[in] pts 交点数组
*/
ENCL_API void EnclQueryFreeCrossPoints(double *pts);	// EXT

/*! \brief 查询线段与等高线要素交点
* \param[in] lon0  线段起始点经度
* \param[in] lat0  线段起始点纬度
* \param[in] lon1  线段结束点经度
* \param[in] lat2  线段结束点纬度
* \param[out] npts 交点个数
* \return 交点数组 （lon, lat, lon,lat ...）
*/
ENCL_API double * EnclQueryLandElevationCrossPoints(double lat0, double lon0, double lat1, double lon1, int *npts);//EXT

/*! \brief 释放 \ref EnclQueryLandAreaCrossPoints or \ref  EnclQueryLandElevationCrossPoints返回的数组内存资源
* \param[in] pts 交点数组
*/
ENCL_API void EnclQueryFreeCrossPoints(double *pts);	// EXT

/*! \brief 将指定的物标清除高亮
* \param id 指定的物标id
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickObjects() 示例
* @endcode
*/
ENCL_API void EnclQueryDehighlightObject(EnclObjectPtr id);

/*! \brief 获取当前视口内的海图列表
* \param cnt 返回海图个数
* \return 返回海图id指针
*
* @par 示例:
* @code
*	int nCnt = 0;
*	EnclObjectPtr* pObjs = EnclQueryDisplayedCharts(&nCnt);
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*		int len;
*		char * pAttrs = EnclQueryGetObjectAttributes(pObjs[i], &len);
*		EnclQueryFreeAttributes(pAttrs);
*	}
*	
*	EnclFreeObjectIDs(pObjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryDisplayedCharts(int * cnt);

/*! \brief 根据海图名字获取海图
* \param chartName 海图名称(8个字符)
* \return 返回海图id
*
* @par 示例:
* @code
*	EnclObjectPtr chart = EnclQueryGetChartObjectByName(m_params[0].c_str());
*	
*	int nCnt;
*	EnclObjectPtr* pobjs = EnclQueryGetAllObjectsInChart(chart, &nCnt);
*	
*	EnclFreeObjectIDs(pobjs);
* @endcode
*/
ENCL_API EnclObjectPtr EnclQueryGetChartObjectByName(const char * chartName);

/*! \brief 获取指定海图的物标列表
* \param chartId 指定海图id
* \param nCnt 返回物标个数
* \return 返回物标id指针
*
* @par 示例:
* @code
*	参考 EnclQueryGetChartObjectByName() 示例
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryGetAllObjectsInChart(EnclObjectPtr chartId, int * nCnt);

/*! \brief 获取当前视口内指定屏幕坐标处的海图列表
* \param pickX 指定屏幕坐标x
* \param pickY 指定屏幕坐标y
* \param cnt 返回海图个数
* \return 返回海图id指针
*
* @par 示例:
* @code
*	int nCnt = 0;
*	EnclObjectPtr* pObjs = EnclQueryDisplayedChartsAtPos(100, 200, &nCnt);
*	EnclFreeObjectIDs(pObjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryDisplayedChartsAtPos(int pickX, int pickY, int * cnt);

/*! \brief 获取当前视口内指定经纬度坐标处的海图列表
* \param lon 指定经度
* \param lat 指定纬度
* \param cnt 返回海图个数
* \return 返回海图id指针
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclObjectPtr* pobjs = EnclQueryDisplayedChartsAtLonLat(61.234, 32.456, &ncnt);
*	EnclFreeObjectIDs(pobjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryDisplayedChartsAtLonLat(double lon, double lat, int * cnt);

/*! \brief 获取指定经纬度坐标处的海图列表（不限定视口内，而是所有已安装的海图）
* \param lon 指定经度
* \param lat 指定纬度
* \param cnt 返回海图个数
* \return 返回海图id指针
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclObjectPtr* pobjs = EnclQueryInstalledChartsAtLonLat(61.234, 32.456, &ncnt);
*	EnclFreeObjectIDs(pobjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryInstalledChartsAtLonLat(double lon, double lat, int * cnt);

/*! \brief 定义警戒区
* \param guardArea 多边形数据结构PolyData指针，用于定义一个区域
* \return 返回警戒区id
*
* @par 示例:
* @code
*	double * pdata = new double[n];
*	for(int i = 0; i < n; ++ i)
*		pdata[i] = toDouble(m_params[i]);
*	
*	n /= 2;
*	PolyData pd(1, &n, pdata);
*	delete [] pdata;
*	
*	int areaid = EnclQueryDefinedGuardArea(&pd);
*	EnclQuerySetGuardAreaVisible(areaid, true);
*	int nCnt;
*	int grpid;
*	DangerObjInfo * pInfos = EnclQueryDangerObjects(areaid, &nCnt, true, &grpid);
*	
*	EnclQueryFreeDangerObjects(pInfos);
* @endcode
*/
ENCL_API int EnclQueryDefinedGuardArea(PolyData * guardArea);

/*! \brief 更新指定警戒区
* \param guardArea 多边形数据结构PolyData指针，用于更新警戒区坐标数据
* \param areaId 指定警戒区id，由 EnclQueryDefinedGuardArea 产生
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	PolyData pd;
*	CalculateGuardArea(&pd);
*	bool upd = EnclQueryUpdateGuardArea(&pd, m_curGuardAreaId);
* @endcode
*/
ENCL_API bool EnclQueryUpdateGuardArea(PolyData * guardArea, int areaId);

/*! \brief 删除指定警戒区
* \param areaId 指定警戒区id，由 EnclQueryDefinedGuardArea 产生
* \return 无
*
* @par 示例:
* @code
*	EnclQueryRemoveGuardArea(m_curGuardAreaId);
* @endcode
*/
ENCL_API void EnclQueryRemoveGuardArea(int areaId);

/*! \brief 设置警戒区内报警类型的状态
* \param type 报警类型
* \param bSet true表示报警，false表示不报警
* \return 无
*
* @par 示例:
* @code
*	EnclQueryConfigChartAlert(ENCL_CAT_NAVI_HAZARDS, true);
*	EnclQueryConfigChartAlert(ENCL_CAT_SAFETY_CONTOUR, false);
* @endcode
*/
ENCL_API void EnclQueryConfigChartAlert(EnclChartAlertType type, bool bSet);

/*! \brief 获取报警类型的状态
* \param type 报警类型
* \return true表示报警，false表示不报警
*
* @par 示例:
* @code
*	bool typestate = EnclQueryGetChartAlert(ENCL_CAT_NAVI_HAZARDS);
* @endcode
*/
ENCL_API bool EnclQueryGetChartAlert(EnclChartAlertType type);

/*! \brief 获取指定警戒区内的危险物标
* \param iAreaID 指定警戒区id
* \param nCnt 返回危险物标的个数
* \param bHighlight true表示高亮危险物标，false表示不高亮
* \param highlightGrpId 此批危险物的分组 id
* \return 危险物数据结构 DangerObjInfo 指针
*
* @par 示例:
* @code
*	参考 EnclQueryDefinedGuardArea() 示例
* @endcode
*/
ENCL_API DangerObjInfo* EnclQueryDangerObjects(int iAreaID, int * nCnt, bool bHighlight, 
	int * highlightGrpId);

/*! \brief 获取指定警戒区内的危险物标
* \param iAreaID 指定警戒区id
* \param nCnt 返回危险物标的个数
* \param bHighlight true表示高亮危险物标，false表示不高亮
* \param highlightGrpId 此批危险物的分组 id
* \return 危险物数据结构 DangerObjInfo 指针
*
* @par 示例:
* @code
*	参考 EnclQueryDefinedGuardArea() 示例
* @endcode
*/

/*! \brief 取消指定分组的危险物的高亮状态
* \param groupId 此批危险物的分组 id
* \return 无
*
* @par 示例:
* @code
*	EnclQueryDehighlightDangerObjects(m_lastHighlightGrpId);
* @endcode
*/
ENCL_API void EnclQueryDehighlightDangerObjects(int groupId);

/*! \brief 取消所有危险物的高亮状态
* \return 无
*
* @par 示例:
* @code
*	EnclQueryDehighlightAllDangerObjects();
* @endcode
*/
ENCL_API void EnclQueryDehighlightAllDangerObjects();

/*! \brief 释放危险物信息
* \param pInfos 危险物数据结构 DangerObjInfo 指针，由 EnclQueryDangerObjects 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryDefinedGuardArea() 示例
* @endcode
*/
ENCL_API void EnclQueryFreeDangerObjects(DangerObjInfo * pInfos);

/*! \brief 设置警戒区是否显示外框线
* \param areaId 指定警戒区id
* \param bVisible true 显示外框线，false 不显示
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryDefinedGuardArea() 示例
* @endcode
*/
ENCL_API void EnclQuerySetGuardAreaVisible(int areaId, bool bVisible);

/*! \brief 获取所有的物标类
* \param nCnt 返回物标类的个数
* \return 返回物标类指针
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclS57ClassInfo* pinfos = EnclQueryS57ClassGetAllClass(&ncnt);
*	for(int i = 0; i < ncnt; i++){
*	    m_classlist.push_back(pinfos[i]);
*	}
*	EnclQueryS57ClassFreeAllClass(pinfos);
* @endcode
*/
ENCL_API EnclS57ClassInfo * EnclQueryS57ClassGetAllClass(int * nCnt);

/*! \brief 释放物标类
* \param pinfos 物标类指针，由 EnclQueryS57ClassGetAllClass 产生。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetAllClass() 示例
* @endcode
*/
ENCL_API void EnclQueryS57ClassFreeAllClass(EnclS57ClassInfo * pinfos);

/*! \brief 获取所有的物标属性信息
* \param nCnt 返回物标属性信息的个数
* \return 返回物标属性信息指针
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclS57ClassAttrInfo* pinfos = EnclQueryS57ClassGetAllAttr(&ncnt);
*	for(int i = 0; i < ncnt; i++){
*		m_attrlist.push_back(pinfos[i]);
*	}
*	EnclQueryS57ClassFreeAllAttr(pinfos);
* @endcode
*/
ENCL_API EnclS57ClassAttrInfo * EnclQueryS57ClassGetAllAttr(int * nCnt);

/*! \brief 释放属性信息
* \param pinfos 物标属性信息指针，由 EnclQueryS57ClassGetAllAttr 产生。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetAllAttr() 示例
* @endcode
*/
ENCL_API void EnclQueryS57ClassFreeAllAttr(EnclS57ClassAttrInfo * pinfos);

/*! \brief 获取指定物标类的属性id列表
* \param classcode 指定的物标类id
* \param attrcodes 返回属性id列表指针
* \param attrnum 属性id的个数
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	EnclS57ClassAttrInfo attrinfo;
*	int *attrcodes;
*	int attrnum;
*	EnclQueryS57ClassGetAttrcodesByClass(m_classlist[m_curclass].classcode, &attrcodes, &attrnum);
*	for(int i = 0; i < attrnum; i++){
*		int code = attrcodes[i];
*		if(EnclQueryS57ClassGetAttrInfo(code, &attrinfo)){
*			m_attrlist.push_back(attrinfo);
*		}
*	}
*	EnclQueryS57ClassFreeAttrcodes(attrcodes);
* @endcode
*/
ENCL_API bool EnclQueryS57ClassGetAttrcodesByClass(int classcode, int **attrcodes, int *attrnum);

/*! \brief 获取指定物标类的强制属性id列表
* \param classcode 指定的物标类id
* \param attrcodes 返回强制属性id列表指针
* \param attrnum 强制属性id的个数
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	int *mandatoryattrcodes;
*	int mandatoryattrnum;
*	EnclQueryS57ClassGetMandatoryAttrcodesByClass(m_classlist[m_curclass].classcode, &mandatoryattrcodes, &mandatoryattrnum);
*	for(int i = 0; i < mandatoryattrnum; i++){
*		int code = mandatoryattrcodes[i];
*		m_mandatoryattrlist.push_back(code);
*	}
*	EnclQueryS57ClassFreeAttrcodes(mandatoryattrcodes);
* @endcode
*/
ENCL_API bool EnclQueryS57ClassGetMandatoryAttrcodesByClass(int classcode, int **attrcodes, int *attrnum);

/*! \brief 释放属性id列表
* \param attrcodes 属性id列表指针，由 EnclQueryS57ClassGetAttrcodesByClass, EnclQueryS57ClassGetMandatoryAttrcodesByClass 产生。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetAttrcodesByClass() 示例
* @endcode
*/
ENCL_API void EnclQueryS57ClassFreeAttrcodes(int *attrcodes);

/*! \brief 获取指定属性id的详细信息
* \param attrcode 指定的属性id
* \param attrinfo 返回属性信息
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetAttrcodesByClass() 示例
* @endcode
*/
ENCL_API bool EnclQueryS57ClassGetAttrInfo(int attrcode, EnclS57ClassAttrInfo *attrinfo);

/*! \brief 获取指定属性id的取值列表（枚举类型）
* \param attrcode 指定的属性id
* \param enums 返回取值（枚举类型）列表指针
* \param enums 返回取值（枚举类型）个数
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	int *enumcodes;
*	int enumnum;
*	EnclQueryS57ClassGetEnumsByAttrcode(attrcode, &enumcodes, &enumnum);
*	for(int i = 0; i < enumnum; i++){
*		EnclQueryS57ClassGetEnumInfo(attrcode, enumcodes[i], &enuminfo);
*		m_enumlist.push_back(enuminfo);
*	}
*	EnclQueryS57ClassFreeEnums(enumcodes);
* @endcode
*/
ENCL_API bool EnclQueryS57ClassGetEnumsByAttrcode(int attrcode, int **enums, int *valnum);

/*! \brief 释放取值列表（枚举类型）
* \param enums 取值列表指针
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetEnumsByAttrcode() 示例
* @endcode
*/
ENCL_API void EnclQueryS57ClassFreeEnums(int *enums);

/*! \brief 获取指定属性id的指定枚举值的详细信息
* \param attrcode 指定的属性id
* \param enumcode 指定的枚举值
* \param enuminfo 返回指定属性id的指定枚举值的详细信息
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	参考 EnclQueryS57ClassGetEnumsByAttrcode() 示例
* @endcode
*/
ENCL_API bool EnclQueryS57ClassGetEnumInfo(int attrcode, int enumcode, EnclS57ClassEnumInfo *enuminfo);
/** @} */

/** @defgroup projection 投影和转换 
* 投影和转换等
* @{
*/

/*! \brief 设置当前视口的投影模式
* \param value 投影模式
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	EnclTransformSetProjection(ENCL_PROJECTION_MERCATOR);
* @endcode
*/
ENCL_API bool EnclTransformSetProjection(EnclProjection value);

/*! \brief 获取当前视口的投影模式
* \return 返回当前视口的投影模式
*
* @par 示例:
* @code
*	EnclProjection pm = EnclTransformGetProjection();
* @endcode
*/
ENCL_API EnclProjection EnclTransformGetProjection();

//nouse
ENCL_API bool EnclTransformToWGS84(double lon1, double lat1, double* lon2, double* lat2);	//TODO	confirm cancel.
ENCL_API bool EnclTransformFromWGS84(double lon1, double lat1, double* lon2, double* lat2);	//TODO	confirm cancel.
// following 2 apis confirm cancel, but still exist in standard, so we return correctly
ENCL_API bool  EnclTransformSetDatum(EnclDatum  value);		//TODO	confirm cancel.
ENCL_API EnclDatum  EnclTransformGetDatum();		//TODO	confirm cancel.

/*! \brief 在指定视口信息下，将屏幕坐标转换为地理坐标
* \param scrnPointX 屏幕坐标x
* \param scrnPointY 屏幕坐标y
* \param lon 返回经度
* \param lat 返回纬度
* \param vp 指定视口信息的指针，默认为空，表示当前视口。注意：该信息可以取用已创建好的视口对象，也可以自行配置。
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	EnclViewPort vp;
*	
*	int scrnX = toInt(m_params[0]);
*	int scrnY = toInt(m_params[1]);
*	vp.width = toInt(m_params[2]);
*	vp.height = toInt(m_params[3]);
*	vp.centerLon = toDouble(m_params[4]);
*	vp.centerLat = toDouble(m_params[5]);
*	vp.viewScale = toDouble(m_params[6]);
*	vp.bearing = toDouble(m_params[7]);
*	vp.projection = ENCL_PROJECTION_MERCATOR;
*	
*	double lat, lon;
*	EnclTransformScrnToGeoByViewport(scrnX, scrnY, & lon, & lat, &vp);
* @endcode
*/
ENCL_API bool EnclTransformScrnToGeoByViewport(int scrnPointX, int scrnPointY,
	double* lon, double* lat, EnclViewPort * vp = 0);
//obsolete
ENCL_API bool EnclTransformScrnToGeo(int scrnPointX, int scrnPointY, double* lon, double* lat, double scale=0.0);
// calculate in current viewport if vp == 0;

/*! \brief 在指定视口信息下，将地理坐标转换为屏幕坐标
* \param lon 待转换点经度
* \param lat 待转换点纬度
* \param scrnPointX 返回屏幕坐标x
* \param scrnPointY 返回屏幕坐标y
* \param vp 指定视口信息的指针，默认为空，表示当前视口。注意：该信息可以取用已创建好的视口对象，也可以自行配置。
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	EnclViewPort vp;
*	
*	double lon = toDouble(m_params[0]);
*	double lat = toDouble(m_params[1]);
*	vp.width = toInt(m_params[2]);
*	vp.height = toInt(m_params[3]);
*	vp.centerLon = toDouble(m_params[4]);
*	vp.centerLat = toDouble(m_params[5]);
*	vp.viewScale = toDouble(m_params[6]);
*	vp.bearing = toDouble(m_params[7]);
*	vp.projection = ENCL_PROJECTION_MERCATOR;
*	
*	int x, y;
*	EnclTransformGeoToScrnByViewport(lon, lat, &x, &y, &vp);
* @endcode
*/
ENCL_API bool EnclTransformGeoToScrnByViewport(double lon, double lat,
	int * scrnPointX, int * scrnPointY, EnclViewPort * vp = 0);
//obsolete
ENCL_API bool EnclTransformGeoToScrn(double lon, double lat, int * scrnPointX, int * scrnPointY, double scale=0.0);
//todo
ENCL_API void EnclTransformProject(double lon, double lat, double* x, double* y);
ENCL_API void EnclTransformReverseProject(double x, double y, double* lon, double* lat);
/** @} */

/** @defgroup Calculations 计算 
* 距离，角度计算等
* @{
*/

/*! \brief 计算起点和终点（地理坐标）的恒向线距离和角度
* \param lon1 起点经度
* \param lat1 起点纬度
* \param lon2 终点经度
* \param lat2 终点纬度
* \param bear 返回的角度
* \param dist 返回的恒向线距离
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double bear, dist;
*	EnclCalculateRhumblineDistanceAndBearing(60.123, 30.456, 61.345, 31.678, &bear, &dist);
* @endcode
*/
ENCL_API bool EnclCalculateRhumblineDistanceAndBearing(double lon1, double lat1,
	double lon2, double lat2, double *bear, double *dist);

/*! \brief 依据起点地理坐标，恒向线距离和角度，计算出终点的地理坐标
* \param lon1 起点经度
* \param lat1 起点纬度
* \param dist 给定的恒向线距离
* \param bear 给定的角度
* \param lon2 返回终点经度
* \param lat2 返回终点纬度
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double lon2, lat2;
*	EnclCalculateRhumbLineTravel(60.123, 30.456, 1852.0, 30.0, &lon2, &lat2);
* @endcode
*/
ENCL_API bool EnclCalculateRhumbLineTravel(double lon1, double lat1,
	double dist, double bear, double * lon2, double *lat2);

/*! \brief 计算起点和终点（地理坐标）的大圆距离和角度
* \param lon1 起点经度
* \param lat1 起点纬度
* \param lon2 终点经度
* \param lat2 终点纬度
* \param bear1 返回的起点切线角度
* \param bear2 返回的终点切线角度
* \param dist 返回的大圆距离
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double bear1, bear2, dist;
*	EnclCalculateGreatCircleDistanceAndBearing(60.123, 30.456, 61.345, 31.678, &bear1, &bear2, &dist);
* @endcode
*/
ENCL_API bool EnclCalculateGreatCircleDistanceAndBearing(double lon1,double lat1,
	double lon2, double lat2, double *bear1, double *bear2, double *dist);

/*! \brief 依据起点地理坐标，大圆距离和切线角度，计算出终点的地理坐标和切线角度
* \param lon1 起点经度
* \param lat1 起点纬度
* \param dist 给定的大圆距离
* \param bear1 给定的起点切线角度
* \param lon2 返回终点经度
* \param lat2 返回终点纬度
* \param bear2 给定的终点切线角度
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double lon2, lat2, bear2;
*	EnclCalculateGreatCircleTravel(60.123, 30.456, 1852.0, 30.0, &lon2, &lat2, &bear2);
* @endcode
*/
ENCL_API bool EnclCalculateGreatCircleTravel(double lon1, double lat1,
	double dist, double bear1, double * lon2, double * lat2, double *bear2);

/*! \brief 计算多点（地理坐标）的大圆距离和
* \param lons 经度列表指针
* \param lats 纬度列表指针
* \param count 坐标点个数
* \param dist 返回的大圆距离和
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double * plons = new double[m_params.size() / 2];
*	double * plats = new double[m_params.size() / 2];
*	for(int i = 0; i < m_params.size(); ++ i)
*	{
*		if(i % 2)
*			plats[(i - 1) / 2] = toDouble(m_params[i]);
*	    else
*			plons[i / 2]  = toDouble(m_params[i]);
*	}
*	
*	double dist;
*	EnclCalculateGreatCirclePolylineDistance(plons, plats, m_params.size() / 2, &dist);
* @endcode
*/
ENCL_API bool EnclCalculateGreatCirclePolylineDistance(double * lons,
	double *lats, int count, double *dist);

/*! \brief 计算多点（地理坐标）的恒向线距离和
* \param lons 经度列表指针
* \param lats 纬度列表指针
* \param count 坐标点个数
* \param dist 返回的恒向线距离和
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double * plons = new double[m_params.size() / 2];
*	double * plats = new double[m_params.size() / 2];
*	for(int i = 0; i < m_params.size(); ++ i)
*	{
*		if(i % 2)
*			plats[(i - 1) / 2] = toDouble(m_params[i]);
*	    else
*			plons[i / 2]  = toDouble(m_params[i]);
*	}
*	
*	double dist;
*	EnclCalculateRhumblinePolylineDistance(plons, plats, m_params.size() / 2, &dist);
* @endcode
*/
ENCL_API bool EnclCalculateRhumblinePolylineDistance(double * lons, double *lats,
	int count, double *dist);

/*! \brief 计算多点（地理坐标）构成区域的面积
* \param lons 经度列表指针
* \param lats 纬度列表指针
* \param count 坐标点个数
* \param area 返回的面积
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double * plons = new double[m_params.size() / 2];
*	double * plats = new double[m_params.size() / 2];
*	for(int i = 0; i < m_params.size(); ++ i)
*	{
*		if(i % 2)
*			plats[(i - 1) / 2] = toDouble(m_params[i]);
*	    else
*			plons[i / 2]  = toDouble(m_params[i]);
*	}
*	
*	double area;
*	EnclCalculateArea(plons, plats, m_params.size() / 2, &area);
* @endcode
*/
ENCL_API bool EnclCalculateArea(double *lons,double *lats,int count,double *area);

/*! \brief 3D空间（带高度）的两点间距离方位计算
* \param[in] lon1  第一点经度坐标
* \param[in] lat1  第一点纬度坐标
* \param[in] height1  第一点高度，单位（米）
* \param[in] lon2  第二点经度坐标
* \param[in] lat2  第二点纬度坐标
* \param[in] height2  第二点高度，单位（米）
* \param[out] bear  第一点相对第二点的方位
* \param[out] dist  两点间空间距离 单位（米）
* \return 查询结果物标ID数组 通过调用\ref EnclFreeObjectIDs 释放数组内存资源
*/
ENCL_API bool EnclCalculate3DDistanceAndBearing(double lon1, double lat1, double height1, double lon2, double lat2, double height2, double * bear, double *dist);

ENCL_API double * EnclCalculateGreatCirclePositions(double lat0, double lon0, double lat1, double lon1, int *npts, double intervalInMeter);
ENCL_API double * EnclCalculateRhumbPositions(double lat0, double lon0, double lat1, double lon1, int *npts, double intervalInMeter);
ENCL_API void EnclCalculateFreePositions(double * pPts);
ENCL_API void EnclCalculateCreatCirclePolygonPerimeterAndArea(double *lats, double *lons,int count,double *perimeter,double *area);
ENCL_API void EnclCalculateRhumbPolygonPerimeterAndArea(double *lats, double *lons,int count,double *perimeter,double *area);
ENCL_API void EnclCalculateCPA(double ownLat,double ownLon, double ownSpeed, double ownCourse, double otherLat, double otherLon, double otherSpeed, double otherCourse, double *ca, double *tcpa, double *cpaLat, double *cpaLon);
ENCL_API double EnclCalculateXTE(double lato, double lono, double lata, double lona, double latb, double lonb);
/** @} */

/** @defgroup manualupdate 手动更新
* 手动物标的新增，删除，
* @{
*/
//ENCL_API EnclObjectPtr EnclManualUpdateAddObject(const ManUpdObjData * info,
//		const ManUpdObjAttrs * attrs, const EnclObjectGeoData * data);

/*! \brief 新增手动更新类物标
* \param obj 手动更新类物标结构 EnclManualUpdateObject 指针
* \return 返回手动更新类物标id
*
* @par 示例:
* @code
*	//1. collect all infos
*	EnclManualUpdateObject obj;
*	memset(&obj, 0, sizeof(obj));
*	
*	//1.1 objname
*	QString objname = m_classlist[m_curclass].acronym;
*	strcpy(obj.name, objname.toStdString().c_str());
*	
*	//1.2 objtype
*	EnclPrimitiveType objtype;
*	QString sobjtype = m_typelist[m_curtype];
*	if(sobjtype.contains("Point")){
*	    objtype = ENCL_P_PRIM;
*	}else if(sobjtype.contains("Line")){
*	    objtype = ENCL_L_PRIM;
*	}else if(sobjtype.contains("Area")){
*	    objtype = ENCL_A_PRIM;
*	}else{
*	    objtype = ENCL_NONE;
*	}
*	obj.type = objtype;
*	
*	//1.3 position
*	int npts = 0;
*	double *pt = 0;
*	QStringList vals = m_attrvals[0].split(",");
*	npts = vals.size() / 3;
*	
*	bool poserr = false;
*	if(obj.type == ENCL_P_PRIM){
*	    if(npts < 1){
*	        poserr = true;
*	    }
*	}else if(obj.type == ENCL_L_PRIM){
*	    if(npts < 2){
*	        poserr = true;
*	    }
*	}else if(obj.type == ENCL_A_PRIM){
*	    if(npts < 3){
*	        poserr = true;
*	    }
*	}
*	
*	if(poserr){
*	    QMessageBox::warning(this, tr("Manual Update"), tr("Two few postion value!"));
*	    return;
*	}
*	
*	int valsnum;
*	if(npts > 0){
*	    if(m_classlist[m_curclass].classcode == 129){
*	        //sounding
*	        pt = new double[npts * 3];
*	        valsnum = 3;
*	    }else{
*	        pt = new double[npts * 2];
*	        valsnum = 2;
*	    }
*	}
*	for(int i = 0; i < npts; i++)
*	{
*	    double lon = QString(vals[i*3]).toDouble();
*	    double lat = QString(vals[i*3+1]).toDouble();
*	    double depth = QString(vals[i*3+2]).toDouble();
*	    pt[i*valsnum] = lon;
*	    pt[i*valsnum+1] = lat;
*	    if(m_classlist[m_curclass].classcode == 129){
*	        pt[i*valsnum+2] = depth;
*	    }
*	}
*	obj.ptCount = npts;
*	obj.coords = pt;
*	
*	//1.4 objattr
*	QString objattr = "";
*	QMap<int, QString>::iterator it = m_attrvals.begin();
*	while(it != m_attrvals.end())
*	{
*	    int attridx = it.key();
*	    QString attrval = it.value();
*	    int attrcode = m_attrlist[attridx].attrcode;
*	    if(attrcode == 0){
*	        //position can't into objattr
*	    }else{
*	        QString acronym = m_attrlist[attridx].acronym;
*	        if(!objattr.isEmpty()){
*	            objattr += QString("!");
*	        }
*	        objattr += acronym + QString(":") + attrval;
*	    }
*	    it++;
*	}
*	char *attr = 0;
*	if(!objattr.isEmpty()){
*	    int size = objattr.size() + 1;
*	    attr = new char[size];
*	    memset(attr, 0, size);
*	    strcpy(attr, objattr.toStdString().c_str());
*	    obj.length = strlen(attr);
*	    obj.s57attrs = attr;
*	}else{
*	    obj.length = 0;
*	    obj.s57attrs = 0;
*	}
*	
*	//1.5 objremark
*	QString objremark = ui->descTextEdit->toPlainText();
*	strncpy(obj.remark, objremark.toStdString().c_str(), sizeof(obj.remark));
*	
*	//1.6 objtime
*	obj.time = 0;
*	
*	//1.7 ownerchart
*	strcpy(obj.ownerChartName, "");
*	double lon = pt[0];
*	double lat = pt[1];
*	int x, y;
*	EnclTransformGeoToScrnByViewport(lon, lat, &x, &y);
*	int ncnt = 0;
*	EnclObjectPtr* pObjs = EnclQueryDisplayedChartsAtPos(x, y, &ncnt);
*	if(pObjs){
*	    int len;
*	    char * pAttrs = EnclQueryGetObjectAttributes(pObjs[0], &len);
*	    QStringList attrlist = QString(pAttrs).split("!");
*	    for(int i = 0; i < attrlist.size(); i++){
*	        QString attr = attrlist.at(i);
*	        QStringList itemlist = attr.split(":");
*	        QString itemtitle = itemlist.at(0);
*	        QString itemval = itemlist.at(1);
*	        if(itemtitle == "chart name"){
*	            strcpy(obj.ownerChartName, itemval.toStdString().c_str());
*	            break;
*	        }
*	    }
*	    EnclQueryFreeAttributes(pAttrs);
*	}
*	EnclFreeObjectIDs(pObjs);
*	
*	//1.8 commit
*	if(m_mode == ManualUpdateEdit::MU_EDIT_NEW){
*	    EnclObjectPtr id = EnclManualUpdateAddObject(&obj);
*	}else{
*	    EnclManualUpdateModifyObject(m_objid, &obj, 1);
*	}
*	
*	if(pt) delete [] pt;
*	if(attr) delete[] attr;
* @endcode
*/
ENCL_API EnclObjectPtr EnclManualUpdateAddObject(EnclManualUpdateObject * obj);

// EnclManualUpdateModifyObject is commented out since we do not know how this
// should be done. We should only provide the capability to modify objects on
// Enc charts? Since modification of manual updated objects can be done with a
// remove and add operation, and we do not need to append both add and delete
// symbols on it. And if objects on enc charts can be modified, what value of
// the object can we modify? the s57 attributes? and manual updated object
// specific attributes?
//ENCL_API void EnclManualUpdateModifyObject(EnclObjectPtr id,
//		const ManUpdObjAttrs *attrs);

/*! \brief 更新物标
* \param id 待修改的物标id（该物标可以是手动更新类物标，也可以是原图上的物标）
* \param obj 手动更新类物标结构 EnclManualUpdateObject 指针
* \param nflag 0表示原图物标，其他表示手动更新类物标
* \return 返回更新过的物标新的id
*
* @par 示例:
* @code
*	参考 EnclManualUpdateAddObject() 示例
* @endcode
*/
ENCL_API EnclObjectPtr EnclManualUpdateModifyObject(EnclObjectPtr id,
		EnclManualUpdateObject * obj, int nflag);

/*! \brief 永久性删除手动更新类物标
* \param id 待删除手动更新类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	for(int i = 0; i < selrows.size(); i++){
*		EnclObjectPtr id = ui->objsList->item(selrows[i], 0)->data(Qt::UserRole).value<EnclObjectPtr>();
*		EnclManualUpdateDeleteObject(id);
*	}
* @endcode
*/
ENCL_API bool EnclManualUpdateDeleteObject(EnclObjectPtr id);

/*! \brief 拾取自动更新类物标
* \param lon 拾取中心点经度
* \param lat 拾取中心点纬度
* \param pickRad 拾取半径（像素）
* \param count 返回拾取的自动更新类物标个数
* \return 返回拾取的自动更新类物标id列表指针
*
* @par 示例:
* @code
*	int nCnt;
*	EnclObjectPtr * pObjs = EnclQueryPickManualUpdateObjects(60.123, 30.456, 20, &nCnt);
*	
*	CommandOutput coAll;
*	coAll << nCnt << " Objects picked.";
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*		EnclManualUpdateObject obj;
*		EnclManualUpdateGetObjectAttrs(pObjs[i], &obj);
*		coAll << "id:" << (void*)&(pObjs[i]);
*		coAll << ";name:" << obj.name;
*		coAll << ";owner chart name:" << obj.ownerChartName;
*		coAll << ";attribute length:" << obj.length;
*		coAll << ";attributes:" << obj.s57attrs;
*		coAll << ";author:" << obj.author;
*		coAll << ";type:" << obj.type;
*		coAll << ";time:" << obj.time;
*		coAll << ";remark:" << obj.remark;
*		coAll << ";status:" << obj.status;
*		coAll << ";point cnt:" << obj.ptCount;
*		for(int i = 0; i < obj.ptCount; ++ i)
*		{
*			coAll << "coordinates:" << obj.coords[i * 2] << "," << obj.coords[i * 2 + 1]<< ";";
*		}
*		EnclManualUpdateFreeObjectAttrs(&obj);
*	}
*	
*	EnclFreeObjectIDs(pObjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryPickManualUpdateObjects(double lon, double lat, int pickRad, int *count);

/*! \brief 获取指定id的的自动更新类物标信息
* \param id 指定的自动更新类物标id
* \param obj 返回的自动更新类物标结构 EnclManualUpdateObject 指针
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	参考 EnclQueryPickManualUpdateObjects() 示例
* @endcode
*/
ENCL_API bool EnclManualUpdateGetObjectAttrs(EnclObjectPtr id,EnclManualUpdateObject * obj);

/*! \brief 释放自动更新类物标信息
* \param obj 返回的自动更新类物标结构 EnclManualUpdateObject 指针，由 EnclManualUpdateGetObjectAttrs 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickManualUpdateObjects() 示例
* @endcode
*/
ENCL_API void EnclManualUpdateFreeObjectAttrs(EnclManualUpdateObject * obj);
// We decide to not provide image presentation of manual update object. It's not
// flexible and we do not know which symbols should be provided to user.
//ENCL_API unsigned char * EnclManualUpdateGetSymbolImage(
//	EnclS57ClassInfo * info, int imgWidth, int imgHeight);

/*! \brief 重新加载刷新自动更新类物标。当自动更新类数据库有改变（譬如从外部被覆盖)可以通过此接口来重新加载刷新
* \return 无
*
* @par 示例:
* @code
*	EnclManualUpdateReload();
* @endcode
*/
ENCL_API void EnclManualUpdateReload(void);

/*! \brief 获取自动更新类物标列表
* \param nObjs 返回自动更新类物标个数
* \return 返回自动更新类物标id列表
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclObjectPtr *pids = EnclManualUpdateGetObjectsList(&ncnt);
*	
*	EnclManualUpdateObject objdata;
*	for(int i = 0; i < ncnt; i++)
*	{
*		EnclManualUpdateGetObjectAttrs(pids[i], &objdata);
*		if(objdata.removed == 1){
*			//the obj is removed
*			EnclManualUpdateFreeObjectAttrs(&objdata);
*			continue;
*		}
*		EnclManualUpdateFreeObjectAttrs(&objdata);
*	}
*	
*	EnclFreeObjectIDs(pids);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclManualUpdateGetObjectsList(int * nObjs);

/*! \brief 删除指定的原图物标
* \param id 指定的原图物标id
* \return 返回自动更新类物标id
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	EnclObjectPtr idr = EnclManualUpdateRemoveEncObject(id);
* @endcode
*/
ENCL_API EnclObjectPtr EnclManualUpdateRemoveEncObject(EnclObjectPtr id);

/*! \brief 移动指定的原图物标
* \param id 指定的原图物标id
* \param d 新的位置坐标
* \param nObjs 返回自动物标的个数。一般0表示未发现该原图物标，2表示移动成功生成了对应的两个自动更新物标
* \return 返回自动更新类物标id指针
*
* @par 示例:
* @code
*	EnclObjectPtr * pids = 0;
*	int nObjs = 0;
*	
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	
*	if(m_params.size() == 3)
*	{
*		EnclObjectGeoData data(toDouble(m_params[1]), toDouble(m_params[2]));
*		pids= EnclManualUpdateMoveEncObject(id,&data, nObjs);
*	}
*	else if(m_params.size() == 4)
*	{
*		double pt25[3];
*		pt25[0] = toDouble(m_params[1]);
*		pt25[1] = toDouble(m_params[2]);
*		pt25[2] = toDouble(m_params[3]);
*		PolyData pd(pt25, pt25[2]);
*		EnclObjectGeoData data(EnclObjectGeoData::GT_25D_POINT, &pd);
*		pids= EnclManualUpdateMoveEncObject(id,&data, nObjs);
*	}
*	else if(m_params.size() > 4)
*	{
*		int len;
*		char * pAttrs = EnclQueryGetObjectAttributes(id, &len);
*		ATTR_PAIRS aps;
*		ParseAttributesString(pAttrs, aps);
*		EnclQueryFreeAttributes(pAttrs);
*		
*		int npts = (m_params.size() - 1) / 2;
*		double * pt = new double[npts * 2];
*		for(int i = 0; i < npts; ++ i)
*		{
*			pt[i * 2] = toDouble(m_params[1 + i * 2]);
*			pt[i * 2 + 1] = toDouble(m_params[1 + i * 2 + 1]);
*		}
*		
*		PolyData pd(1, &npts, pt);
*		
*		std::string val = GetAttrValueByName(aps, "geometric primitive");
*		if(val == "1")
*		{
*			EnclObjectGeoData data(EnclObjectGeoData::GT_LINE, &pd);
*			pids= EnclManualUpdateMoveEncObject(id, &data, nObjs);
*		}
*		else if(val == "2")
*		{
*			EnclObjectGeoData data(EnclObjectGeoData::GT_AREA, &pd);
*			pids= EnclManualUpdateMoveEncObject(id, &data, nObjs);
*		}
*		
*		delete [] pt;
*	}
*	
*	if(pids)
*	{
*		CommandOutput co;
*		co << "deleted: " << (void*) &(pids[0]);
*		co << "created: " << (void*) &(pids[1]);
*		SetOutput(co);
*		AppendToOutput(co.ToFormatedString());
*	}
* @endcode
*/
ENCL_API EnclObjectPtr* EnclManualUpdateMoveEncObject(EnclObjectPtr id, const EnclObjectGeoData * d, int & nObjs);

/*! \brief 临时性删除手动更新类物标（只是打上了删除标记，并未实际删除）
* \param id 待删除手动更新类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	EnclManualUpdateRemoveObject(id);
* @endcode
*/
ENCL_API bool EnclManualUpdateRemoveObject(EnclObjectPtr id);

/*! \brief 恢复临时性删除的手动更新类物标，对应于 EnclManualUpdateRemoveObject
* \param id 待恢复的手动更新类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	EnclManualUpdateRestoreObject(id);
* @endcode
*/
ENCL_API bool EnclManualUpdateRestoreObject(EnclObjectPtr id);
/** @} */

/** @defgroup userchart 用户海图管理 
* userchart 用户海图管理等相关功能
* @{
*/

/*! \brief 新增用户海图类物标
* \param obj 用户海图类物标结构 EnclUserChartObject 指针
* \return 返回用户海图类物标id
*
* @par 示例:
* @code
*	//1. collect all infos
*	EnclUserChartObject obj;
*	memset(&obj, 0, sizeof(obj));
*	
*	//1.1 objname
*	QString objname = m_classlist[m_curclass].acronym;
*	strcpy(obj.name, objname.toStdString().c_str());
*	
*	//1.2 objtype
*	EnclPrimitiveType objtype;
*	QString sobjtype = m_typelist[m_curtype];
*	if(sobjtype.contains("Point")){
*	    objtype = ENCL_P_PRIM;
*	}else if(sobjtype.contains("Line")){
*	    objtype = ENCL_L_PRIM;
*	}else if(sobjtype.contains("Area")){
*	    objtype = ENCL_A_PRIM;
*	}else{
*	    objtype = ENCL_NONE;
*	}
*	obj.type = objtype;
*	
*	//1.3 position
*	int npts = 0;
*	double *pt = 0;
*	QStringList vals = m_attrvals[0].split(",");
*	npts = vals.size() / 3;
*	
*	bool poserr = false;
*	if(obj.type == ENCL_P_PRIM){
*	    if(npts < 1){
*	        poserr = true;
*	    }
*	}else if(obj.type == ENCL_L_PRIM){
*	    if(npts < 2){
*	        poserr = true;
*	    }
*	}else if(obj.type == ENCL_A_PRIM){
*	    if(npts < 3){
*	        poserr = true;
*	    }
*	}
*	
*	if(poserr){
*	    QMessageBox::warning(this, tr("User Chart"), tr("Two few postion value!"));
*	    return;
*	}
*	
*	int valsnum;
*	if(npts > 0){
*	    if(m_classlist[m_curclass].classcode == 129){
*	        //sounding
*	        pt = new double[npts * 3];
*	        valsnum = 3;
*	    }else{
*	        pt = new double[npts * 2];
*	        valsnum = 2;
*	    }
*	}
*	for(int i = 0; i < npts; i++)
*	{
*	    double lon = QString(vals[i*3]).toDouble();
*	    double lat = QString(vals[i*3+1]).toDouble();
*	    double depth = QString(vals[i*3+2]).toDouble();
*	    pt[i*valsnum] = lon;
*	    pt[i*valsnum+1] = lat;
*	    if(m_classlist[m_curclass].classcode == 129){
*	        pt[i*valsnum+2] = depth;
*	    }
*	}
*	obj.ptCount = npts;
*	obj.coords = pt;
*	
*	//1.4 objattr
*	QString objattr = "";
*	QMap<int, QString>::iterator it = m_attrvals.begin();
*	while(it != m_attrvals.end())
*	{
*	    int attridx = it.key();
*	    QString attrval = it.value();
*	    int attrcode = m_attrlist[attridx].attrcode;
*	    if(attrcode == 0){
*	        //position can't into objattr
*	    }else{
*	        QString acronym = m_attrlist[attridx].acronym;
*	        if(!objattr.isEmpty()){
*	            objattr += QString("!");
*	        }
*	        objattr += acronym + QString(":") + attrval;
*	    }
*	    it++;
*	}
*	char *attr = 0;
*	if(!objattr.isEmpty()){
*	    int size = objattr.size() + 1;
*	    attr = new char[size];
*	    memset(attr, 0, size);
*	    strcpy(attr, objattr.toStdString().c_str());
*	    obj.length = strlen(attr);
*	    obj.s57attrs = attr;
*	}else{
*	    obj.length = 0;
*	    obj.s57attrs = 0;
*	}
*	
*	//1.5 objremark
*	QString objremark = ui->descTextEdit->toPlainText();
*	strncpy(obj.remark, objremark.toStdString().c_str(), sizeof(obj.remark));
*	
*	//1.6 objtime
*	obj.time = 0;
*	
*	//1.7 ownerchart
*	strcpy(obj.ownerChartName, "");
*	double lon = pt[0];
*	double lat = pt[1];
*	int x, y;
*	EnclTransformGeoToScrnByViewport(lon, lat, &x, &y);
*	int ncnt = 0;
*	EnclObjectPtr* pObjs = EnclQueryDisplayedChartsAtPos(x, y, &ncnt);
*	if(pObjs){
*	    int len;
*	    char * pAttrs = EnclQueryGetObjectAttributes(pObjs[0], &len);
*	    QStringList attrlist = QString(pAttrs).split("!");
*	    for(int i = 0; i < attrlist.size(); i++){
*	        QString attr = attrlist.at(i);
*	        QStringList itemlist = attr.split(":");
*	        QString itemtitle = itemlist.at(0);
*	        QString itemval = itemlist.at(1);
*	        if(itemtitle == "chart name"){
*	            strcpy(obj.ownerChartName, itemval.toStdString().c_str());
*	            break;
*	        }
*	    }
*	    EnclQueryFreeAttributes(pAttrs);
*	}
*	EnclFreeObjectIDs(pObjs);
*	
*	//1.8 commit
*	if(m_mode == UserChartEdit::UC_EDIT_NEW){
*	    EnclObjectPtr id = EnclUserChartAddObject(&obj);
*	}else{
*	    EnclUserChartModifyObject(m_objid, &obj);
*	}
*	
*	if(pt) delete [] pt;
*	if(attr) delete[] attr;
* @endcode
*/
ENCL_API EnclObjectPtr EnclUserChartAddObject(EnclUserChartObject * obj);

/*! \brief 更新用户海图类物标
* \param id 待修改的用户海图类物标id
* \param obj 用户海图类物标结构 EnclUserChartObject 指针
* \return 返回更新过的用户海图类物标新的id
*
* @par 示例:
* @code
*	参考 EnclUserChartAddObject() 示例
* @endcode
*/
ENCL_API EnclObjectPtr EnclUserChartModifyObject(EnclObjectPtr id,
		EnclUserChartObject * obj);

/*! \brief 永久性删除用户海图类物标
* \param id 待删除用户海图类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	for(int i = 0; i < selrows.size(); i++){
*		EnclObjectPtr id = ui->objsList->item(selrows[i], 0)->data(Qt::UserRole).value<EnclObjectPtr>();
*		EnclUserChartDeleteObject(id);
*	}
* @endcode
*/
ENCL_API bool EnclUserChartDeleteObject(EnclObjectPtr id);

/*! \brief 拾取用户海图类物标
* \param lon 拾取中心点经度
* \param lat 拾取中心点纬度
* \param pickRad 拾取半径（像素）
* \param count 返回拾取的用户海图类物标个数
* \return 返回拾取的用户海图类物标id列表指针
*
* @par 示例:
* @code
*	int ncnt;
*	EnclObjectPtr * pobjs = EnclQueryPickUserChartObjects(m_centerlon, m_centerlat, 20, &ncnt);
*	
*	m_objinfos.clear();
*	m_objattrvals.clear();
*	
*	EnclUserChartObject objdata;
*	for(int i = 0; i < ncnt; i++)
*	{
*		EnclUserChartGetObjectAttrs(pobjs[i], &objdata);
*		if(objdata.removed == 1){
*			EnclUserChartFreeObjectAttrs(&objdata);
*			continue;
*		}
*	
*		ObjInfo objinfo;
*		objinfo.ownerchart = objdata.ownerChartName;
*		objinfo.classname = objdata.name;
*		if(objdata.type == ENCL_P_PRIM){
*			objinfo.geotype = tr("Point");
*		}else if(objdata.type == ENCL_L_PRIM){
*			objinfo.geotype = tr("Line");
*		}else if(objdata.type == ENCL_A_PRIM){
*			objinfo.geotype = tr("Area");
*		}else{
*			objinfo.geotype = tr("Unknown");
*		}
*		objinfo.poslon = QString::number(objdata.coords[0]);
*		objinfo.poslat = QString::number(objdata.coords[1]);
*		objinfo.description = objdata.remark;
*	
*		char attrs[1024];
*		memset(attrs, 0, 1024);
*		strncpy(attrs, objdata.s57attrs, objdata.length);
*		QString qsattrs = attrs;
*		QStringList qslattrs = qsattrs.split("!");
*		QMap<QString, QString> attrvals;
*		attrvals.clear();
*		for(int j = 0; j < qslattrs.size(); j++){
*			QString attr = qslattrs.at(j);
*			if(attr.isEmpty()){
*				continue;
*			}
*			QString acronym = attr.mid(0, 6);
*			QString value = attr.mid(6);
*			int attrcode = 0;
*			QString edesc = "";
*			QString unit = "";
*			EnclS57ClassAttrType attrtype = ENCL_SAT_FREE_TEXT;
*			QString resolution = "";
*			for(int k = 0; k < m_attrlist.size(); k++){
*				if(strncmp(m_attrlist[k].acronym, acronym.toStdString().c_str(), 6) == 0){
*					attrcode = m_attrlist[k].attrcode;
*					edesc = m_attrlist[k].e_desc;
*					unit = m_attrlist[k].unit;
*					attrtype = m_attrlist[k].type;
*					resolution = m_attrlist[k].resolution;
*					break;
*				}
*			}
*			if(attrcode > 0){
*				if(attrtype == ENCL_SAT_ENUM){
*					int ival = value.toInt();
*					EnclS57ClassEnumInfo enuminfo;
*					memset(&enuminfo, 0, sizeof(EnclS57ClassEnumInfo));
*					EnclQueryS57ClassGetEnumInfo(attrcode, ival, &enuminfo);
*					value = enuminfo.e_desc;
*				}else if((attrtype == ENCL_SAT_FLOAT) && !resolution.isEmpty()){
*					double dval = value.toDouble();
*					int nbit = resolution.size() - 2;
*					value = QString::number(dval, 10, nbit);
*				}
*	
*				if(unit.isEmpty()){
*					attrvals[edesc] = value;
*				}else{
*					attrvals[edesc] = value + QString(" ") + unit;
*				}
*			}
*		}
*		m_objinfos.push_back(objinfo);
*		m_objattrvals.push_back(attrvals);
*		EnclUserChartFreeObjectAttrs(&objdata);
*	}
*	
*	EnclFreeObjectIDs(pobjs);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclQueryPickUserChartObjects(double  lon,double  lat,int pickrad,int  *count);

/*! \brief 获取指定id的的用户海图类物标信息
* \param id 指定的用户海图类物标id
* \param obj 返回的用户海图类物标结构 EnclUserChartObject 指针
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	参考 EnclQueryPickUserChartObjects() 示例
* @endcode
*/
ENCL_API bool EnclUserChartGetObjectAttrs(EnclObjectPtr id,EnclUserChartObject * obj);

/*! \brief 释放用户海图类物标信息
* \param obj 返回的用户海图类物标结构 EnclUserChartObject 指针，由 EnclUserChartGetObjectAttrs 产生
* \return 无
*
* @par 示例:
* @code
*	参考 EnclQueryPickUserChartObjects() 示例
* @endcode
*/
ENCL_API void EnclUserChartFreeObjectAttrs(EnclUserChartObject * obj);

/*! \brief 重新加载刷新用户海图类物标。当用户海图类数据库有改变（譬如从外部被覆盖)可以通过此接口来重新加载刷新
* \return 无
*
* @par 示例:
* @code
*	EnclUserChartReload();
* @endcode
*/
ENCL_API void EnclUserChartReload(void);

/*! \brief 获取用户海图类物标列表
* \param nObjs 返回用户海图类物标个数
* \return 返回用户海图类物标id列表
*
* @par 示例:
* @code
*	int ncnt = 0;
*	EnclObjectPtr *pids = EnclUserChartGetObjectsList(&ncnt);
*	
*	EnclUserChartObject objdata;
*	for(int i = 0; i < ncnt; i++)
*	{
*		EnclUserChartGetObjectAttrs(pids[i], &objdata);
*	
*		if(objdata.removed == 1){
*			//the obj is removed
*			EnclUserChartFreeObjectAttrs(&objdata);
*			continue;
*		}
*	
*		//name
*		objname = objdata.name;
*	
*		//type
*		if(objdata.type == ENCL_P_PRIM){
*			objtype = tr("Point");
*		}else if(objdata.type == ENCL_L_PRIM){
*			objtype = tr("Line");
*		}else if(objdata.type == ENCL_A_PRIM){
*			objtype = tr("Area");
*		}else{
*			objtype = "";
*		}
*	
*		//display inputid
*		char attrs[1024];
*		memset(attrs, 0, 1024);
*		strncpy(attrs, objdata.s57attrs, objdata.length);
*		QString qsattrs = attrs;
*		QStringList qslattrs = qsattrs.split("!");
*		objinputid = QString("--------");
*		for(int j = 0; j < qslattrs.size(); j++){
*			if(QString(qslattrs[j]).contains("inptid")){
*				objinputid = QString(qslattrs[j]).mid(6);
*				break;
*			}
*		}
*	
*		itemobjname = new QTableWidgetItem(QString(objname));
*		itemobjtype = new QTableWidgetItem(QString(objtype));
*		itemobjinputid = new QTableWidgetItem(QString(objinputid));
*		itemobjname->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
*		itemobjtype->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
*		itemobjinputid->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
*	
*		//save id to the item data
*		itemobjname->setData(Qt::UserRole, QVariant::fromValue<EnclObjectPtr>(pids[i]));
*	
*		int currow = ui->objsList->rowCount();
*		ui->objsList->insertRow(currow);
*		ui->objsList->setItem(currow, 0, itemobjname);
*		ui->objsList->setItem(currow, 1, itemobjtype);
*		ui->objsList->setItem(currow, 2, itemobjinputid);
*	
*		EnclUserChartFreeObjectAttrs(&objdata);
*	}
*	
*	EnclFreeObjectIDs(pids);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclUserChartGetObjectsList(int * nObjs);

/*! \brief 临时性删除用户海图类物标（只是打上了删除标记，并未实际删除）
* \param id 待删除用户海图类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	EnclUserChartRemoveObject(id);
* @endcode
*/
ENCL_API bool EnclUserChartRemoveObject(EnclObjectPtr id);

/*! \brief 恢复临时性删除的用户海图类物标，对应于 EnclUserChartRemoveObject
* \param id 待恢复的用户海图类物标id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	EnclUserChartRestoreObject(id);
* @endcode
*/
ENCL_API bool EnclUserChartRestoreObject(EnclObjectPtr id);
/** @} */

/** @defgroup route 航线管理 
* route 航线管理等相关功能
* @{
*/

/*! \brief 创建航线
* \param routeName 航线名称
* \return 返回航线 id
*
* @par 示例:
* @code
*	EnclObjectPtr id = EnclRouteCreateRoute(m_params[0].c_str());
*	
*	char wptName[32];
*	for (int i= 0; i < m_params.size()/ 2 - 1; ++ i)
*	{
*		WayPointObjData d;
*		d.m_wpLon = toDouble(m_params[i * 2 + 1]);
*		d.m_wpLat = toDouble(m_params[i * 2 + 2]);
*		
*		sprintf(wptName, "WPT_%d", i);
*		strncpy(d.m_name, wptName, 32);
*		EnclRouteAppendWayPoint(d, id);
*	}
*	
*	EnclRouteSetRouteVisible(id, true);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteCreateRoute(const char* routeName);

/*! \brief 保存指定航线
* \param routeId 指定航线id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	
*	bool succ =  EnclRouteSaveRoute(idRt);
* @endcode
*/
ENCL_API bool EnclRouteSaveRoute(EnclObjectPtr routeId);

/*! \brief 查询指定航线是否已保存
* \param routeId 指定航线id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	if(!m_curRouteId.IsNull() && !EnclRouteIsRouteSaved(m_curRouteId))
*	{
*		QMessageBox::warning(this, tr("RoutePlan"), tr("Please save current route!"));
*		return;
*	}
* @endcode
*/
ENCL_API bool EnclRouteIsRouteSaved(EnclObjectPtr routeId);

/*! \brief 重新加载刷新所有航线。当航线数据库有改变（譬如从外部被覆盖)可以通过此接口来重新加载刷新
* \return 无
*
* @par 示例:
* @code
*	EnclRouteReloadAllRoutes();
* @endcode
*/
ENCL_API void EnclRouteReloadAllRoutes();

/*! \brief 在指定航线中插入节点
* \param wpData 节点数据
* \param routeId 指定航线id
* \param pos 插入的位置，-1表示插入到最后，0表示插入到最前
* \return 返回新添加的航线节点id
*
* @par 示例:
* @code
*	WayPointObjData newData;
*	double lon, lat;
*	EnclViewGetCenter(&lon, &lat);
*	
*	strncpy(newData.m_name, wptName.toStdString().c_str(), MAX_LEN_WP_NAME);
*	newData.m_wpLon = lon;
*	newData.m_wpLat = lat;
*	newData.m_leglineType = LT_RHUMB;
*	newData.m_radius = vm["DefaultTurnRadius"].toDouble();
*	newData.m_xtd = vm["DefaultXtd"].toDouble();
*	newData.m_speed = vm["DefaultPlanSpeed"].toDouble();
*	
*	EnclRouteInsertWayPoint(newData, routeId, -1);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteInsertWayPoint( WayPointObjData & wpData, EnclObjectPtr routeId, int pos );

/*! \brief 在指定航线中追加节点
* \param wpData 节点数据
* \param routeId 指定航线id
* \return 返回新追加的航线节点id
*
* @par 示例:
* @code
*	WayPointObjData newData;
*	double lon, lat;
*	EnclViewGetCenter(&lon, &lat);
*	
*	strncpy(newData.m_name, wptName.toStdString().c_str(), MAX_LEN_WP_NAME);
*	newData.m_wpLon = lon;
*	newData.m_wpLat = lat;
*	newData.m_leglineType = LT_RHUMB;
*	newData.m_radius = vm["DefaultTurnRadius"].toDouble();
*	newData.m_xtd = vm["DefaultXtd"].toDouble();
*	newData.m_speed = vm["DefaultPlanSpeed"].toDouble();
*	
*	EnclRouteAppendWayPoint(newData, routeId);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteAppendWayPoint( WayPointObjData & wpData, EnclObjectPtr routeId );

/*! \brief 在指定航线中删除指定节点
* \param routeId 指定航线id
* \param wptId 指定节点
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	bool ret = EnclRouteDeleteWayPoint(m_curRoute, m_curWaypoint);
* @endcode
*/
ENCL_API bool EnclRouteDeleteWayPoint( EnclObjectPtr routeId, EnclObjectPtr wptId );

/*! \brief 删除指定航线
* \param routeId 指定航线id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	bool ret = EnclRouteDeleteRoute(id);
* @endcode
*/
ENCL_API bool EnclRouteDeleteRoute(EnclObjectPtr routeId );

/*! \brief 依据航线名称获取所有同名航线
* \param routeName 航线名称
* \param nRts 返回同名航线的个数
* \return 返回同名航线id列表指针
*
* @par 示例:
* @code
*	int nRts;
*	EnclObjectPtr *p = EnclRouteGetRoutesByName(rrtinfo.name.toUtf8().data(), &nRts);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclRouteGetRoutesByName(const char* routeName, int *nRts);

/*! \brief 反转指定航线的节点顺序
* \param routeId 指定航线id
* \param createNewRoute true表示反转后生成新的航线，false表示直接反转指定航线并不生成新航线
* \return 返回反转后的航线id
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	EnclRouteReverseRoute(idRt, true);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteReverseRoute(EnclObjectPtr routeId, bool createNewRoute);

/*! \brief 获取指定航线的节点列表
* \param routeId 指定航线id
* \param nWpts 返回节点的个数
* \return 返回节点id列表指针
*
* @par 示例:
* @code
*	EnclObjectPtr id;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(id);
*	int itype = toInt(m_params[1]);
*	LegType type = LT_RHUMB;
*	switch (itype) {
*	case 0:
*		type = LT_RHUMB;
*		break;
*	case 1:
*		type = LT_GREATCIRCLE;
*		break;
*	default:
*		break;
*	}
*	
*	int npts = 0;
*	EnclObjectPtr * pts = EnclRouteGetAllWayPoints(id, npts);
*	
*	for(int i = 0; i < npts; ++ i)
*	{
*		WayPointObjData d;
*		EnclRouteGetWayPointData(id, pts[i], d);
*		d.m_leglineType = type;
*		EnclRouteSetWayPointData(id, pts[i], d);
*	}
*	
*	EnclFreeObjectIDs(pts);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclRouteGetAllWayPoints(EnclObjectPtr routeId, int& nWpts);

/*! \brief 设置更新指定航线的数据信息
* \param routeId 指定航线id
* \param rtData 航线数据信息
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	
*	RouteObjData rtData;
*	EnclRouteGetRouteData(idRt, rtData);
*	strncpy(rtData.name, m_params[1].c_str(), MAX_LEN_RT_NAME);
*	rtData.defaultArrivalRadius = toDouble(m_params[2]);
*	strncpy(rtData.usrMark, m_params[3].c_str(), MAX_LEN_ROUTE_USER_REMARK);
*	
*	EnclRouteSetRouteData(idRt, rtData);
* @endcode
*/
ENCL_API bool EnclRouteSetRouteData(EnclObjectPtr routeId, const RouteObjData & rtData);

/*! \brief 获取指定航线的数据信息
* \param routeId 指定航线id
* \param rtData 返回航线数据信息
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	int nCnt = 0;
*	EnclObjectPtr * pids = EnclRouteGetAllRoutes(nCnt);
*	CommandOutput co;
*	co << nCnt << " routes:\r\n";
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*		RouteObjData dr;
*		EnclRouteGetRouteData(pids[i], dr);
*		
*		co << dr.name << "\r\n";
*	}
*	
*	EnclFreeObjectIDs(pids);
* @endcode
*/
ENCL_API bool EnclRouteGetRouteData(EnclObjectPtr routeId, RouteObjData & rtData);

/*! \brief 设置更新指定航线的指定节点数据信息
* \param routeId 指定航线id
* \param wpId 指定节点id
* \param wpData 节点数据信息
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	参考 EnclRouteGetAllWayPoints() 示例
* @endcode
*/
ENCL_API bool EnclRouteSetWayPointData(EnclObjectPtr routeId, EnclObjectPtr wpId, const WayPointObjData & wpData);

/*! \brief 获取更新指定航线的指定节点数据信息
* \param routeId 指定航线id
* \param wpId 指定节点id
* \param wpData 返回节点数据信息
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	参考 EnclRouteGetAllWayPoints() 示例
* @endcode
*/
ENCL_API bool EnclRouteGetWayPointData(EnclObjectPtr routeId, EnclObjectPtr wpId, WayPointObjData & wpData);

/*! \brief 获取所有航线列表
* \param nRts 返回航线个数
* \return 返回航线id列表指针
*
* @par 示例:
* @code
*	int nCnt = 0;
*	EnclObjectPtr * pids = EnclRouteGetAllRoutes(nCnt);
*	CommandOutput co;
*	co << nCnt << " routes:\r\n";
*	
*	for(int i = 0; i < nCnt; ++ i)
*	{
*		RouteObjData dr;
*		EnclRouteGetRouteData(pids[i], dr);
*		co << dr.name << "\r\n";
*	}
*	
*	EnclFreeObjectIDs(pids);
* @endcode
*/
ENCL_API EnclObjectPtr* EnclRouteGetAllRoutes(int & nRts);

/*! \brief 使指定航线出于检测状态（显示检测区域边框线）
* \param routeId 指定航线id
* \param distMeter 检测距离（边框线到航线的垂直距离，米）
* \return 返回检测区域的id, 以便联合 EnclQueryDangerObjects 进行危险物检测
*
* @par 示例:
* @code
*	m_curRouteAreaid = EnclRouteCheckRoute(m_curRouteId, wptd.m_xtd);
*	int nCnt = 0;
*	int grpId = -1;
*	DangerObjInfo* pInfos = EnclQueryDangerObjects(m_curRouteAreaid, &nCnt, true, &grpId);
* @endcode
*/
ENCL_API int EnclRouteCheckRoute(EnclObjectPtr routeId, double distMeter);

/*! \brief 获取指定航线的指定节点的序号
* \param routeId 指定航线id
* \param wptId 指定节点id
* \return 返回节点序号
*
* @par 示例:
* @code
*	EnclObjectPtr wp = EnclRouteCalculateNextWayPoint(rtId, lon, lat, speed, course);
*	if(wp)
*	{
*		int index = EnclRouteGetWayPntIndexByID(rtId, wp);
*		nextWayPointId = QString::number(index);
*	}
* @endcode
*/
ENCL_API int EnclRouteGetWayPntIndexByID(EnclObjectPtr routeId, EnclObjectPtr wptId);

/*! \brief 获取指定航线的指定序号的节点
* \param routeId 指定航线id
* \param index 指定节点序号
* \return 返回节点id
*
* @par 示例:
* @code
*	EnclObjectPtr wpt1Id = EnclRouteGetWayPntIDbyIndex(m_curRouteId, 1);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetWayPntIDbyIndex (EnclObjectPtr routeId, int index);

/*! \brief 在给定位置给定半径的范围内，获取航线节点
* \param lon 给定的经度
* \param lat 给定的纬度
* \param radius 给定的半径（米）
* \param prouteId 返回航线id
* \return 返回节点id
*
* @par 示例:
* @code
*	EnclObjectPtr rtid;
*	EnclObjectPtr wptid = EnclRouteGetWayPntAtLocation(60.123, 31.456, 100, rtid);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetWayPntAtLocation (double lon, double lat, double radius, EnclObjectPtr * prouteId/*out*/);

/*! \brief 获取指定航线的指定节点的下一个节点
* \param routeId 指定航线id
* \param wptId 指定节点id
* \return 返回节点id
*
* @par 示例:
* @code
*	EnclObjectPtr nextWaypoint = EnclRouteGetNextWapPnt(m_curRoute, m_curWaypoint);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetNextWapPnt(EnclObjectPtr routeId, EnclObjectPtr wptId);

/*! \brief 获取指定航线的指定节点的上一个节点
* \param routeId 指定航线id
* \param wptId 指定节点id
* \return 返回节点id
*
* @par 示例:
* @code
*	EnclObjectPtr prevWaypoint = EnclRouteGetPrevWapPnt(m_curRoute, m_curWaypoint);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetPrevWapPnt(EnclObjectPtr routeId, EnclObjectPtr wptId);

/*! \brief 获取指定航线的指定名称的节点
* \param routeId 指定航线id
* \param wptName 指定名称
* \param nWpts 名称符合的节点个数
* \return 返回名称符合节点列表指针
*
* @par 示例:
* @code
*	int npts = 0;
*	EnclObjectPtr * pts = EnclRouteGetWayPointsByName(m_curRoute, npts);
*	EnclFreeObjectIDs(pts);
* @endcode
*/
ENCL_API EnclObjectPtr * EnclRouteGetWayPointsByName(EnclObjectPtr routeId, const char* wptName , int & nWpts);

/*! \brief 获取指定航线的活动节点
* \param routeId 指定航线id
* \return 返回活动节点id
*
* @par 示例:
* @code
*	EnclObjectPtr rtId = EnclRouteGetActiveRoute();
*	if(rtId.IsNull())
*		return;
*	EnclObjectPtr curWptId = EnclRouteGetActiveWayPnt(rtId);
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetActiveWayPnt(EnclObjectPtr routeId);

/*! \brief 将指定航线的指定节点设置为活动节点
* \param routeId 指定航线id
* \param wptId 指定节点id
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr idRt, idWpt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	idWpt = EnclRouteGetWayPntIDbyIndex(idRt, toInt(m_params[1]));
*	EnclRouteSetActiveWayPnt(idRt, idWpt);
* @endcode
*/
ENCL_API bool EnclRouteSetActiveWayPnt(EnclObjectPtr routeId,EnclObjectPtr wptId);

/*! \brief 将指定航线设置为活动/非活动航线
* \param routeId 指定航线id
* \param bActivate true表示活动状态，false表示非活动状态
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	EnclObjectPtr id = EnclRouteGetActiveRoute();
*	if(id.IsNull())
*	    return;
*	
*	EnclRouteActivateRoute(id, false);
* @endcode
*/
ENCL_API bool EnclRouteActivateRoute(EnclObjectPtr routeId, bool bActivate);

/*! \brief 获取活动航线
* \return 返回活动航线id
*
* @par 示例:
* @code
*	参考 EnclRouteActivateRoute() 示例
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteGetActiveRoute(void);

/*! \brief 将指定航线设置为可见/隐藏状态
* \param routeId 指定航线id
* \param visible true表示可见状态，false表示隐藏状态
* \return 无
*
* @par 示例:
* @code
*	EnclObjectPtr id = EnclRouteCreateRoute(name.toStdString().c_str());
*	if(id.IsNull())
*	{
*	    QMessageBox::warning(this, tr("RoutePlan"), tr("Route name must be unique."));
*	    return;
*	}
*	else
*	{
*	   EnclRouteSetRouteVisible(id, true);
*	   SetCurentRoute(id);
*	}
*	* @endcode
*/
ENCL_API void EnclRouteSetRouteVisible(EnclObjectPtr routeId, bool visible);

/*! \brief 获取指定航线的状态（可见/隐藏）
* \param routeId 指定航线id
* \return 返回true表示可见状态，false表示隐藏状态
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	
*	bool ret = EnclRouteGetRouteVisible(idRt);
* @endcode
*/
ENCL_API bool EnclRouteGetRouteVisible(EnclObjectPtr routeId);

/*! \brief 设置指定航线的转向提前量（米）
* \param routeId 指定航线id
* \param distInMeter 提前量（米）
* \return 无
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	
*	SetRouteAdvancedDistance(idR, 100.0);
*	ShowRouteWheelOverLine(idR, true);
* @endcode
*/
ENCL_API void EnclSetRouteAdvancedDistance(EnclObjectPtr routeId, double distInMeter);

/*! \brief 显示指定航线的转向提前量标志
* \param routeId 指定航线id
* \param bShow true显示提前量标志，false不显示
* \return 无
*
* @par 示例:
* @code
*	参考 SetRouteAdvancedDistance() 示例
* @endcode
*/
ENCL_API void EnclShowRouteWheelOverLine(EnclObjectPtr routeId, bool bShow);
// Calculate the distance to go before arriving at specified waypoint on route.
// The arrive position is returned if required.
ENCL_API double EnclRouteCalculateDTGBeforeArriveWaypoint(EnclObjectPtr rtId,
	EnclObjectPtr wptId, double lonShip, double latShip, double course, double *arriveLon, double * arriveLat);

/*! \brief 依据本船位置航向速度计算出指定航上的下一个节点，一般用于本船监控
* \param rtId 指定航线id
* \param lon 本船位置经度
* \param lat 本船位置纬度
* \param speed 本船速度
* \param course 本船航向
* \return 返回计算出的节点id, 0表示无结果
*
* @par 示例:
* @code
*	double lon, lat;
*	EnclOwnshipGetPosition(&lon, &lat);
*	double speed = EnclOwnshipGetSOG();
*	double course = EnclOwnshipGetCOG();
*	EnclObjectPtr wp = EnclRouteCalculateNextWayPoint(rtId, lon, lat, speed, course);
*	if(wp)
*	{
*		int index = EnclRouteGetWayPntIndexByID(rtId, wp);
*		nextWayPointId = QString::number(index);
*	}
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteCalculateNextWayPoint(EnclObjectPtr rtId, double lon, double lat, double speed, double course);

/*! \brief 检测指定位置在指定航线上是否到达了某个节点
* \param rtId 指定航线id
* \param lon 指定位置经度
* \param lat 指定位置纬度
* \return 返回节点id, 0表示无结果，非0表示到达了该节点
*
* @par 示例:
* @code
*	EnclObjectPtr idRt;
*	CmdRegisterMgr::m_regs[CmdRegister::stringToRegIdx(m_params[0])].Read(idRt);
*	
*	EnclObjectPtr idWpt = EnclRouteAnyWaypointArrived(idRt, 60.123, 30.456);
*	
*	CommandOutput co;
*	if(idWpt != 0)
*	{
*		WayPointObjData d;
*		EnclRouteGetWayPointData(idRt, idWpt, d);
*		co << "Arrived at point: " << d.m_name;
*	}
*	else
*	{
*		co << "No waypoint arrived!";
*	}
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteAnyWaypointArrived(EnclObjectPtr rtId, double lon, double lat);

/*! \brief 检测指定位置在指定航线上是否到达了指定节点
* \param rtId 指定航线id
* \param wptId 指定节点id
* \param lon 指定位置经度
* \param lat 指定位置纬度
* \return true表示到达了该节点，false表示未到达
*
* @par 示例:
* @code
*	EnclObjectPtr rtId = EnclRouteGetActiveRoute();
*	EnclObjectPtr wptId = EnclRouteGetActiveWayPnt(rtId);
*	if(EnclRouteIsWaypointArrived(rtId, wptId, 30.456, 60.123))
*	{
*	}
* @endcode
*/
ENCL_API bool EnclRouteIsWaypointArrived(EnclObjectPtr rtId, EnclObjectPtr wptId, double lat, double lon);

/*! \brief 将指定航线信息转换为xml格式内容输出
* \param rtId 指定航线id
* \param len 返回xml格式内容长度（字节数）
* \return 返回xml格式内容指针
*
* @par 示例:
* @code
*	int len;
*	char * pxml = EnclRouteExportRouteToXml(id, &len);
*	
*	qDebug() << "TransferRoute:" << type << name << "len=" << len;
*	
*	RouteInfo route;
*	route.name = name;
*	route.type = type;
*	route.time = QDateTime::currentDateTime();
*	route.data = QByteArray(pxml);
*	route.routeid = id;
* @endcode
*/
ENCL_API char* EnclRouteExportRouteToXml(EnclObjectPtr rtId, int *len);

/*! \brief 将xml格式内容转换为航线信息并生成航线
* \param xml xml格式内容指针
* \return 返回新生成航线id
*
* @par 示例:
* @code
*	QString fileName = QFileDialog::getOpenFileName(this, tr("Import Route"), "", tr("RTZ File(*.rtz);;All Files (*)"));
*	
*	if(fileName.length() == 0)
*	    return;
*	
*	QFile file(fileName);
*	if(file.open(QIODevice::ReadOnly))
*	{
*		QByteArray data = file.readAll();
*		EnclObjectPtr id = EnclRouteImportRouteFromXml(data.data());
*		if(id.IsNull())
*		{
*			QMessageBox::warning(this, tr("Import Route"), tr("Failed to import route."));
*		}
*		else
*		{
*			LoadRoutes();
*			QMessageBox::information(this, tr("Import Route"), tr("Route sucessfully imported."));
*		}
*		
*		file.close();
*	}
* @endcode
*/
ENCL_API EnclObjectPtr EnclRouteImportRouteFromXml(char * xml);

//obsolete
ENCL_API EnclObjectPtr EnclManualUpdateAddRoute(EnclRoute* obj);
ENCL_API bool EnclManualUpdateDeleteRoute(EnclObjectPtr id);
/** @} */

/** @defgroup nmea NMEA解析 
* nmea NMEA解析等相关功能
* @{
*/
ENCL_API bool EnclNMEARead(SensorProtocol protocol, const char* msg, RcNMEAData& nmea);
ENCL_API char* EnclNMEAWrite(const RcNMEAData& nmea);
ENCL_API void EnclNMEAFreeSentences(char * sentences);
/** @} */

/** @defgroup ownship 本船管理 
* ownship 本船管理等相关功能
* @{
*/

/*! \brief 创建本船
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipCreateOwnship();
* @endcode
*/
ENCL_API void EnclOwnshipCreateOwnship();

/*! \brief 删除本船
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipDeleteOwnship();
* @endcode
*/
ENCL_API void EnclOwnshipDeleteOwnship();

/*! \brief 设置本船位置
* \param lon 本船位置经度
* \param lat 本船位置纬度
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetPosition(60.123, 30.456);
* @endcode
*/
ENCL_API void EnclOwnshipSetPosition(double lon, double lat);

/*! \brief 获取本船位置
* \param lon 返回本船位置经度
* \param lat 返回本船位置纬度
* \return 无
*
* @par 示例:
* @code
*	double lat, lon;
*	EnclOwnshipGetPosition(&lon, &lat);
* @endcode
*/
ENCL_API void EnclOwnshipGetPosition(double *lon, double *lat);

/*! \brief 设置本船首向（度）
* \param heading 本船首向（度）
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetHeading(45.0);
* @endcode
*/
ENCL_API void EnclOwnshipSetHeading(double heading);

/*! \brief 获取本船首向（度）
* \return 返回本船首向（度）
*
* @par 示例:
* @code
*	double heading = EnclOwnshipGetHeading();
* @endcode
*/
ENCL_API double EnclOwnshipGetHeading();

/*! \brief 设置本船尺寸
* \param sd 本船尺寸数据
* \return 无
*
* @par 示例:
* @code
*	ShipDimension sd;
*	sd.length = len;
*	sd.breadth = width;
*	EnclOwnshipSetDimension(sd);
* @endcode
*/
ENCL_API void EnclOwnshipSetDimension(ShipDimension sd);

/*! \brief 获取本船尺寸
* \return 返回本船尺寸数据
*
* @par 示例:
* @code
*	ShipDimension sd;
*	sd = EnclOwnshipGetDimension();
* @endcode
*/
ENCL_API ShipDimension EnclOwnshipGetDimension();

/*! \brief 设置本船CCRP（公共参考点）
* \param dx 相对于活动状态的GPS设备所在位置的横向偏移量（米），左负右正
* \param dy 相对于活动状态的GPS设备所在位置的纵向偏移量（米），下负上正
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetCCRPOffset(10, -15);   //CCRP在GPS设备的右边10米，下边15米处
* @endcode
*/
ENCL_API void EnclOwnshipSetCCRPOffset(double dx, double dy);

/*! \brief 获取本船CCRP（公共参考点）
* \param dx 返回CCPR的横向偏移量（米），左负右正
* \param dy 返回CCRP的纵向偏移量（米），下负上正
* \return 无
*
* @par 示例:
* @code
*   double dx, dy;
*	EnclOwnshipGetCCRPOffset(&dx, &dy);
* @endcode
*/
ENCL_API void EnclOwnshipGetCCRPOffset(double *dx, double *dy);

/*! \brief 设置本船对地航向
* \param cog 对地航向（度）
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetCOG(-30.0);
* @endcode
*/
ENCL_API void EnclOwnshipSetCOG(double cog);

/*! \brief 获取本船对地航向
* \return 返回本船对地航向
*
* @par 示例:
* @code
    double cog = EnclOwnshipGetCOG();
* @endcode
*/
ENCL_API double EnclOwnshipGetCOG();

/*! \brief 设置本船对地速度
* \param sog 对地速度
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetSOG(25.0);
* @endcode
*/
ENCL_API void EnclOwnshipSetSOG(double sog);

/*! \brief 获取本船对地速度
* \return 返回本船对地速度
*
* @par 示例:
* @code
    double sog = EnclOwnshipGetSOG();
* @endcode
*/
ENCL_API double EnclOwnshipGetSOG();	// kn

/*! \brief 设置本船对水航向
* \param ctw 对水航向（度）
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetCTW(-30.0);
* @endcode
*/
ENCL_API void EnclOwnshipSetCTW(double ctw);

/*! \brief 获取本船对水航向
* \return 返回本船对水航向
*
* @par 示例:
* @code
    double ctw = EnclOwnshipGetCTW();
* @endcode
*/
ENCL_API double EnclOwnshipGetCTW();

/*! \brief 设置本船对水速度
* \param sog 对水速度（节）
* \return 无
*
* @par 示例:
* @code
*	EnclOwnshipSetSTW(25.0);
* @endcode
*/
ENCL_API void EnclOwnshipSetSTW(double stw);

/*! \brief 获取本船对水速度
* \return 返回本船对水速度
*
* @par 示例:
* @code
    double stw = EnclOwnshipGetSTW();
* @endcode
*/
ENCL_API double EnclOwnshipGetSTW();	// kn


ENCL_API void EnclOwnshipShowHeadingline(bool bShow);
ENCL_API void EnclOwnshipShowBeamline(bool bShow);
ENCL_API void EnclOwnshipShowFullScaleSymbol(bool bFullScale);
ENCL_API void EnclOwnshipShowVelocityVector(bool bShow);
ENCL_API void EnclOwnshipSetVelocityVectorTime(double min);
ENCL_API void EnclOwnshipSetVelocityVectorTimeInc(double min);
ENCL_API void EnclOwnshipShowTimeIncMarker(bool bShow);
ENCL_API void EnclOwnshipShowStabilizeIndicator(bool bShow);
ENCL_API void EnclOwnshipShowWaterRefVector(bool bShow);
ENCL_API void EnclOwnshipShowGroundRefVector(bool bShow);
ENCL_API void EnclOwnshipAddPastTrackPosition(PastTrackInfo & pti);
ENCL_API void EnclOwnshipSetPastTrackLength(double tInMin);
ENCL_API void EnclOwnshipSetPastTrackTimeLabelInterval(double tInMin);
ENCL_API void EnclOwnshipSetShowPastTrackTimeLabel(bool bShow);
ENCL_API void EnclOwnshipSetPaskTrackStyle(PastTrackStyle pts);
ENCL_API PastTrackInfo * EnclOwnshipGetAllPastTrackData(int &size);
ENCL_API PastTrackInfo * EnclOwnshipGetPastTrackByTime(int64_t start, int64_t end, int & size, PositionSource ps);
ENCL_API void EnclOwnshipSetShowPrimaryTrack(bool bShow);
ENCL_API void EnclOwnshipSetShowSecondaryTrack(bool bShow);
ENCL_API void EnclOwnshipFreePastTrackData(PastTrackInfo * data);
ENCL_API void EnclOwnshipDeleteAllPastTrack();
/** @} */

/** @defgroup aismng AIS管理 
* aismng AIS管理等相关功能
* @{
*/
ENCL_API EnclObjectPtr EnclAisCreateTarget(RcNMEAData *d, int *tgtmmsi);
ENCL_API EnclObjectPtr* EnclAisGetAllTargetIds(int *npids);
ENCL_API void EnclAisGetTargetInfo( EnclObjectPtr id, RcAISTargetInfo* targetinfo );
ENCL_API void EnclAisGetTargetInfoByMMSI( int mmsi, RcAISTargetInfo* targetinfo );
ENCL_API char * EnclAisGetNavigationStatusString(int status);
ENCL_API char * EnclAisGetShipTypeString(int type);
ENCL_API char * EnclAisGetClassString(int classno);
ENCL_API void EnclAisGetReportTime(ais_transponder_class type, int navstatus, int cs, double speed, int *imotime, int *losttime, int *autodeltime);
ENCL_API void EnclAisDeleteTarget(int mmsi=0);
ENCL_API void EnclAisSetTargetShow(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetActive(bool active, int mmsi=0);
ENCL_API void EnclAisSetTargetDanger(bool danger, int mmsi=0);
ENCL_API void EnclAisSetTargetLost(bool lost, int mmsi=0);
ENCL_API void EnclAisSetTargetSelected(bool select, int mmsi=0, int idx=0);
ENCL_API bool EnclAisSetTargetAssociated(bool associated, int mmsi=0);
ENCL_API void EnclAisSetTargetShowAssociated(bool showassociated, int mmsi=0);
ENCL_API void EnclAisSetTargetLabel(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetHeadingline(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetTurnIndicator(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetFullScaleSymbol(bool bFullScale, int mmsi=0);
ENCL_API void EnclAisSetTargetVelocityVector(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetVelocityVectorTime(double min, int mmsi=0);
ENCL_API void EnclAisSetTargetVelocityVectorTimeInc(double min, int mmsi=0);
ENCL_API void EnclAisSetTargetTimeIncMarker(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetPastTrack(bool bShow, int mmsi=0);
ENCL_API void EnclAisSetTargetPastTrackLength(double tInMin, int mmsi=0);
ENCL_API void EnclAisSetTargetPastTrackTimeLabelInterval(double tInMin, int mmsi=0);
ENCL_API void EnclAisSetTargetPaskTrackStyle(PastTrackAisStyle pts, int mmsi=0);
ENCL_API EnclObjectPtr* EnclQueryPickAisTargets(double lon, double lat, int pickrad, int *count);
/** @} */

/** @defgroup ttmng 跟踪目标管理 
* ttmng 跟踪目标管理等相关功能
* @{
*/
ENCL_API EnclObjectPtr EnclTtCreateTarget(RcNMEAData *d, int *tgtnum);
ENCL_API EnclObjectPtr* EnclTtGetAllTargetIds(int *npids);
ENCL_API void EnclTtGetTargetInfo( EnclObjectPtr id, RcTTTargetInfo* targetinfo );
ENCL_API void EnclTtGetTargetInfoByNum( int num, RcTTTargetInfo* targetinfo );
ENCL_API void EnclTtDeleteTarget(int num=0);
ENCL_API void EnclTtSetTargetShow(bool bShow, int num=0);
ENCL_API void EnclTtSetTargetDanger(bool danger, int num=0);
ENCL_API void EnclTtSetTargetLost(bool lost, int num=0);
ENCL_API void EnclTtSetTargetSelected(bool select, int num=0, int idx=0);
ENCL_API bool EnclTtSetTargetAssociated(bool associated, int num=0);
ENCL_API void EnclTtSetTargetShowAssociated(bool showassociated, int num=0);
ENCL_API void EnclTtSetTargetLabel(bool bShow, int num=0);
ENCL_API void EnclTtSetTargetVelocityVector(bool bShow, int num=0);
ENCL_API void EnclTtSetTargetVelocityVectorTime(double min, int num=0);
ENCL_API void EnclTtSetTargetVelocityVectorTimeInc(double min, int num=0);
ENCL_API void EnclTtSetTargetTimeIncMarker(bool bShow, int num=0);
ENCL_API void EnclTtSetTargetPastTrack(bool bShow, int num=0);
ENCL_API void EnclTtSetTargetPastTrackLength(double tInMin, int num=0);
ENCL_API void EnclTtSetTargetPastTrackTimeLabelInterval(double tInMin, int num=0);
ENCL_API void EnclTtSetTargetPaskTrackStyle(PastTrackTtStyle pts, int num=0);
ENCL_API EnclObjectPtr* EnclQueryPickTtTargets(double lon, double lat, int pickrad, int *count);
/** @} */

// Anchor watch
/** @defgroup anchor 锚位检测 
* anchor 锚位检测等相关功能
* @{
*/
ENCL_API void EnclAnchorWatchSetPosition(double lon, double lat);
ENCL_API void EnclAnchorWatchSetRange(double rangeInMeters);
ENCL_API void EnclAnchorWatchEnable();
ENCL_API void EnclAnchorWatchDisable();
// return true if inside watch range, return false otherwise.
ENCL_API bool EnclAnchorWatchQuery(double * dist, double *bearing);
/** @} */

/** @defgroup alert 警告管理 
* alert 警告管理等相关功能
* @{
*/
ENCL_API  EnclObjectPtr EnclAlertNewAlert(int alertId, int alertInst);
// Call EnclNMEAFreeSentences to free returned NMEA sentences string.
ENCL_API  char * EnclAlertGetAlertALFSentences(EnclObjectPtr alert);
ENCL_API  char * EnclAlertGetALCSentences();
ENCL_API  void EnclAlertUpdateAlerts();
// Call EnclFreeObjectIDs to free returned alert object array.
ENCL_API  EnclObjectPtr * EnclAlertGetAllAlerts(int & nCnt);
ENCL_API void EnclAlertGetAlertInfo(EnclObjectPtr id, EnclAlert & alertInfo);
ENCL_API  EnclObjectPtr EnclAlertGetAlert(int alertId, int alertInst);
// return false indicate a refuse of acknowledge.
ENCL_API  bool EnclAlertAcknowledgeAlert(EnclObjectPtr id, bool remote);
ENCL_API  void EnclAlertSilenceAlert(EnclObjectPtr id);
ENCL_API  void EnclAlertClearAlert(EnclObjectPtr id);
ENCL_API  void EnclAlertRectifyAlert(EnclObjectPtr id);
ENCL_API  void EnclAlertTransferAlert(EnclObjectPtr id);
ENCL_API  EnclAlertTableEntry* EnclAlertGetAlertDef(int alertId, int alertInst);
ENCL_API  void EnclAlertAddAlertDef(const EnclAlertTableEntry & entry);
/** @} */

/** @defgroup other 其他 
* other 其他功能
* @{
*/
// Free Object ID object array. Call this function to free Object ID array return by APIs.
ENCL_API void EnclFreeObjectIDs(EnclObjectPtr * pIDs);
ENCL_API void EnclQueryFreeObjectIDs(EnclObjectPtr * objIDs);

// Encode angle to DegreeMinuteSecond String. flag: indicate the angle is longitude or latitude
// which will and S/W/N/E to the end of the result string. comp indicate the result string will
// be ended with degree / minute / second. precision is the number of digits after decimal point.
// User have to provide enough space for output buffer.
ENCL_API void EnclUtilityDMSEncode(double angle, DmsFlag flag, DmsComponent comp, int precision, char * output);
// Turn zero terminated string to angle (double). Allowed formats refer to the GeographicLib DMS:Decode .
ENCL_API double EnclUtilityDMSDecode(const char * string);
/** @} */


/** @defgroup  鹰眼
* 鹰眼显示及操作相关功能
* @{
*/

/*! \brief 获取鹰眼图片数据
* \param[in] width 指定鹰眼图片宽度
* \param[in] height 指定鹰眼图片高度
* \return 鹰眼图片RGB格式像素数据
*/
ENCL_API EnclImgDataPtr EnclEagleEyeGetImage(int width, int height);

/*! \brief 鹰眼视口坐标转换。将地理坐标转换为鹰眼图片上像素单位坐标。鹰眼图片左上角为（0,0）点。
* y 向下为正。
* \param[in] lon  地理坐标经度
* \param[in] lat  地理坐标纬度
* \param[out] x	  鹰眼像素坐标x
* \param[out] y	  鹰眼像素最表y
*/
ENCL_API void EnclEagleEyeGeoToPix(double lon, double lat, int & x, int &y);


/*! \brief 鹰眼视口坐标转换。将鹰眼图片上像素单位坐标转换为地理坐标。鹰眼图片左上角为（0,0）点。
* y 向下为正。
* \param[in] x	  鹰眼像素坐标x
* \param[in] y	  鹰眼像素最表y
* \param[out] lon  地理坐标经度
* \param[out] lat  地理坐标纬度
*/
ENCL_API void EnclEagleEyePixToGeo(int x, int y,double & lon, double & lat);
/** @} */

#endif // ECDISINTERFACE_H
