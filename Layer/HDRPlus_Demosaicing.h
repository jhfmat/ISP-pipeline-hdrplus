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
		m_nConfigParamList.ConfigParamListAddVariable("nDirThre", &m_nDirThre, 0, 4095);
		m_nDirThre = 0;
		m_nConfigParamList.ConfigParamListAddVariable("bDeleteMinMaxYEnable", &m_bDeleteMinMaxYEnable, 0, 1);
		m_bDeleteMinMaxYEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nGbGrThre", &m_nGbGrThre, 0, 4095);
		m_nGbGrThre = 16;
		m_nConfigParamList.ConfigParamListAddVariable("nGbGrSlope", &m_nGbGrSlope, 0, 1024);
		m_nGbGrSlope = 64;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Demosaicing");
	}
public:
	int m_bDumpFileEnable;
	int m_nCFAPattern;
	int m_nMin;
	int m_nMax;
	int m_nDirThre;
	int m_bDeleteMinMaxYEnable;
	int m_nGbGrThre;
	int m_nGbGrSlope;
	CHDRPlus_Demosaicing()
	{
		Initialize();
	}
	bool Forward(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl * pControl);
	bool Forward2(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl * pControl);
private:
	__inline int Pos(int d)
	{
		return (d >= 0) ? (d) : (0);
	}
	void GCFilter51(int GCBuf[5][2], int YUVH[4], int bFlag)
	{
		int CC[2];
		int nGbGrThre = m_nGbGrThre;
		YUVH[0] = (GCBuf[2][0] * 6 + (GCBuf[1][0] + GCBuf[3][0]) * 2 - GCBuf[0][0] - GCBuf[4][0]) / 8;//增强边缘
		YUVH[3] = GCBuf[2][0] - YUVH[0];//Edge信号
		if (YUVH[0] > m_nMin)
		{
			nGbGrThre += ((YUVH[0] - m_nMin) * m_nGbGrSlope) / 1024;
		}
		YUVH[3] = SoftThre(YUVH[3], nGbGrThre);//去噪Edge较小的信号噪声
		CC[bFlag] = (GCBuf[0][1] + GCBuf[2][1] * 6 + GCBuf[4][1]) / 8;//偶数同一个通道滤波
		CC[bFlag ^ 1] = (GCBuf[1][1] + GCBuf[3][1]) / 2;//基数同一个通道滤波
		YUVH[1] = (CC[0] + CC[1]) / 2;	//U=(B-2G+R)/2;
		YUVH[2] = (CC[0] - CC[1]);	//V=(B-R)/2;
		YUVH[0] += YUVH[1];
	}
	void Bayer2GC(unsigned short CFA[5], int GC[2], int bGreenFlag)
	{
		int tC = (CFA[1] + CFA[3]) / 2 + (CFA[2] * 2 - CFA[0] - CFA[4]) / 4;
		if (CFA[1] < CFA[3])
		{
			if (tC < CFA[1])
				tC = CFA[1];
			else if (CFA[3] < tC)
				tC = CFA[3];
		}
		else
		{
			if (CFA[1] < tC)
				tC = CFA[1];
			else if (tC < CFA[3])
				tC = CFA[3];
		}
		GC[bGreenFlag] = tC;
		GC[bGreenFlag ^ 1] = CFA[2];
		GC[1] -= GC[0];//R-G 或B-G
		GC[1] /= 2;
	}
	void RAWToYUVH(unsigned short BlockRaw[5][5], int nCFA, int HVYUVH[6], int bYFlag, int bXFlag, int bHGreenFlag, int bVGreenFlag);
	bool RawToHVYUVHImage(MultiUshortImage* pInImage, MultiUshortImage* pOutImage);
	bool HVYUVHToHV3x3Image(MultiUshortImage* pInImage, CImageData_UINT32* pOutImage);
	bool HVToDirImage(CImageData_UINT32 * pInImage, MultiShortImage * pOutImage, int nDirThre);
	bool MergeHVImage(MultiUshortImage * pInImage, MultiShortImage * pDirImage, MultiUshortImage * pOutImage);
	void DeleteMinMaxYImage(MultiUshortImage * pYUVHImage);
	void HYUVToRGBH3Line(unsigned short *pInLine, unsigned short *pOutLine, int nWidth);
	void VYUVToRGB3Line(unsigned short * pInLines[], unsigned short * pOutLine, int nWidth);
	bool YUVToRGBImage(MultiUshortImage * pInImage, MultiUshortImage * pOutImage);
	void HGaussHV7Line(unsigned int * pInLine, unsigned int * pOutLine, int nWidth);
	void VGaussHV7Line(unsigned int * pInLines[], unsigned int * pOutLine, int nWidth);
	bool GaussHV7x7Image(CImageData_UINT32 * pHVWImage);
};
#endif
