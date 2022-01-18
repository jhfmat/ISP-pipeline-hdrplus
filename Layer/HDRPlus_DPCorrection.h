#ifndef __HDRPlus_DEFECT_PIXEL_CORRECION_H_
#define __HDRPlus_DEFECT_PIXEL_CORRECION_H_
#include "../Mat/WeightConfig.h"
class CHDRPlus_DPCorrection : public CSingleConfigTitleFILE
{
private:
	int m_nBLC;
	int m_nMAXS;
	int ProcessBlock(unsigned int nWindow[][5],unsigned int nMax[5], unsigned int nMin[5]);
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("bWhitePointCEnable", &m_bWhitePointCEnable, 0, 1);
		m_bWhitePointCEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nWhitePointCThre", &m_nWhitePointCThre, 0, 65536);
		m_nWhitePointCThre = 5;//越小白点少
		m_nConfigParamList.ConfigParamListAddVariable("nWhitePointLRatioT", &m_nWhitePointLRatioT, 0, 65536);
		m_nWhitePointLRatioT = 0;
		m_nConfigParamList.ConfigParamListAddVariable("bBlackPointCEnable", &m_bBlackPointCEnable, 0, 1);
		m_bBlackPointCEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nBlackPointCThre", &m_nBlackPointCThre, 0, 65536);
		m_nBlackPointCThre = 5;//越小黑点越少
		m_nConfigParamList.ConfigParamListAddVariable("nBlackPointLRatioT", &m_nBlackPointLRatioT, 0, 65536);
		m_nBlackPointLRatioT = 0;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_DPCorrection");
	}
public:
	int m_bDumpFileEnable;
	int m_bWhitePointCEnable;
	int m_nWhitePointCThre;
	int m_nWhitePointLRatioT;
	int m_bBlackPointCEnable;
	int m_nBlackPointCThre;
	int m_nBlackPointLRatioT;
	CHDRPlus_DPCorrection()
	{
		Initialize();
	}
	void ProcessLine(unsigned short *pInLines[], unsigned short *pOutLine, int nWidth);
	bool Forward(MultiUshortImage *pInImage, MultiUshortImage *pOutImage, TGlobalControl *pControl);
};

#endif
