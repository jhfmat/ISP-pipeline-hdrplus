#ifndef __HDRPlus_DEFECT_PIXEL_CORRECION_H_
#define __HDRPlus_DEFECT_PIXEL_CORRECION_H_
#include "../Mat/WeightConfig.h"
class CHDRPlus_DPCorrection : public CSingleConfigTitleFILE
{
private:
	int m_nBLC;
	int m_nMAXS;
	int m_nWindow[3][5];
	int m_nSum[5];
	int m_nMax[5];
	int m_nMin[5];
	int ProcessWindow(int nWindow[][5], int nMax[5], int nMin[5]);	
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("bWPCEnable", &m_bWPCEnable, 0, 1);
		m_bWPCEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nWPCThre", &m_nWPCThre, 0, 65536);
		m_nWPCThre = 5;//越小白点少
		m_nConfigParamList.ConfigParamListAddVariable("nWPCRatioT", &m_nWPCRatioT, 0, 65536);
		m_nWPCRatioT = 0;
		m_nConfigParamList.ConfigParamListAddVariable("bBPCEnable", &m_bBPCEnable, 0, 1);
		m_bBPCEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nBPCThre", &m_nBPCThre, 0, 65536);
		m_nBPCThre = 5;//越小黑点越少
		m_nConfigParamList.ConfigParamListAddVariable("nBPCRatioT", &m_nBPCRatioT, 0, 65536);
		m_nBPCRatioT = 0;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_DPCorrection");
	}
public:
	int m_bDumpFileEnable;
	int m_bWPCEnable;
	int m_nWPCThre;
	int m_nWPCRatioT;
	int m_bBPCEnable;
	int m_nBPCThre;
	int m_nBPCRatioT;
	CHDRPlus_DPCorrection()
	{
		Initialize();
	}
	void ProcessLine(unsigned short *pInLines[], unsigned short *pOutLine, int nWidth);
	bool Forward(MultiUshortImage *pInImage, MultiUshortImage *pOutImage, TGlobalControl *pControl);
};

#endif
