#include "HDRPlus_Demosaicing.h"
bool CHDRPlus_Demosaicing::Forward(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl *pControl)
{
	m_nMin = pControl->nBLC;
	m_nMax = pControl->nWP;
	int nWidth = pInRAWImage->GetImageWidth();
	int nHeight = pInRAWImage->GetImageHeight();
	if (pOutRGBImage->GetImageWidth() != nWidth || pOutRGBImage->GetImageHeight() != nHeight)
	{
		if (!pOutRGBImage->CreateImage(nWidth, nHeight, 3, 16))return false;
	}
	const int padx = 2;
	const int pady = 2;
	MultiUshortImage pInRAWImageExtend2;
	pInRAWImage->FillImageAround(&pInRAWImageExtend2, padx, pady);
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y += 2)
	{
		int realplaceY = y + pady;
		unsigned short *pRGBline0 = pOutRGBImage->GetImageLine(y);
		unsigned short *pRGBline1 = pOutRGBImage->GetImageLine(y + 1);
		unsigned short *pRAWlineSub2 = pInRAWImageExtend2.GetImageLine(realplaceY - 2);
		unsigned short *pRAWlineSub1 = pInRAWImageExtend2.GetImageLine(realplaceY - 1);
		unsigned short *pRAWlineCurrent = pInRAWImageExtend2.GetImageLine(realplaceY);
		unsigned short *pRAWlineAdd1 = pInRAWImageExtend2.GetImageLine(realplaceY + 1);
		unsigned short *pRAWlineAdd2 = pInRAWImageExtend2.GetImageLine(realplaceY + 2);
		unsigned short *pRAWlineAdd3 = pInRAWImageExtend2.GetImageLine(realplaceY + 3);
		pRAWlineSub2 += padx;
		pRAWlineSub1 += padx;
		pRAWlineCurrent += padx;
		pRAWlineAdd1 += padx;
		pRAWlineAdd2 += padx;
		pRAWlineAdd3 += padx;
		int RGB0[6] = { 0,0,0,0,0,0 };
		int RGB1[6] = { 0,0,0,0,0,0 };
		for (int x = 0; x < nWidth; x += 2)
		{
			int x1 = x + 1;
			switch (pControl->nCFAPattern)
			{
			case BGGR:
				//R--at--B
				//G--at--B
				//B--at--B
				//rgb
				RGB0[0] = (pRAWlineCurrent[x] * 12 + (pRAWlineSub1[x - 1] + pRAWlineSub1[x + 1] + pRAWlineAdd1[x + 1] + pRAWlineAdd1[x - 1]) * 4 - (pRAWlineAdd2[x] + pRAWlineSub2[x] + pRAWlineCurrent[x + 2] + pRAWlineCurrent[x - 2]) * 3) >> 4;
				RGB0[1] = (pRAWlineCurrent[x] * 4 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineSub1[x] + pRAWlineAdd1[x]) * 2 - pRAWlineCurrent[x - 2] - pRAWlineCurrent[x + 2] - pRAWlineSub2[x] - pRAWlineAdd2[x]) >> 3;
				RGB0[2] = pRAWlineCurrent[x];
				//R--at--G
				//G--at--G
				//B--at--G
				//rgb
				RGB0[3] = (pRAWlineCurrent[x1] * 10 + (pRAWlineAdd1[x1] + pRAWlineSub1[x1]) * 8 + pRAWlineCurrent[x1 - 2] + pRAWlineCurrent[x1 + 2] - (pRAWlineAdd1[x1 - 1] + pRAWlineAdd1[x1 + 1] + pRAWlineSub1[x1 - 1] + pRAWlineSub1[x1 + 1] + pRAWlineSub2[x1] + pRAWlineAdd2[x1]) * 2) >> 4;
				RGB0[4] = pRAWlineCurrent[x1];
				RGB0[5] = (pRAWlineCurrent[x1] * 10 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1]) * 8 + pRAWlineAdd2[x1] + pRAWlineSub2[x1] - (pRAWlineCurrent[x1 - 2] + pRAWlineCurrent[x1 + 2] + pRAWlineSub1[x1 + 1] + pRAWlineAdd1[x1 + 1] + pRAWlineSub1[x1 - 1] + pRAWlineAdd1[x1 - 1]) * 2) >> 4;
				//R--at--G
				//G--at--G
				//B--at--G		
				//rgb
				RGB1[0] = (pRAWlineAdd1[x] * 10 + (pRAWlineAdd1[x - 1] + pRAWlineAdd1[x + 1]) * 8 + pRAWlineSub1[x] + pRAWlineAdd3[x] - (pRAWlineAdd1[x - 2] + pRAWlineAdd1[x + 2] + pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x - 1] + pRAWlineAdd2[x + 1]) * 2) >> 4;
				RGB1[1] = pRAWlineAdd1[x];
				RGB1[2] = (pRAWlineAdd1[x] * 10 + (pRAWlineCurrent[x] + pRAWlineAdd2[x]) * 8 + pRAWlineAdd1[x + 2] + pRAWlineAdd1[x - 2] - (pRAWlineAdd3[x] + pRAWlineSub1[x] + pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x - 1] + pRAWlineAdd2[x + 1]) * 2) >> 4;
				//R--at--R
				//G--at--R
				//B--at--R
				RGB1[3] = pRAWlineAdd1[x1];
				RGB1[4] = (pRAWlineAdd1[x1] * 4 + (pRAWlineAdd1[x1 - 1] + pRAWlineAdd1[x1 + 1] + pRAWlineCurrent[x1] + pRAWlineAdd2[x1]) * 2 - pRAWlineAdd1[x1 - 2] - pRAWlineAdd1[x1 + 2] - pRAWlineSub1[x1] - pRAWlineAdd3[x1]) >> 3;
				RGB1[5] = (pRAWlineAdd1[x1] * 12 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 + 1] + pRAWlineAdd2[x1 - 1]) * 4 - (pRAWlineAdd3[x1] + pRAWlineSub1[x1] + pRAWlineAdd1[x1 + 2] + pRAWlineAdd1[x1 - 2]) * 3) >> 4;
				break;
			case GBRG:
				//R--at--G
				//G--at--G
				//B--at--G
				//rgb
				RGB0[0] = (pRAWlineCurrent[x] * 10 + (pRAWlineAdd1[x] + pRAWlineSub1[x]) * 8 + pRAWlineCurrent[x - 2] + pRAWlineCurrent[x + 2] - (pRAWlineAdd1[x - 1] + pRAWlineAdd1[x + 1] + pRAWlineSub1[x - 1] + pRAWlineSub1[x + 1] + pRAWlineSub2[x] + pRAWlineAdd2[x]) * 2) >> 4;
				RGB0[1] = pRAWlineCurrent[x];
				RGB0[2] = (pRAWlineCurrent[x] * 10 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1]) * 8 + pRAWlineAdd2[x] + pRAWlineSub2[x] - (pRAWlineCurrent[x - 2] + pRAWlineCurrent[x + 2] + pRAWlineSub1[x + 1] + pRAWlineAdd1[x + 1] + pRAWlineSub1[x - 1] + pRAWlineAdd1[x - 1]) * 2) >> 4;
				//R--at--B
				//G--at--B
				//B--at--B
				//rgb
				RGB0[3] = (pRAWlineCurrent[x1] * 12 + (pRAWlineSub1[x1 - 1] + pRAWlineSub1[x1 + 1] + pRAWlineAdd1[x1 + 1] + pRAWlineAdd1[x1 - 1]) * 4 - (pRAWlineAdd2[x1] + pRAWlineSub2[x1] + pRAWlineCurrent[x1 + 2] + pRAWlineCurrent[x1 - 2]) * 3) >> 4;
				RGB0[4] = (pRAWlineCurrent[x1] * 4 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineSub1[x1] + pRAWlineAdd1[x1]) * 2 - pRAWlineCurrent[x1 - 2] - pRAWlineCurrent[x1 + 2] - pRAWlineSub2[x1] - pRAWlineAdd2[x1]) >> 3;
				RGB0[5] = pRAWlineCurrent[x1];
				//R--at--R
				//G--at--R
				//B--at--R
				RGB1[0] = pRAWlineAdd1[x];
				RGB1[1] = (pRAWlineAdd1[x] * 4 + (pRAWlineAdd1[x - 1] + pRAWlineAdd1[x + 1] + pRAWlineCurrent[x] + pRAWlineAdd2[x]) * 2 - pRAWlineAdd1[x - 2] - pRAWlineAdd1[x + 2] - pRAWlineSub1[x] - pRAWlineAdd3[x]) >> 3;
				RGB1[2] = (pRAWlineAdd1[x] * 12 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x + 1] + pRAWlineAdd2[x - 1]) * 4 - (pRAWlineAdd3[x] + pRAWlineSub1[x] + pRAWlineAdd1[x + 2] + pRAWlineAdd1[x - 2]) * 3) >> 4;
				//R--at--G
				//G--at--G
				//B--at--G		
				//rgb
				RGB1[3] = (pRAWlineAdd1[x1] * 10 + (pRAWlineAdd1[x1 - 1] + pRAWlineAdd1[x1 + 1]) * 8 + pRAWlineSub1[x1] + pRAWlineAdd3[x1] - (pRAWlineAdd1[x1 - 2] + pRAWlineAdd1[x1 + 2] + pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 - 1] + pRAWlineAdd2[x1 + 1]) * 2) >> 4;
				RGB1[4] = pRAWlineAdd1[x1];
				RGB1[5] = (pRAWlineAdd1[x1] * 10 + (pRAWlineCurrent[x1] + pRAWlineAdd2[x1]) * 8 + pRAWlineAdd1[x1 + 2] + pRAWlineAdd1[x1 - 2] - (pRAWlineAdd3[x1] + pRAWlineSub1[x1] + pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 - 1] + pRAWlineAdd2[x1 + 1]) * 2) >> 4;
				break;
			case GRBG:
				//R--at--G
				//G--at--G
				//B--at--G
				RGB0[0] = (pRAWlineCurrent[x] * 10 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1]) * 8 + pRAWlineSub2[x] + pRAWlineAdd2[x] - (pRAWlineCurrent[x - 2] + pRAWlineCurrent[x + 2] + pRAWlineSub1[x + 1] + pRAWlineAdd1[x + 1] + pRAWlineSub1[x - 1] + pRAWlineAdd1[x - 1]) * 2) >> 4;
				RGB0[1] = pRAWlineCurrent[x];
				RGB0[2] = (pRAWlineCurrent[x] * 10 + (pRAWlineSub1[x] + pRAWlineAdd1[x]) * 8 + pRAWlineCurrent[x + 2] + pRAWlineCurrent[x - 2] - (pRAWlineAdd2[x] + pRAWlineSub2[x] + pRAWlineSub1[x + 1] + pRAWlineAdd1[x + 1] + pRAWlineSub1[x - 1] + pRAWlineAdd1[x - 1]) * 2) >> 4;
				//R--at--R
				//G--at--R
				//B--at--R
				RGB0[3] = pRAWlineCurrent[x1];
				RGB0[4] = (pRAWlineCurrent[x1] * 4 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineSub1[x1] + pRAWlineAdd1[x1]) * 2 - pRAWlineCurrent[x1 - 2] - pRAWlineCurrent[x1 + 2] - pRAWlineSub2[x1] - pRAWlineAdd2[x1]) >> 3;
				RGB0[5] = (pRAWlineCurrent[x1] * 12 + (pRAWlineSub1[x1 - 1] + pRAWlineSub1[x1 + 1] + pRAWlineAdd1[x1 + 1] + pRAWlineAdd1[x1 - 1]) * 4 - (pRAWlineAdd2[x1] + pRAWlineSub2[x1] + pRAWlineCurrent[x1 + 2] + pRAWlineCurrent[x1 - 2]) * 3) >> 4;
				//R--at--B
				//G--at--B
				//B--at--B
				RGB1[0] = (pRAWlineAdd1[x] * 12 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x + 1] + pRAWlineAdd2[x - 1]) * 4 - (pRAWlineAdd3[x] + pRAWlineSub1[x] + pRAWlineAdd1[x + 2] + pRAWlineAdd1[x - 2]) * 3) >> 4;
				RGB1[1] = (pRAWlineAdd1[x] * 4 + (pRAWlineAdd1[x - 1] + pRAWlineAdd1[x + 1] + pRAWlineCurrent[x] + pRAWlineAdd2[x]) * 2 - pRAWlineAdd1[x - 2] - pRAWlineAdd1[x + 2] - pRAWlineSub1[x] - pRAWlineAdd3[x]) >> 3;
				RGB1[2] = pRAWlineAdd1[x];
				//R--at--G
				//G--at--G
				//B--at--G
				//rgb
				RGB1[3] = (pRAWlineAdd1[x1] * 10 + (pRAWlineCurrent[x1] + pRAWlineAdd2[x1]) * 8 + pRAWlineAdd1[x1 + 2] + pRAWlineAdd1[x1 - 2] - (pRAWlineAdd3[x1] + pRAWlineSub1[x1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 + 1] + pRAWlineCurrent[x1 - 1] + pRAWlineAdd2[x1 - 1]) * 2) >> 4;
				RGB1[4] = pRAWlineAdd1[x1];
				RGB1[5] = (pRAWlineAdd1[x1] * 10 + (pRAWlineAdd1[x1 - 1] + pRAWlineAdd1[x1 + 1]) * 8 + pRAWlineSub1[x1] + pRAWlineAdd3[x1] - (pRAWlineAdd1[x1 - 2] + pRAWlineAdd1[x1 + 2] + pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 - 1] + pRAWlineAdd2[x1 + 1]) * 2) >> 4;
				break;
			case RGGB:
				//R--at--R
				//G--at--R
				//B--at--R
				RGB0[0] = pRAWlineCurrent[x];
				RGB0[1] = (pRAWlineCurrent[x] * 4 + (pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineSub1[x] + pRAWlineAdd1[x]) * 2 - pRAWlineCurrent[x - 2] - pRAWlineCurrent[x + 2] - pRAWlineSub2[x] - pRAWlineAdd2[x]) >> 3;
				RGB0[2] = (pRAWlineCurrent[x] * 12 + (pRAWlineSub1[x - 1] + pRAWlineSub1[x + 1] + pRAWlineAdd1[x + 1] + pRAWlineAdd1[x - 1]) * 4 - (pRAWlineAdd2[x] + pRAWlineSub2[x] + pRAWlineCurrent[x + 2] + pRAWlineCurrent[x - 2]) * 3) >> 4;
				//R--at--G
				//G--at--G
				//B--at--G
				//rgb
				RGB0[3] = (pRAWlineCurrent[x1] * 10 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1]) * 8 + pRAWlineSub2[x1] + pRAWlineAdd2[x1] - (pRAWlineCurrent[x1 - 2] + pRAWlineCurrent[x1 + 2] + pRAWlineSub1[x1 + 1] + pRAWlineAdd1[x1 + 1] + pRAWlineSub1[x1 - 1] + pRAWlineAdd1[x1 - 1]) * 2) >> 4;
				RGB0[4] = pRAWlineCurrent[x1];
				RGB0[5] = (pRAWlineCurrent[x1] * 10 + (pRAWlineSub1[x1] + pRAWlineAdd1[x1]) * 8 + pRAWlineCurrent[x1 + 2] + pRAWlineCurrent[x1 - 2] - (pRAWlineAdd2[x1] + pRAWlineSub2[x1] + pRAWlineSub1[x1 + 1] + pRAWlineAdd1[x1 + 1] + pRAWlineSub1[x1 - 1] + pRAWlineAdd1[x1 - 1]) * 2) >> 4;
				//R--at--G
				//G--at--G
				//B--at--G
				//rgb
				RGB1[0] = (pRAWlineAdd1[x] * 10 + (pRAWlineCurrent[x] + pRAWlineAdd2[x]) * 8 + pRAWlineAdd1[x + 2] + pRAWlineAdd1[x - 2] - (pRAWlineAdd3[x] + pRAWlineSub1[x] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x + 1] + pRAWlineCurrent[x - 1] + pRAWlineAdd2[x - 1]) * 2) >> 4;
				RGB1[1] = pRAWlineAdd1[x];
				RGB1[2] = (pRAWlineAdd1[x] * 10 + (pRAWlineAdd1[x - 1] + pRAWlineAdd1[x + 1]) * 8 + pRAWlineSub1[x] + pRAWlineAdd3[x] - (pRAWlineAdd1[x - 2] + pRAWlineAdd1[x + 2] + pRAWlineCurrent[x - 1] + pRAWlineCurrent[x + 1] + pRAWlineAdd2[x - 1] + pRAWlineAdd2[x + 1]) * 2) >> 4;
				//R--at--B
				//G--at--B
				//B--at--B
				//rgb
				RGB1[3] = (pRAWlineAdd1[x1] * 12 + (pRAWlineCurrent[x1 - 1] + pRAWlineCurrent[x1 + 1] + pRAWlineAdd2[x1 + 1] + pRAWlineAdd2[x1 - 1]) * 4 - (pRAWlineAdd3[x1] + pRAWlineSub1[x1] + pRAWlineAdd1[x1 + 2] + pRAWlineAdd1[x1 - 2]) * 3) >> 4;
				RGB1[4] = (pRAWlineAdd1[x1] * 4 + (pRAWlineAdd1[x1 - 1] + pRAWlineAdd1[x1 + 1] + pRAWlineCurrent[x1] + pRAWlineAdd2[x1]) * 2 - pRAWlineAdd1[x1 - 2] - pRAWlineAdd1[x1 + 2] - pRAWlineSub1[x1] - pRAWlineAdd3[x1]) >> 3;
				RGB1[5] = pRAWlineAdd1[x1];
				break;
			default:
				printf("Please enter correctly RawTYPE\n");
				break;
			}
			pRGBline0[0] = CLIP(RGB0[0], m_nMin, m_nMax);
			pRGBline0[1] = CLIP(RGB0[1], m_nMin, m_nMax);
			pRGBline0[2] = CLIP(RGB0[2], m_nMin, m_nMax);
			pRGBline0[3] = CLIP(RGB0[3], m_nMin, m_nMax);
			pRGBline0[4] = CLIP(RGB0[4], m_nMin, m_nMax);
			pRGBline0[5] = CLIP(RGB0[5], m_nMin, m_nMax);
			pRGBline1[0] = CLIP(RGB1[0], m_nMin, m_nMax);
			pRGBline1[1] = CLIP(RGB1[1], m_nMin, m_nMax);
			pRGBline1[2] = CLIP(RGB1[2], m_nMin, m_nMax);
			pRGBline1[3] = CLIP(RGB1[3], m_nMin, m_nMax);
			pRGBline1[4] = CLIP(RGB1[4], m_nMin, m_nMax);
			pRGBline1[5] = CLIP(RGB1[5], m_nMin, m_nMax);
			pRGBline0 += 6;
			pRGBline1 += 6;
		}
	}
	return true;
}
void CHDRPlus_Demosaicing::RAWToYUVH(unsigned short BlockRaw[5][5], int nCFA, int HVYUVH[6], int bYFlag, int bXFlag, int bHGreenFlag, int bVGreenFlag)
{
	int i, HYUVH[4], VYUVH[4];
	//int bHGreenFlag = (nCFA & 1) ^ bYFlag;
	//int bVGreenFlag = bXFlag ^ bYFlag;
	unsigned short VRAW[5];
	int pHGC[5][2];
	int pVGC[5][2];
	//行方向计算//R-G 或B-G
	Bayer2GC(BlockRaw[0], pHGC[0], bHGreenFlag);
	Bayer2GC(BlockRaw[1], pHGC[1], bHGreenFlag ^ 1);
	Bayer2GC(BlockRaw[2], pHGC[2], bHGreenFlag);
	Bayer2GC(BlockRaw[3], pHGC[3], bHGreenFlag ^ 1);
	Bayer2GC(BlockRaw[4], pHGC[4], bHGreenFlag);
	//列方向计算//R-G 或B-G
	VRAW[0] = BlockRaw[0][0];
	VRAW[1] = BlockRaw[1][0];
	VRAW[2] = BlockRaw[2][0];
	VRAW[3] = BlockRaw[3][0];
	VRAW[4] = BlockRaw[4][0];
	Bayer2GC(VRAW, pVGC[0], bVGreenFlag);
	VRAW[0] = BlockRaw[0][1];
	VRAW[1] = BlockRaw[1][1];
	VRAW[2] = BlockRaw[2][1];
	VRAW[3] = BlockRaw[3][1];
	VRAW[4] = BlockRaw[4][1];
	Bayer2GC(VRAW, pVGC[1], bVGreenFlag ^ 1);
	VRAW[0] = BlockRaw[0][2];
	VRAW[1] = BlockRaw[1][2];
	VRAW[2] = BlockRaw[2][2];
	VRAW[3] = BlockRaw[3][2];
	VRAW[4] = BlockRaw[4][2];
	Bayer2GC(VRAW, pVGC[2], bVGreenFlag);
	VRAW[0] = BlockRaw[0][3];
	VRAW[1] = BlockRaw[1][3];
	VRAW[2] = BlockRaw[2][3];
	VRAW[3] = BlockRaw[3][3];
	VRAW[4] = BlockRaw[4][3];
	Bayer2GC(VRAW, pVGC[3], bVGreenFlag ^ 1);
	VRAW[0] = BlockRaw[0][4];
	VRAW[1] = BlockRaw[1][4];
	VRAW[2] = BlockRaw[2][4];
	VRAW[3] = BlockRaw[3][4];
	VRAW[4] = BlockRaw[4][4];
	Bayer2GC(VRAW, pVGC[4], bVGreenFlag);

	GCFilter51(pHGC, HYUVH, bYFlag);
	GCFilter51(pVGC, VYUVH, bXFlag);
	/////////////////////
	HVYUVH[0] = (HYUVH[0] + VYUVH[0]) / 2;
	HVYUVH[1] = (HYUVH[1] + VYUVH[1]) / 2 + 32768;
	HVYUVH[2] = HYUVH[2] + 32768;
	HVYUVH[3] = VYUVH[2] + 32768;
	HVYUVH[4] = (HYUVH[0] + HYUVH[3] - HVYUVH[0]) + 32768;
	HVYUVH[5] = (VYUVH[0] + VYUVH[3] - HVYUVH[0]) + 32768;
	if (HVYUVH[0] < 0)HVYUVH[0] = 0;	if (HVYUVH[0] > m_nMax)HVYUVH[0] = m_nMax;
	for (i = 1; i < 6; i++)
	{
		if (HVYUVH[i] < 0)HVYUVH[i] = 0;	if (HVYUVH[i] > 65535)HVYUVH[i] = 65535;
	}
}
bool CHDRPlus_Demosaicing::RawToHVYUVHImage(MultiUshortImage* pInImage, MultiUshortImage* pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->SetImageSize(nWidth, nHeight, 6))return false;
	int bYFlag = (m_nCFAPattern >> 1) & 1;
	const int WIN = 5;
	const int WINSub_1 = (WIN - 1);
	const int WINcenter = (WIN / 2);
	unsigned short rawPixel;
	int HVYUVH[6] = { 0,0,0,0,0,0 };
	unsigned short BlockRaw[WIN][WIN];
	unsigned short lineBuf[WINSub_1][6000];
	for (int n = 0; n < WIN; n++)
	{
		for (int m = 0; m < WIN; m++)
		{
			BlockRaw[n][m] = 0;
		}
	}
	for (int n = 0; n < WINSub_1; n++)
	{
		for (int m = 0; m < 6000; m++)
		{
			lineBuf[n][m] = 0;
		}
	}
	unsigned short* pRaw = pInImage->GetImageData();
	unsigned short* pOutYUVH = pOutImage->GetImageData();
	for (int y = 0; y < nHeight; y++)
	{
		int bHGreenFlag = (m_nCFAPattern & 1) ^ bYFlag;
		int bXFlag = (m_nCFAPattern & 1);
		for (int x = 0; x < nWidth; x++)
		{
			int bVGreenFlag = bXFlag ^ bYFlag;
			rawPixel = (int)(*pRaw++);
			for (int k = 0; k < WIN; k++)
			{
				for (int l = 0; l < WINSub_1; l++)
				{
					BlockRaw[k][l] = BlockRaw[k][l + 1];
				}
			}
			for (int k = 0; k < WINSub_1; k++)
			{
				BlockRaw[k][WINSub_1] = lineBuf[k][x];
			}
			BlockRaw[WINSub_1][WINSub_1] = rawPixel;
			for (int k = 0; k < WINSub_1; k++)
			{
				lineBuf[k][x] = BlockRaw[k + 1][WINSub_1];
			}
			if ((y > WINcenter + 1) && (x > WINcenter + 1))
			{
				RAWToYUVH(BlockRaw, m_nCFAPattern, HVYUVH, bYFlag, bXFlag, bHGreenFlag, bVGreenFlag);
			}
			else
			{
				RAWToYUVH(BlockRaw, m_nCFAPattern, HVYUVH, bYFlag, bXFlag, bHGreenFlag, bVGreenFlag);
			}
			if ((y > WINcenter) || ((y == WINcenter) && (x > WINcenter - 1)))
			{
				pOutYUVH[0] = ((unsigned short)HVYUVH[0]);
				pOutYUVH[1] = ((unsigned short)HVYUVH[1]);
				pOutYUVH[2] = ((unsigned short)HVYUVH[2]);
				pOutYUVH[3] = ((unsigned short)HVYUVH[3]);
				pOutYUVH[4] = ((unsigned short)HVYUVH[4]);
				pOutYUVH[5] = ((unsigned short)HVYUVH[5]);
				pOutYUVH += 6;
			}
			bHGreenFlag ^= 1;
			bXFlag ^= 1;
		}
		bYFlag ^= 1;
	}
	for (int x = 0; x < WINcenter; x++)
	{
		pOutYUVH[0] = ((unsigned short)HVYUVH[0]);
		pOutYUVH[1] = ((unsigned short)HVYUVH[1]);
		pOutYUVH[2] = ((unsigned short)HVYUVH[2]);
		pOutYUVH[3] = ((unsigned short)HVYUVH[3]);
		pOutYUVH[4] = ((unsigned short)HVYUVH[4]);
		pOutYUVH[5] = ((unsigned short)HVYUVH[5]);
		pOutYUVH += 6;
	}
	for (int y = 0; y < WINcenter; y++)
	{
		int bHGreenFlag = (m_nCFAPattern & 1) ^ bYFlag;
		int bXFlag = (m_nCFAPattern & 1);
		for (int x = 0; x < nWidth; x++)
		{
			int bVGreenFlag = bXFlag ^ bYFlag;
			for (int k = 0; k < WIN; k++)
			{
				for (int l = 0; l < WINSub_1; l++)
				{
					BlockRaw[k][l] = BlockRaw[k][l + 1];
				}
			}
			for (int k = 0; k < WINSub_1; k++)
			{
				BlockRaw[k][WINSub_1] = lineBuf[k][x];
			}
			//BlockRaw[WINSub_1][WINSub_1] = rawPixel;
			for (int k = 0; k < WINSub_1; k++)
			{
				lineBuf[k][x] = BlockRaw[k + 1][WINSub_1];
			}
			RAWToYUVH(BlockRaw, m_nCFAPattern, HVYUVH, bYFlag, bXFlag, bHGreenFlag, bVGreenFlag);
			pOutYUVH[0] = ((unsigned short)HVYUVH[0]);
			pOutYUVH[1] = ((unsigned short)HVYUVH[1]);
			pOutYUVH[2] = ((unsigned short)HVYUVH[2]);
			pOutYUVH[3] = ((unsigned short)HVYUVH[3]);
			pOutYUVH[4] = ((unsigned short)HVYUVH[4]);
			pOutYUVH[5] = ((unsigned short)HVYUVH[5]);
			pOutYUVH += 6;
			bHGreenFlag ^= 1;
			bXFlag ^= 1;
		}
		bYFlag ^= 1;
	}
	return true;
}

