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
*	EnclSENCInit("/usr/local/Enclib");
*	EnclSENCInit(0);
* @endcode
*/
ENCL_API bool EnclSENCInit(const char *encLibPath);

/*! \brief 释放 Enclib 库
* \return 无。
*/
ENCL_API void EnclSENCFree();

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

/*! \brief 获取海图库版本字符串.
* \return 海图库版本字符串.
*/
ENCL_API const char* EnclSoftwareGetVersion();
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
* \return 成功返回true，否则返回false。
*/
ENCL_API bool EnclSENCMENCImportFile(const char * encFile);

/*! \brief 安装MENC海图（目录方式）
* \param encPath 目录路径。
* \return 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
*/
ENCL_API int EnclSENCMENCImportTree(const char * encPath);

/*! \brief 安装S57海图（单个文件方式）
* \param encFile 待安装海图文件路径名称。
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	bool ret = EnclSENCS57ImportFile(cellfpath.toStdString().c_str());
* @endcode
*/
ENCL_API bool EnclSENCS57ImportFile(const char * encFile);

/*! \brief 安装S57海图（目录方式）
* \param encPath 目录路径。
* \return 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
*
* @par 示例:
* @code
*	static bool ErrCallBack(EnclImportType type, EnclImportStep step, int errorCode, const char *chartno, EnclErrCellPermitCallbackData *usrdata)
*	{
*	    ...
*	}
*    
*   EnclSENCSetImportErrorCallback(ErrCallBack);
*	int errnum = EnclSENCS57ImportTree(rootpath);
* @endcode
*/
ENCL_API int EnclSENCS57ImportTree(const char * encPath);

/*! \brief 安装S63海图（目录方式）
* \param encPath 目录路径。
* \param permitFile 许可证文件路径。
* \param hwId HWID。
* \param crtFile 安全证书路径。
* \return 返回的错误码，0 表示成功，其他错误码可以查阅相关文档
*
* @par 示例:
* @code
*	static bool ErrCallBack(EnclImportType type, EnclImportStep step, int errorCode, const char *chartno, EnclErrCellPermitCallbackData *usrdata)
*	{
*	    ...
*	}
*    
*   EnclSENCSetImportErrorCallback(ErrCallBack);
*	int errnum = EnclSENCS63ImportTree(rootPath, permitFilePath, hw_id, ihoCrtFilePath);
* @endcode
*/
ENCL_API int EnclSENCS63ImportTree(const char* encPath, const char* permitFile, const unsigned char* hwId, const char* crtFile);

/*! \brief 设置安装回调函数，用于安装过程中相关信息输出
* \param iec 回调函数。
* \return 无
*
* @par 示例:
* @code
*	参考 EnclSENCS63ImportTree() 示例
* @endcode
*/
ENCL_API void EnclSENCSetImportErrorCallback(EnclSENCImportErrCallBack iec);

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

/*! \brief 设置海图风格
* \param style ENCL_DS_STANDARD：标准风格   ENCL_DS_PAPER：纸图风格
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetDisplayStyle((EnclDisplayStyle)toInt(m_params[0]));
*	EnclDisplayStyle ret = EnclDrawGetDisplayStyle();
* @endcode
*/
ENCL_API void EnclDrawSetDisplayStyle(EnclDisplayStyle style);

/*! \brief 获取海图风格
* \return 返回海图风格，ENCL_DS_STANDARD：标准风格   ENCL_DS_PAPER：纸图风格
*
* @par 示例:
* @code
*	参考 EnclDrawSetDisplayStyle() 示例
* @endcode
*/
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

/*! \brief 设置孤立危险物开关，用于控制所有孤立危险物的显示。
* \param flag true表示打开，false表示关闭
* \return 无。
*
* @par 示例:
* @code
*	EnclDrawSetShowIsolatedDangerObjects(true);
* @endcode
*/
ENCL_API void EnclDrawSetShowIsolatedDangerObjects(bool bShow);

/*! \brief 获取孤立危险物开关状态
* \return 返回孤立危险物开关状态，true打开，false关闭。
*
* @par 示例:
* @code
*	bool ret = EnclDrawGetShowIsolatedDangerObjects();
* @endcode
*/
ENCL_API bool EnclDrawGetShowIsolatedDangerObjects();

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

