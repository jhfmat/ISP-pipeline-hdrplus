#ifndef __HDRPlus_BlockMatchFusion_H_
#define __HDRPlus_BlockMatchFusion_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/Mat.h"
class CHDRPlus_BlockMatchFusion : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nManualAmount", &m_nManualAmount, 1, 100000,16);
		m_nManualAmount = 16;
		m_nConfigParamList.ConfigParamListAddVariable("bAutoAmount", &m_bAutoAmount, 0, 1);
		m_bAutoAmount = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nGainOption", &m_nGainOption, 0, 2);
		m_nGainOption = 0;
		m_nConfigParamList.ConfigParamListAddArray("nGainList", m_nGainList, 3, 0, 8192,16);
		m_nGainList[0] = 32;
		m_nGainList[1] = 64;
		m_nGainList[2] = 128;
		m_nConfigParamList.ConfigParamListAddArray("nAmountGainList", m_nAmountGainList, 3, 0, 4096,16);
		m_nAmountGainList[0] = 16;
		m_nAmountGainList[1] = 32;
		m_nAmountGainList[2] = 48;

		m_nConfigParamList.ConfigParamListAddVariable("nAmountFactor", &m_nAmountFactor, 0, 10000);
		m_nAmountFactor = 4;
		m_nConfigParamList.ConfigParamListAddVariable("nMinDist", &m_nMinDist, 0, 10000);
		m_nMinDist = 10;
		m_nConfigParamList.ConfigParamListAddVariable("nMaxDist", &m_nMaxDist, 0, 10000);
		m_nMaxDist = 300;
		m_nConfigParamList.ConfigParamListAddArray("nOffsetxLevel", m_nOffsetxLevel, 4, 0, 1000);
		m_nOffsetxLevel[0] = 4;
		for (int k = 1; k < 4; k++)
		{
			m_nOffsetxLevel[k] = 2;
		}
		m_nConfigParamList.ConfigParamListAddArray("nOffsetyLevel", m_nOffsetyLevel, 4, 0, 1000);
		m_nOffsetyLevel[0] = 4;
		for (int k = 1; k < 4; k++)
		{
			m_nOffsetyLevel[k] = 2;
		}
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_BlockMatchFusion");
	}
public:
	CHDRPlus_BlockMatchFusion()
	{
		Initialize();
	}
	int m_bDumpFileEnable;
	int m_nManualAmount;
	int m_bAutoAmount;
	int m_nGainOption;
	int m_nGainList[3];
	int m_nAmountGainList[3];
	int m_nAmountFactor;
	int m_nMinDist;
	int m_nMaxDist;
	int m_nOffsetxLevel[4];
	int m_nOffsetyLevel[4];
	void MergeWeight(MultiUshortImage * pOutMergeSingleImage, CImageData_UINT32 * pInMergeMultiImage, unsigned int Max);
	bool EstimatedOffsetNoRef(MultiUshortImage * pInRefImage, MultiUshortImage * pInDebugImage, MultiShortImage * pOutOffsetxImage, MultiShortImage * pOutOffsetyImage, int nMoveRangex, int nMoveRangey);
	bool EstimatedOffsetAndRef(MultiUshortImage * pInRefImage, MultiUshortImage * pInDebugImage, MultiShortImage * pPreOffsetxImage, MultiShortImage * pPreOffsetyImage, MultiShortImage * pOutOffsetxImage, MultiShortImage * pOutOffsetyImage, int nMoveRangex, int nMoveRangey);
	bool EstimatedWeight(MultiUshortImage * pInImage, int nFrame, MultiShortImage * pPreOffsetxImage, MultiShortImage * pPreOffsetyImage, CImage_FLOAT * pOutWeightImage);
	void MergeTemporal(MultiUshortImage * pRawPadImage, int nFrame, MultiShortImage * pOffsetxImage, MultiShortImage * pOffsetyImage, CImage_FLOAT * pInWeightImage, CImageData_UINT32 * pMergeImage);
	void FillUnsignedShortImage(unsigned short * pInImage, unsigned short * pOutImage, int nx, int ny, int padx, int pady);
	bool BoxDownx2(MultiUshortImage * pInImage, MultiUshortImage * pOutImage);
	bool UpScaleOffsetAndValuex2(MultiShortImage * InImage, MultiShortImage * pOutImage);
	void Forward(MultiUshortImage * pInImages, int nFrameID[], int Framenum,TGlobalControl *pControl);
};
#endif
