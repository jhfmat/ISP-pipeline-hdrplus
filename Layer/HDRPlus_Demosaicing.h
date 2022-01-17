#ifndef __HDRPlus_Demosaicing_H_
#define __HDRPlus_Demosaicing_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_Demosaicing : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Demosaicing");
	}
public:
	int m_bDumpFileEnable;
	CHDRPlus_Demosaicing()
	{
		Initialize();
	}
	bool Forward(MultiUshortImage * pInRAWImage, MultiUshortImage * pOutRGBImage, TGlobalControl * pControl);
};

#endif