bool CHDRPlus_Demosaicing::Forward2(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl *pControl)
{
	MultiUshortImage HVYUVHImage;
	CImageData_UINT32 HVImage;
	MultiShortImage DirImage;
	MultiUshortImage YUVImage;
	m_nCFAPattern =  pControl->nCFAPattern;
	m_nMax = (1 << pControl->nBit) - 1;
	m_nMin = pControl->nBLC;
	if (!RawToHVYUVHImage(pInRAWImage, &HVYUVHImage))return false;
	if (!HVYUVHToHV3x3Image(&HVYUVHImage, &HVImage))return false;
	if (!GaussHV7x7Image(&HVImage))return false;
	if (!HVToDirImage(&HVImage, &DirImage, m_nDirThre))return false;
	if (!MergeHVImage(&HVYUVHImage, &DirImage, &YUVImage))return false;
	if (m_bDeleteMinMaxYEnable)
	{
		DeleteMinMaxYImage(&YUVImage);
	}
	if (!YUVToRGBImage(&YUVImage, pOutRGBImage))return false;
	return true;
}
bool CHDRPlus_Demosaicing::HVYUVHToHV3x3Image(MultiUshortImage* pInImage, CImageData_UINT32* pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->SetImageSize(nWidth, nHeight, 2))return false;
	const int WIN = 3;
	const int WINSub_1 = (WIN - 1);
	const int WINcenter = (WIN / 2);
	int InYU[2], InHVE[4], OUTHV[2];
	unsigned short BlockA[WIN][WIN];
	unsigned short BlockB[WIN][WIN];
	unsigned short BlockC[WIN][WIN];
	unsigned short BlockD[WIN][WIN];
	unsigned short lineBufA[WINSub_1][6000];
	unsigned short lineBufB[WINSub_1][6000];
	unsigned short lineBufC[WINSub_1][6000];
	unsigned short lineBufD[WINSub_1][6000];
	int nVDVBuf[3];
	int nVDHBuf[3];
	int nHDVBuf[3];
	int nHDHBuf[3];
	for (int n = 0; n < WIN; n++)
	{
		for (int m = 0; m < WIN; m++)
		{
			BlockA[n][m] = 0;
			BlockB[n][m] = 0;
			BlockC[n][m] = 0;
			BlockD[n][m] = 0;
		}
	}
	for (int n = 0; n < WINSub_1; n++)
	{
		for (int m = 0; m < 6000; m++)
		{
			lineBufA[n][m] = 0;
			lineBufB[n][m] = 0;
			lineBufC[n][m] = 0;
			lineBufD[n][m] = 0;
		}
	}
	unsigned short* pInData = pInImage->GetImageData();
	unsigned int* pOutData = pOutImage->GetImageData();
	for (int y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth; x++)
		{
			InYU[0] = pInData[0];
			InYU[1] = pInData[0];
			InHVE[0] = pInData[0];
			InHVE[1] = pInData[0];
			InHVE[2] = pInData[0];
			InHVE[3] = pInData[0];
			pInData += 6;
			for (int k = 0; k < WIN; k++)
			{
				for (int l = 0; l < WINSub_1; l++)
				{
					BlockA[k][l] = BlockA[k][l + 1];
					BlockB[k][l] = BlockB[k][l + 1];
					BlockC[k][l] = BlockC[k][l + 1];
					BlockD[k][l] = BlockD[k][l + 1];
				}
			}
			for (int k = 0; k < WINSub_1; k++)
			{
				BlockA[k][WINSub_1] = lineBufA[k][x];
				BlockB[k][WINSub_1] = lineBufB[k][x];
				BlockC[k][WINSub_1] = lineBufC[k][x];
				BlockD[k][WINSub_1] = lineBufD[k][x];
			}
			BlockA[WINSub_1][WINSub_1] = InHVE[0];
			BlockB[WINSub_1][WINSub_1] = InHVE[1];
			BlockC[WINSub_1][WINSub_1] = InHVE[2];
			BlockD[WINSub_1][WINSub_1] = InHVE[3];
			for (int k = 0; k < WINSub_1 - 1; k++)
			{
				lineBufA[k][x] = lineBufA[k + 1][x];
				lineBufB[k][x] = lineBufB[k + 1][x];
				lineBufC[k][x] = lineBufC[k + 1][x];
				lineBufD[k][x] = lineBufD[k + 1][x];
			}
			lineBufA[WINSub_1 - 1][x] = InHVE[0];
			lineBufB[WINSub_1 - 1][x] = InHVE[1];
			lineBufC[WINSub_1 - 1][x] = InHVE[2];
			lineBufD[WINSub_1 - 1][x] = InHVE[3];
			if ((y > WINcenter + 1) && (x > WINcenter + 1))
			{
				//主方向
				nVDVBuf[0] = Pos(DIFF(BlockA[0][0], BlockA[2][0]) - DIFF(BlockB[0][0], BlockB[2][0]));
				nVDVBuf[1] = Pos(DIFF(BlockA[0][1], BlockA[2][1]) - DIFF(BlockB[0][1], BlockB[2][1]));
				nVDVBuf[2] = Pos(DIFF(BlockA[0][2], BlockA[2][2]) - DIFF(BlockB[0][2], BlockB[2][2]));
				//次方向
				nVDHBuf[0] = Pos(DIFF(BlockC[0][0], BlockC[2][0]) - DIFF(BlockD[0][0], BlockD[2][0]));
				nVDHBuf[1] = Pos(DIFF(BlockC[0][1], BlockC[2][1]) - DIFF(BlockD[0][1], BlockD[2][1]));
				nVDHBuf[2] = Pos(DIFF(BlockC[0][2], BlockC[2][2]) - DIFF(BlockD[0][2], BlockD[2][2]));
				//主方向
				nHDVBuf[0] = Pos(DIFF(BlockA[0][0], BlockA[0][2]) - DIFF(BlockB[0][0], BlockB[0][2]));
				nHDVBuf[1] = Pos(DIFF(BlockA[1][0], BlockA[1][2]) - DIFF(BlockB[1][0], BlockB[1][2]));
				nHDVBuf[2] = Pos(DIFF(BlockA[2][0], BlockA[2][2]) - DIFF(BlockB[2][0], BlockB[2][2]));
				//次方向
				nHDHBuf[0] = Pos(DIFF(BlockC[0][0], BlockC[0][2]) - DIFF(BlockD[0][0], BlockD[0][2]));
				nHDHBuf[1] = Pos(DIFF(BlockC[1][0], BlockC[1][2]) - DIFF(BlockD[1][0], BlockD[1][2]));
				nHDHBuf[2] = Pos(DIFF(BlockC[2][0], BlockC[2][2]) - DIFF(BlockD[2][0], BlockD[2][2]));


				OUTHV[0] = (nVDVBuf[0] + nVDVBuf[1] * 2 + nVDVBuf[2]);
				OUTHV[0] += (nVDHBuf[0] + nVDHBuf[1] * 2 + nVDHBuf[2]) >> 1;
				OUTHV[1] = (nHDVBuf[0] + nHDVBuf[1] * 2 + nHDVBuf[2]);
				OUTHV[1] += (nHDHBuf[0] + nHDHBuf[1] * 2 + nHDHBuf[2]) >> 1;
			}
			else
			{
				OUTHV[0] = 0;
				OUTHV[1] = 0;
			}
			if ((y > WINcenter) || ((y == WINcenter) && (x > WINcenter - 1)))
			{
				pOutData[0] = OUTHV[0];
				pOutData[1] = OUTHV[1];
			}
		}
	}
	for (int cnt = 0; cnt < WINcenter * nWidth + WINcenter; cnt++)
	{
		pOutData[0] = OUTHV[0];
		pOutData[1] = OUTHV[0];
	}
	return true;
}
bool CHDRPlus_Demosaicing::HVToDirImage(CImageData_UINT32 *pInImage, MultiShortImage *pOutImage, int nDirThre)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->CreateImage(nWidth, nHeight, 1))return false;
	#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned int *pIn = pInImage->GetImageLine(y);
		short *pOut = pOutImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			unsigned int hw = *(pIn++);
			unsigned int vw = *(pIn++);
			int conf = 16;
			int dw = DIFF(vw, hw);
			int flag = 0;
			int dir=0;
			if (dw > nDirThre)
			{
				if (dw <= 5 * nDirThre)
				{
					conf = ((dw - nDirThre) * 16) / (4 * nDirThre);
				}
				if (vw < hw)
				{
					unsigned int tmp = vw;
					vw = hw;
					hw = tmp;
					flag = 1;
				}
				if (vw > hw * 2)
					dir = 128;
				else
					dir = vw * 128 / hw - 128;
				dir = (dir*conf) >> 4;
				if (flag == 1)dir = -dir;
			}	
			*(pOut++) = (short)dir;
		}
	}
	return true;
}
bool CHDRPlus_Demosaicing::MergeHVImage(MultiUshortImage *pInImage, MultiShortImage *pDirImage, MultiUshortImage *pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->SetImageSize(nWidth, nHeight, 3))return false;
	#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		int HVYUVH[6], YUV[3];
		unsigned short *pIn = pInImage->GetImageLine(y);
		short *pDir = pDirImage->GetImageLine(y);
		unsigned short *pOut = pOutImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			for (int i = 0; i < 6; i++)
			{
				HVYUVH[i] = *(pIn++);
			}
			int dir = *(pDir++);
			YUV[0] = HVYUVH[0];
			YUV[1] = HVYUVH[1];
			///////////////////////MergeHV//////////////////
			YUV[2] = (HVYUVH[2] + HVYUVH[3]) / 2;
			if (dir >= 0)
			{
				YUV[2] += ((HVYUVH[3] - YUV[2])*dir) / 128;
				YUV[0] += ((HVYUVH[5] - 32768)*dir) / 128;
			}
			else
			{
				dir = -dir;
				YUV[2] += ((HVYUVH[2] - YUV[2])*dir) / 128;
				YUV[0] += ((HVYUVH[4] - 32768)*dir) / 128;
			}
			if (YUV[0] < 0)YUV[0] = 0;	if (YUV[0] > m_nMax)YUV[0] = m_nMax;
			if (YUV[2] < 0)YUV[2] = 0;	if (YUV[2] > 65535)YUV[2] = 65535;
			for (int i = 0; i < 3; i++)
			{
				*(pOut++) = (unsigned short)YUV[i];
			}
		}
	}
	return true;
}
void CHDRPlus_Demosaicing::DeleteMinMaxYImage(MultiUshortImage *pYUVImage)
{
	int nWidth = pYUVImage->GetImageWidth();
	int nHeight = pYUVImage->GetImageHeight();
	int nChannel = pYUVImage->GetImageDim();
	#pragma omp parallel for 
	for (int y = 1; y < nHeight - 1; y++)
	{
		unsigned short *pInLines[3];
		pInLines[0] = pYUVImage->GetImageLine(y - 1) + nChannel;
		pInLines[1] = pYUVImage->GetImageLine(y) + nChannel;
		pInLines[2] = pYUVImage->GetImageLine(y + 1) + nChannel;
		for (int x = 1; x < nWidth - 1; x++)
		{
			unsigned short Y0 = pInLines[1][0];
			unsigned short minY = 65535;
			unsigned short maxY = 0;
			for (int i = 0; i < 3; i++)
			{
				for (int j = -nChannel; j <= nChannel; j += nChannel)
				{
					if (i == 1 && j == 0)continue;
					if (minY > pInLines[i][j])
					{
						minY = pInLines[i][j];
					}
					if (maxY < pInLines[i][j])
					{
						maxY = pInLines[i][j];
					}
				}
			}
			if (Y0 < minY)
			{
				pInLines[1][0] = minY;
			}
			else if (Y0 > maxY)
			{
				pInLines[1][0] = maxY;
			}
			pInLines[0] += nChannel;
			pInLines[1] += nChannel;
			pInLines[2] += nChannel;
		}
	}
}
void CHDRPlus_Demosaicing::HYUVToRGBH3Line(unsigned short *pInLine, unsigned short *pOutLine, int nWidth)
{
	int i, x;
	unsigned int YUV[3];
	unsigned short *pIn[3];

	pIn[0] = pIn[1] = pInLine;
	pIn[2] = pIn[1] + 3;

	for (x = 0; x < nWidth - 1; x++)
	{
		for (i = 0; i < 3; i++)
		{
			YUV[i] = (unsigned int)pIn[0][i];
			YUV[i] += (unsigned int)pIn[1][i] * 2;
			YUV[i] += (unsigned int)pIn[2][i];
			YUV[i] >>= 2;
			pOutLine[i] = (unsigned short)YUV[i];
		}
		pOutLine[3] = pIn[1][0];
		pOutLine += 12;

		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] += 3;
	}
	pIn[2] = pIn[1];
	for (i = 0; i < 3; i++)
	{
		YUV[i] = (unsigned int)pIn[0][i];
		YUV[i] += (unsigned int)pIn[1][i] * 2;
		YUV[i] += (unsigned int)pIn[2][i];
		YUV[i] >>= 2;
		pOutLine[i] = (unsigned short)YUV[i];
	}
	pOutLine[3] = pIn[1][0];
	pOutLine += 12;
}
void CHDRPlus_Demosaicing::VYUVToRGB3Line(unsigned short *pInLines[], unsigned short *pOutLine, int nWidth)
{
	int i, x, YUVH[4], BGRH[4];
	unsigned short *pIn[3];

	for (i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i];
	}
	for (x = 0; x < nWidth; x++)
	{
		for (i = 0; i < 3; i++)
		{
			YUVH[i] = (int)pIn[0][i];
			YUVH[i] += (int)pIn[1][i] * 2;
			YUVH[i] += (int)pIn[2][i];
			YUVH[i] = (YUVH[i] + 2) >> 2;
		}
		YUVH[3] = pIn[1][3];

		pIn[0] += 12;
		pIn[1] += 12;
		pIn[2] += 12;
		YUVH[1] -= 32768;
		YUVH[2] -= 32768;
		YUVH[3] -= YUVH[0];
		/////YUVH---------BGRH
		BGRH[0] = YUVH[0] + YUVH[1] + YUVH[2];
		BGRH[1] = YUVH[0] - YUVH[1];
		BGRH[2] = YUVH[0] + YUVH[1] - YUVH[2];
		BGRH[3] = YUVH[3];
		for (int i = 0; i < 3; i++)
		{
			if (BGRH[i] < 0)BGRH[i] = 0;
			if (BGRH[i] > m_nMax)BGRH[i] = m_nMax;
		}
		///////////////////
		for (int i = 0; i < 3; i++)
		{
			BGRH[i] += BGRH[3];
			if (BGRH[i] < 0)BGRH[i] = 0;
			if (BGRH[i] > m_nMax)BGRH[i] = m_nMax;
		}
		*(pOutLine++) = (unsigned short)BGRH[2];
		*(pOutLine++) = (unsigned short)BGRH[1];
		*(pOutLine++) = (unsigned short)BGRH[0];
	}
}
bool CHDRPlus_Demosaicing::YUVToRGBImage(MultiUshortImage *pInImage, MultiUshortImage *pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	int nDim = pInImage->GetImageDim();
	unsigned short *pHLines[3];
	if (!pOutImage->CreateImage(nWidth, nHeight, 3, 16))return false;
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[nWidth * 4 * 3* nProcs];//yuvh=4 pHLines[3]=3
	if (pBuffer == NULL)return false;
	int loop = 0;
	#pragma omp parallel for num_threads(nProcs)  firstprivate(loop) private(pHLines)
	for (int y = 0; y < nHeight; y++)
	{
		if (loop==0)
		{
			int nThreadId = omp_get_thread_num();
			for (int i = 0; i < 3; i++)
			{
				pHLines[i] = pBuffer + 4 * i+ nWidth * 4 * 3 * nThreadId;
			}
			HYUVToRGBH3Line(pInImage->GetImageLine(y-1), pHLines[0], nWidth);
			HYUVToRGBH3Line(pInImage->GetImageLine(y), pHLines[1], nWidth);
			loop++;
		}
		HYUVToRGBH3Line(pInImage->GetImageLine(y + 1), pHLines[2], nWidth);
		VYUVToRGB3Line(pHLines, pOutImage->GetImageLine(y), nWidth);
		unsigned short *pTemp = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pTemp;
	}
	delete[] pBuffer;
	return true;
}
void CHDRPlus_Demosaicing::HGaussHV7Line(unsigned int *pInLine, unsigned int *pOutLine, int nWidth)
{
	int i, x;
	unsigned int nOut[2];
	const int nMask[7] = { 1, 6, 15, 20, 15, 6, 1 };
	unsigned int *pIn[7];
	pIn[0] = pIn[1] = pIn[2] = pIn[3] = pInLine;
	pIn[4] = pIn[3] + 2;
	pIn[5] = pIn[4] + 2;
	for (x = 0; x < nWidth - 3; x++)
	{
		pIn[6] = pIn[5] + 2;

		nOut[0] = nOut[1] = 0;
		for (i = 0; i < 7; i++)
		{
			nOut[0] += pIn[i][0] * nMask[i];
			if (nOut[1] < pIn[i][1])nOut[1] = pIn[i][1];
		}
		nOut[0] >>= 4;
		pOutLine[0] = (unsigned short)nOut[0];
		pOutLine[1] = (unsigned short)nOut[1];
		pOutLine += 14;
		for (i = 0; i < 6; i++)
		{
			pIn[i] = pIn[i + 1];
		}
	}
	for (; x < nWidth; x++)
	{
		nOut[0] = nOut[1] = 0;
		for (i = 0; i < 7; i++)
		{
			nOut[0] += pIn[i][0] * nMask[i];
			if (nOut[1] < pIn[i][1])nOut[1] = pIn[i][1];
		}
		nOut[0] >>= 4;
		pOutLine[0] = (unsigned short)nOut[0];
		pOutLine[1] = (unsigned short)nOut[1];
		pOutLine += 14;
		for (i = 0; i < 6; i++)
		{
			pIn[i] = pIn[i + 1];
		}
	}
}
void CHDRPlus_Demosaicing::VGaussHV7Line(unsigned int *pInLines[], unsigned int *pOutLine, int nWidth)
{
	int i, x;
	unsigned int nOut[2];
	const unsigned int nMask[7] = { 1, 6, 15, 20, 15, 6, 1 };
	unsigned int *pIn[7];
	for (i = 0; i < 7; i++)
	{
		pIn[i] = pInLines[i];
	}
	for (x = 0; x < nWidth; x++)
	{
		nOut[0] = nOut[1] = 0;
		for (i = 0; i < 7; i++)
		{
			if (nOut[0] < pIn[i][0])nOut[0] = pIn[i][0];
			nOut[1] += pIn[i][1] * nMask[i];
			pIn[i] += 14;
		}
		nOut[1] >>= 4;
		*(pOutLine++) = nOut[0];
		*(pOutLine++) = nOut[1];
	}
}
bool CHDRPlus_Demosaicing::GaussHV7x7Image(CImageData_UINT32 *pHVImage)
{
	int nWidth = pHVImage->GetImageWidth();
	int nHeight = pHVImage->GetImageHeight();
	int nPitch = nWidth * 2;
	unsigned int *pHLines[7];
	int nProcs = omp_get_num_procs();
	unsigned int *pBuffer = new unsigned int[nPitch * 7* nProcs];
	if (pBuffer == NULL)return false;
	int loop = 0;
	#pragma omp parallel for num_threads(nProcs) firstprivate(loop) private(pHLines)
	for (int y=0; y < nHeight; y++)
	{
		if (loop==0)
		{
			int nThreadId = omp_get_thread_num();
			for (int i = 0; i < 7; i++)
			{
				pHLines[i] = pBuffer + 2*i+ nPitch * 7*nThreadId;
			}
			HGaussHV7Line(pHVImage->GetImageLine(y - 3), pHLines[0], nWidth);
			HGaussHV7Line(pHVImage->GetImageLine(y - 2), pHLines[1], nWidth);
			HGaussHV7Line(pHVImage->GetImageLine(y - 1), pHLines[2], nWidth);
			HGaussHV7Line(pHVImage->GetImageLine(y + 0), pHLines[3], nWidth);
			HGaussHV7Line(pHVImage->GetImageLine(y + 1), pHLines[4], nWidth);
			HGaussHV7Line(pHVImage->GetImageLine(y + 2), pHLines[5], nWidth);
			loop++;
		}
		HGaussHV7Line(pHVImage->GetImageLine(y + 3), pHLines[6], nWidth);
		VGaussHV7Line(pHLines, pHVImage->GetImageLine(y), nWidth);
		unsigned int *pTemp = pHLines[0];
		for (int i = 0; i < 6; i++)
		{
			pHLines[i] = pHLines[i + 1];
		}
		pHLines[6] = pTemp;
	}
	delete[] pBuffer;
	return true;
}
