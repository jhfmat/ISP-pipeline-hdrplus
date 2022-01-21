#ifndef __HDRPlus_GammaCorrect_H_
#define __HDRPlus_GammaCorrect_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_GammaCorrect : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 0;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_GammaCorrect");
	}
public:
	int m_bDumpFileEnable;
	int m_nMin;
	int m_nMax;
	CHDRPlus_GammaCorrect()
	{
		Initialize();
	}
	void Forward(MultiUshortImage * pRGBImage, TGlobalControl *pControl);
};

#endif