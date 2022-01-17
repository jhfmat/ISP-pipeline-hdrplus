#ifndef __HDRPlus_BlackWhiteLevel_H_
#define __HDRPlus_BlackWhiteLevel_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_BlackWhiteLevel : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_BlackWhiteLevel");
	}
public:
	int m_bDumpFileEnable;
	CHDRPlus_BlackWhiteLevel()
	{
		Initialize();
	}
	void Forward(MultiUshortImage * pInRawImage, TGlobalControl * pControl);
};

#endif