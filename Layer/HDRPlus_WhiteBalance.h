#ifndef __HDRPlus_WhiteBalance_H_
#define __HDRPlus_WhiteBalance_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_WhiteBalance : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_WhiteBalance");
	}
public:
	int m_bDumpFileEnable;
	int m_nMin;
	int m_nMax;
	CHDRPlus_WhiteBalance()
	{
		Initialize();
	}
	void Forward(MultiUshortImage * pInRawImage, TGlobalControl * pControl);
};

#endif