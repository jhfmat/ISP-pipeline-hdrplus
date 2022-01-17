#ifndef __CONFIG_FILTER_H_
#define __CONFIG_FILTER_H_
#include "SubFunction.h"
#include "MultiUshortImage.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
#include "MultiIntImage.h"
typedef struct TagConfigParameter
{
	char m_pConfigParamName[64];
	int *m_pConfigParamAddr;
	int m_nConfigParamMin;
	int m_nConfigParamMax;
	int m_nConfigParamScale;
	TagConfigParameter *m_pConfigParamNext;
	TagConfigParameter()
	{
		m_pConfigParamAddr=NULL;
		m_pConfigParamNext =NULL;
		m_nConfigParamMin = 0;
		m_nConfigParamMax = 0x7fffffff;
		m_nConfigParamScale = 1;
	}
}TConfigParam;
class CConfigParamList
{
private:
	int m_nConfigParamListCount;
	TConfigParam *m_pConfigParamListFirst;
	TConfigParam *m_pConfigParamListLast;
	bool GetConfigParamListValueFromLineStr(char *linestr, char *name, int &val)
	{
		int i, len;
		len = strlen(linestr);
		for (i = 0; i < len; i++)
		{
			if (linestr[i] == '=')
			{
				if (linestr[i + 1] == 'H' || linestr[i + 1] == 'h')//十六进制数值
				{
					if (sscanf(linestr + i + 2, "%x\n", &val) != 1)return false;
				}
				else
				{
					if (sscanf(linestr + i + 1, "%d\n", &val) != 1)return false;
				}
				memcpy(name, linestr, i);//拷贝name
				name[i] = '\0';
				return true;
			}
		}
		return false;
	}
public:
	void ClearConfigParamListMem()
	{
		while (m_pConfigParamListFirst != NULL)
		{
			m_pConfigParamListLast = m_pConfigParamListFirst->m_pConfigParamNext;
			delete m_pConfigParamListFirst;
			m_pConfigParamListFirst = m_pConfigParamListLast;
		}
		m_nConfigParamListCount = 0;
	}
	bool ConfigParamListAddVariable(char *pParamName, int *pParamAddr, int nMin=0, int nMax=0x7fffffff,int nScale=1)
	{
		TConfigParam *pParamItem = new TConfigParam;
		if (pParamItem == NULL)return false;
		strcpy(pParamItem->m_pConfigParamName, pParamName);
		pParamItem->m_pConfigParamAddr = pParamAddr;
		pParamItem->m_nConfigParamMin = nMin;
		pParamItem->m_nConfigParamMax = nMax;
		pParamItem->m_nConfigParamScale = nScale;
		if (m_pConfigParamListFirst == NULL)
		{
			m_pConfigParamListFirst = m_pConfigParamListLast = pParamItem;
		}
		else
		{
			m_pConfigParamListLast->m_pConfigParamNext = pParamItem;
			m_pConfigParamListLast = pParamItem;
		}
		m_nConfigParamListCount++;
		return true;
	}
	bool ConfigParamListAddArray(char *pParamName, int pVector[], int nCh, int nMin = 0, int nMax = 0x7fffffff,int nScale = 1)
	{
		char ParamName[64];
		for (int i = 0; i < nCh; i++)
		{
			sprintf(ParamName, "%s_%d", pParamName, i);
			if (!ConfigParamListAddVariable(ParamName, pVector + i, nMin, nMax, nScale))return false;
		}
		return true;
	}
	bool ConfigParamListAddMatrixRow(char *pParamName, int *pMatrixRow, int nRow, int nCol, int nMin = 0, int nMax = 0x7fffffff,int nScale = 1)
	{
		char ParamName[64];
		sprintf(ParamName, "%s_%d", pParamName, nRow);
		if (!ConfigParamListAddArray(ParamName, pMatrixRow, nCol, nMin, nMax, nScale))return false;
		return true;
	}
	TConfigParam * FindConfigParamName(char *pName)
	{
		TConfigParam *pParamItem = m_pConfigParamListFirst;
		while (pParamItem != NULL)
		{
			if (strcmp(pParamItem->m_pConfigParamName, pName) == 0)return pParamItem;
			pParamItem = pParamItem->m_pConfigParamNext;
		}
		return pParamItem;
	}
	bool LoadParamListFromFile(FILE *fp)
	{
		char linestr[256], ParamName[64];
		int ParamValue;
		TConfigParam *pItemParam;
		if (fp == NULL)return false;
		for (int i = 0; i < m_nConfigParamListCount; i++)
		{
			fgets(linestr, 256, fp);
			if (!GetConfigParamListValueFromLineStr(linestr, ParamName, ParamValue))return false;
			pItemParam = FindConfigParamName(ParamName);
			if (pItemParam != NULL)
			{
				if (ParamValue<pItemParam->m_nConfigParamMin || ParamValue>pItemParam->m_nConfigParamMax)
				{
					printf("%s ConfigParam value %d is out of ValueRange [%d,%d]!!!\n", ParamName, ParamValue, pItemParam->m_nConfigParamMin, pItemParam->m_nConfigParamMax);
					if (ParamValue < pItemParam->m_nConfigParamMin)
					{
						ParamValue = pItemParam->m_nConfigParamMin;
					}
					if (ParamValue > pItemParam->m_nConfigParamMax)
					{
						ParamValue = pItemParam->m_nConfigParamMax;
					}
				}
				*(pItemParam->m_pConfigParamAddr) = ParamValue;
			}
			else
			{
				printf("is not Parameter name == %s!!!\n", ParamName);
				return false;
			}
		}
		return true;
	}
	bool SaveParamListToFile(FILE *fp)
	{
		TConfigParam *pParamItem = m_pConfigParamListFirst;
		if (fp == NULL)return false;
		for (int i = 0; i < m_nConfigParamListCount; i++)
		{
			int ParamValue = *(pParamItem->m_pConfigParamAddr);
			int res = fprintf(fp, "%s=%d;\tValueRange=[%d,%d,%d]\n", pParamItem->m_pConfigParamName, ParamValue, pParamItem->m_nConfigParamMin, pParamItem->m_nConfigParamMax, pParamItem->m_nConfigParamScale);
			pParamItem = pParamItem->m_pConfigParamNext;
		}
		return true;
	}
	CConfigParamList()
	{
		m_pConfigParamListFirst=m_pConfigParamListLast=NULL;
		m_nConfigParamListCount =0;
	}
	~CConfigParamList()
	{
		ClearConfigParamListMem();
	}
};
class CSingleConfigTitleFILE
{
protected:
	virtual void CreateConfigTitleName() = 0;//创建标题----------纯虚函数由子类实现
	virtual void InitConfigParamList() {};//创建参数列表----------虚函数有子类实现（关联成员变量m_nConfigParamList列表）
	virtual void UpdateInternalConfig(){};//更新参数列表----------虚函数由子类实现
public:
	char m_pConfigTitleName[256];//标题
	CConfigParamList m_nConfigParamList;//参数列表
	virtual void Initialize()//子类初始化中先调用他完成父类虚化函数的实现
	{
		CreateConfigTitleName();
		InitConfigParamList();
	}
	virtual bool LoadConfigFile(char *pConfigFileName)//子类没有实现有父类实现
	{
		char ConfigTitleName[64];
		FILE *fp=fopen(pConfigFileName, "rt");
		if(fp==NULL)return false;
		if(fscanf(fp, "\n%s\n", ConfigTitleName)<=0)
		{
			fclose(fp);
			return false;
		}
		if (!(strcmp(m_pConfigTitleName, ConfigTitleName) == 0))
		{
			printf("Can not match Title name %s!!!\n", ConfigTitleName);
			fclose(fp);
			return false;
		}
		if (!m_nConfigParamList.LoadParamListFromFile(fp))
		{
			fclose(fp);
			return false;
		}
		UpdateInternalConfig();
		fclose(fp);
		return true;
	}
	virtual bool SaveSingleConfigFile(char *pFileName)
	{
		FILE *fp=fopen(pFileName, "wt");
		if(fp==NULL)return false;
		fprintf(fp, "\n%s\n", m_pConfigTitleName);//保存标题
		if (!m_nConfigParamList.SaveParamListToFile(fp))//保存参数列表
		{
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}
};
class CMultiConfigFILE : public CSingleConfigTitleFILE
{
protected:
	typedef struct tagConfigTitleList
	{
		CSingleConfigTitleFILE *pSingleConfigTitleFILE;//单项类指针
		tagConfigTitleList *pTitleListNext;//单项表指针
		tagConfigTitleList()
		{
			pSingleConfigTitleFILE =NULL;
			pTitleListNext =NULL;
		}
	}TConfigTitleList;//存放多个单文档
	TConfigTitleList *m_pConfigTitleListFirst;
	TConfigTitleList *m_pConfigTitleListLast;
	int m_nConfigTitleListCount;
	void ClearMultiConfigMem()//将first,last,next相等就相当于没有 count也归0
	{
		while (m_pConfigTitleListFirst != NULL)
		{
			m_pConfigTitleListLast = m_pConfigTitleListFirst->pTitleListNext;
			delete m_pConfigTitleListFirst;
			m_pConfigTitleListFirst = m_pConfigTitleListLast;
		}
		m_nConfigTitleListCount = 0;
	}
	void AddConfigTitle(CSingleConfigTitleFILE *pSingleConfig, char *pNewConfigName=NULL)
	{
		if (pNewConfigName != NULL)
		{
			strcat(pSingleConfig->m_pConfigTitleName, pNewConfigName);
		}
		TConfigTitleList *pTmpTitleList = new TConfigTitleList;
		pTmpTitleList->pSingleConfigTitleFILE = pSingleConfig;
		if (m_pConfigTitleListFirst == NULL)
		{
			m_pConfigTitleListFirst = pTmpTitleList;
		}
		else
		{
			m_pConfigTitleListLast->pTitleListNext = pTmpTitleList;
		}
		m_pConfigTitleListLast = pTmpTitleList;
		m_nConfigTitleListCount++;
	}
	virtual void CreateConfigTitleNameList()=0;//由子类实现
public: 
	virtual void Initialize()//子类初始化中
	{
		CreateConfigTitleName();//总标题创建
		CreateConfigTitleNameList();//分标题创建
		InitConfigParamList();//标题下面的参数创建
	}
	CMultiConfigFILE()
	{
		m_pConfigTitleListFirst=m_pConfigTitleListLast=NULL;
		m_nConfigTitleListCount =0;
	}
	~CMultiConfigFILE()
	{
		ClearMultiConfigMem();
	}
	virtual bool LoadMultiConfigFile(char *pFileName)
	{
		char pConfigTitle[64];
		FILE *fp = fopen(pFileName, "rt");
		if (fp == NULL)return false;
		if (fscanf(fp, "\n%s\n", pConfigTitle) <= 0)
		{
			fclose(fp);
			return false;
		}
		if (!(strcmp(m_pConfigTitleName, pConfigTitle) == 0))
		{
			printf("Can not match Title name %s!!!\n", pConfigTitle);
			fclose(fp);
			return false;
		}
		if (!m_nConfigParamList.LoadParamListFromFile(fp))//本身这个类保存的标题和参数（成员变量用子类的）
		{
			fclose(fp);
			return false;
		}
		int i = 0;
		while (i < m_nConfigTitleListCount)//从链表中取
		{
			if (fscanf(fp, "\n%s\n", pConfigTitle) <= 0) //文件中读取一行
			{
				fclose(fp);
				return false;
			}
			CSingleConfigTitleFILE *pSingleConfig =NULL ;
			TConfigTitleList *pTmpTitleList = m_pConfigTitleListFirst;
			while (pTmpTitleList != NULL)
			{
				if ((strcmp(pTmpTitleList->pSingleConfigTitleFILE->m_pConfigTitleName, pConfigTitle) == 0))//对比文件中读取的和链表中的有的话就将链表中的项取出来
				{
					pSingleConfig= pTmpTitleList->pSingleConfigTitleFILE;
					break;
				}
				pTmpTitleList = pTmpTitleList->pTitleListNext;
			}
			if (pSingleConfig == NULL)
			{
				printf("Can not find Title name %s!!!\n", pConfigTitle);
				return false;
			}
			if (!pSingleConfig->m_nConfigParamList.LoadParamListFromFile(fp))
			{
				fclose(fp);
				return false;
			}
			UpdateInternalConfig();
			i++;
		}
		fclose(fp);
		return true;
	}
	virtual bool SaveMultiConfigFile(char *pFileName)
	{
		FILE *fp = fopen(pFileName, "wt");
		if (fp == NULL)return false;
		fprintf(fp, "\n%s\n", m_pConfigTitleName);//本身这个类保存的标题和参数（成员变量用子类的）
		if (!m_nConfigParamList.SaveParamListToFile(fp))
		{
			fclose(fp);
			return false;
		}
		TConfigTitleList *pTmpTitleList = m_pConfigTitleListFirst;//文件标题链表取数据从第一个取
		while (pTmpTitleList != NULL)
		{
			fprintf(fp, "\n%s\n", pTmpTitleList->pSingleConfigTitleFILE->m_pConfigTitleName);
			if (!pTmpTitleList->pSingleConfigTitleFILE->m_nConfigParamList.SaveParamListToFile(fp))
			{
				fclose(fp);
				return false;
			}
			pTmpTitleList = pTmpTitleList->pTitleListNext;//取下一个
		}
		fclose(fp);
		return true;
	}
};
#endif