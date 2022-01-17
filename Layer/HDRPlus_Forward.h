#ifndef __HDRPlus_Forward_H_
#define __HDRPlus_Forward_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
#include "HDRPlus_BlackWhiteLevel.h"
#include "HDRPlus_DPCorrection.h"
#include "HDRPlus_BlockMatchFusion.h"
#include "HDRPlus_ColorCorect.h"
#include "HDRPlus_Demosaicing.h"
#include "HDRPlus_Normalize.h"
#include "HDRPlus_WhiteBalance.h"
#include "HDRPlus_Tonemapping.h"
#include "HDRPlus_GammaCorrect.h"
#include "HDRPlus_ChromaDenoise.h"
#include "HDRPlus_Contrast.h"
#include "HDRPlus_Sharpen.h"
class CHDRPlus_Forward : public CMultiConfigFILE
{
protected:
	CHDRPlus_BlockMatchFusion m_HDRPlus_BlockMatchFusion;
	CHDRPlus_BlackWhiteLevel m_HDRPlus_BlackWhiteLevel;
	CHDRPlus_DPCorrection m_HDRPlus_DPCorrection;
	CHDRPlus_WhiteBalance m_HDRPlus_WhiteBalance;
	CHDRPlus_Demosaicing m_HDRPlus_Demosaicing;
	CHDRPlus_ColorCorect m_HDRPlus_ColorCorect;
	CHDRPlus_Tonemapping m_HDRPlus_Tonemapping;
	CHDRPlus_GammaCorrect m_HDRPlus_GammaCorrect;
	CHDRPlus_ChromaDenoise m_HDRPlus_ChromaDenoise;
	CHDRPlus_Contrast m_HDRPlus_Contrast;
	CHDRPlus_Sharpen m_HDRPlus_Sharpen;
	CHDRPlus_Normalize m_HDRPlus_Normalize;
	virtual void CreateConfigTitleNameList()
	{
		AddConfigTitle(&m_HDRPlus_BlockMatchFusion);
		AddConfigTitle(&m_HDRPlus_BlackWhiteLevel);
		AddConfigTitle(&m_HDRPlus_DPCorrection);
		AddConfigTitle(&m_HDRPlus_WhiteBalance);
		AddConfigTitle(&m_HDRPlus_Demosaicing);
		AddConfigTitle(&m_HDRPlus_ChromaDenoise);
		AddConfigTitle(&m_HDRPlus_ColorCorect);
		AddConfigTitle(&m_HDRPlus_Tonemapping);
		AddConfigTitle(&m_HDRPlus_GammaCorrect);
		AddConfigTitle(&m_HDRPlus_Contrast);
		AddConfigTitle(&m_HDRPlus_Sharpen);
		AddConfigTitle(&m_HDRPlus_Normalize);
	}
	virtual void CreateFilter()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Flow");
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Forward");
	}
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nBlockMatchFusionEnable", &m_nBlockMatchFusionEnable, 0, 1);
		m_nBlockMatchFusionEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nBlackWhiteLevelEnable", &m_nBlackWhiteLevelEnable, 0, 1);
		m_nBlackWhiteLevelEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nDPCorrectionEnable", &m_nDPCorrectionEnable, 0, 1);
		m_nDPCorrectionEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nWhiteBalanceEnable", &m_nWhiteBalanceEnable, 0, 1);
		m_nWhiteBalanceEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nDemosaicingEnable", &m_nDemosaicingEnable, 0, 1);
		m_nDemosaicingEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nChromaDenoiseEnable", &m_nChromaDenoiseEnable, 0, 1);
		m_nChromaDenoiseEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nColorCorectEnable", &m_nColorCorectEnable, 0, 1);
		m_nColorCorectEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nTonemappingEnable", &m_nTonemappingEnable, 0, 1);
		m_nTonemappingEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nGammaCorrectEnable", &m_nGammaCorrectEnable, 0, 1);
		m_nGammaCorrectEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nContrastEnable", &m_nContrastEnable, 0, 1);
		m_nContrastEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nSharpenEnable", &m_nSharpenEnable, 0, 1);
		m_nSharpenEnable = 1;
	}
public:
	int m_bDumpFileEnable;
	int m_nBlockMatchFusionEnable;
	int m_nBlackWhiteLevelEnable;
	int m_nDPCorrectionEnable;
	int m_nWhiteBalanceEnable;
	int m_nDemosaicingEnable;
	int m_nChromaDenoiseEnable;
	int m_nColorCorectEnable;
	int m_nTonemappingEnable;
	int m_nGammaCorrectEnable;
	int m_nContrastEnable;
	int m_nSharpenEnable;
	CHDRPlus_Forward()
	{
		Initialize();
	}
	void Forward(MultiUshortImage *InputRawData, MultiUcharImage *pOutRGBData, TGlobalControl *pControl);
};
#endif