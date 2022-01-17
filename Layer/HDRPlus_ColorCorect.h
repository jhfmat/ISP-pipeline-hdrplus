#ifndef __HDRPlus_ColorCorect_H_
#define __HDRPlus_ColorCorect_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_ColorCorect : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 0;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_ColorCorect");
	}
public:
	int m_bDumpFileEnable;
	CHDRPlus_ColorCorect()
	{
		Initialize();
	}
	void Forward(MultiUshortImage * pRGBImage, TGlobalControl * pControl);
};

#endif