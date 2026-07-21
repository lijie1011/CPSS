#ifndef __S57_CATALOG_H__ 
#define __S57_CATALOG_H__

//! Catalog 文件中的文件路径信息
typedef struct
{
	char RCNM[3];	/**<记录名，值确定为"CD"*/
	int RCID; 		/**<记录ID*/
	char* FILE;		/**<短文件名*/
	char* LFIL;		/**<长文件名*/
	char* VOLM;		/**<文件所在盘符*/
	char IMPL[4];	/**<文件类型：ASC，文本文件；BIN，二进制文件*/
	double SLAT;	/**<海图覆盖范围，最南纬度*/
	double WLON;	/**<海图覆盖范围，最东经度*/
	double NLAT;	/**<海图覆盖范围，最北纬度*/
	double ELON;	/**<海图覆盖范围，最西经度*/
	char* CRCS;		/**<CRC校验和*/
	char* COMT; 	/**<附加信息*/
}S57CatalogDirectory;

//! Catalog文件中的交叉引用信息
typedef struct
{
	char RCNM[3];	/**<记录名，值确定为"CR":Cross Reference*/
	int RCID;		/**<记录ID*/
	char* NAM1;		/**<Name 1*/
	char* NAM2;		/**<Name 2*/
	char* COMT;		/**<附加信息*/
}S57CatalogCrossReference;

//! 数据集 Catalogue信息
typedef struct
{
	int nCatalogDirectories; 					/**<文件路径记录个数*/
	S57CatalogDirectory** catalogdirectories;	/**<文件路径记录*/
	int nCatalogCrossReferences;				/**<交叉引用记录个数*/
	S57CatalogCrossReference** crossreferences;	/**<交叉引用记录*/
}S57DatasetCatalogueInfo;

#endif
