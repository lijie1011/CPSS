#ifndef __IMPORT_CALLBACK_DEFS_H__
#define __IMPORT_CALLBACK_DEFS_H__

#include "s63defs.h"

typedef enum
{
    ENCL_STEP_READ_CALALOG=0,
    ENCL_STEP_CHECK_DATASETINTEGRITY,
    ENCL_STEP_GET_CORRESPONDINGPERMIT,
    ENCL_STEP_CHECK_CERTIFICATE,
    ENCL_STEP_DECRYPT_S63ENC,
    ENCL_STEP_CONVERT_TOSENC,
    ENCL_STEP_DONE,
}EnclImportStep;

typedef enum
{
    ENCL_IMPORT_MENC=0,
    ENCL_IMPORT_S63,
    ENCL_IMPORT_S57,
}EnclImportType;

//! 海图导入过程回调函数
/*!
*	type： 导入类型，导入s57 或 s63 数据
*	step: 当前导入过程所处阶段
*	pszChartName： 当前导入文件所在路径
*	iCurChart: 当前文件索引
*	totalChartCnt: 要导入文件总数
*	progress: 当前进度(0~1)
*	usrData: 用户数据指针，最终将传递给回调函数
*/
typedef void (*ImportProcCallback)(EnclImportType type, EnclImportStep step,
	const char* pszChartName, int iCurChart, int totalChartCnt, float progress, 
	void * usrData );

//! 海图导入错误信息回调函数
/*!
*	type： 导入类型，导入s57 或 s63 数据
*	step: 当前导入过程所处阶段
*	pszChartName： 当前导入文件所在路径
*	errorCode: 错误编码
*	usrData: 用户数据指针，最终将传递给回调函数
*/
typedef bool (*EnclSENCInstallErrCallBack)(EnclImportType type, EnclImportStep step,
	int errorCode, const char* chartno, const char *ds, void *usrdata);
typedef bool (*EnclSENCImportErrCallBack)(EnclImportType type, EnclImportStep step,
	int errorCode, const char* chartno, EnclErrCellPermitCallbackData *data);

typedef bool (*EnclS63PermitCallback)(EnclS63CellPermitInfo *, bool formatFlag, void *);

#endif