/** @defgroup viewport 视口设置 
* 视口尺寸设置，旋转，漫游等
* @{
*/

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

/*! \brief 设置视口旋转角度
* \param angle 旋转角度（度），正北为0，顺时针为正
* \return 成功返回true，否则返回false。
*
* @par 示例:
* @code
*	bool ret = EnclViewRotate(30);
* @endcode
*/
ENCL_API bool EnclViewRotate(double angle);

/*! \brief 设置视口中心
* \param lon 中心点经度
* \param lat 中心点纬度
* \return 无。
*
* @par 示例:
* @code
*	EnclViewCenter(104.321, 32.1);
* @endcode
*/
ENCL_API void EnclViewCenter(double lon, double lat);

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

/*! \brief 释放物标ID列表
* \param objIDs 物标ID列表指针
* \return 无
*/
ENCL_API void EnclQueryFreeObjectIDs(EnclObjectPtr * objIDs);

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

/*! \brief 将屏幕坐标转换为地理坐标
* \param scrnPointX 屏幕坐标x
* \param scrnPointY 屏幕坐标y
* \param lon 返回经度
* \param lat 返回纬度
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	int scrnX = toInt(m_params[0]);
*	int scrnY = toInt(m_params[1]);
*	
*	double lat, lon;
*	EnclTransformScrnToGeo(scrnX, scrnY, &lon, &lat);
* @endcode
*/
ENCL_API bool EnclTransformScrnToGeo(int scrnPointX, int scrnPointY, double* lon, double* lat, double scale=0.0);

/*! \brief 将地理坐标转换为屏幕坐标
* \param lon 待转换点经度
* \param lat 待转换点纬度
* \param scrnPointX 返回屏幕坐标x
* \param scrnPointY 返回屏幕坐标y
* \return 成功返回true，否则返回false
*
* @par 示例:
* @code
*	double lon = toDouble(m_params[0]);
*	double lat = toDouble(m_params[1]);
*	
*	int x, y;
*	EnclTransformGeoToScrn(lon, lat, &x, &y);
* @endcode
*/
ENCL_API bool EnclTransformGeoToScrn(double lon, double lat, int * scrnPointX, int * scrnPointY, double scale=0.0);

//nouse
ENCL_API bool EnclTransformToWGS84(double lon1, double lat1, double* lon2, double* lat2);	//TODO	confirm cancel.
ENCL_API bool EnclTransformFromWGS84(double lon1, double lat1, double* lon2, double* lat2);	//TODO	confirm cancel.
// following 2 apis confirm cancel, but still exist in standard, so we return correctly
ENCL_API bool  EnclTransformSetDatum(EnclDatum  value);		//TODO	confirm cancel.
ENCL_API EnclDatum  EnclTransformGetDatum();		//TODO	confirm cancel.
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
/** @} */

/** @defgroup manualupdate 手动更新
* 手动物标的新增，删除，
* @{
*/

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

/*! \brief 新增航线
* \param obj 新增航线的物标信息指针
* \return 返回新增航线的ID
*
* @par 示例:
* @code
*	EnclRoute route;
*	route.ptCount = count;
*	route.cords = new double[count*2];
*	QString pos;
*	for(int i=0;i<m_params.size();i++){
*	    route.cords[i] = m_params[i].toDouble();
*	}
*	for(int i=0;i<m_params.size();i+=2){
*	    pos += QString("(%1,%2) ").arg(route.cords[i]).arg(route.cords[i+1]);
*	}
*	
*	EnclObjectPtr routeId = EnclManualUpdateAddRoute(&route);
*
*	EnclManualUpdateDeleteRoute(routeId);
*
* @endcode
*/
ENCL_API EnclObjectPtr EnclManualUpdateAddRoute(EnclRoute* obj);

/*! \brief 删除航线
* \param id 航线ID
* \return 成功返回true, 否则返回false
*
* @par 示例:
* @code
*	参考 EnclManualUpdateAddRoute() 示例
* @endcode
*/
ENCL_API bool EnclManualUpdateDeleteRoute(EnclObjectPtr id);
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
