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
bool CHDRPlus_Demosaicing::Forward2(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl *pControl)
{
	MultiUshortImage HVYUVHImage;
	CImageData_UINT32 HVImage;
	MultiShortImage DirImage;
	MultiUshortImage YUVImage;
	m_nCFAPattern =  pControl->nCFAPattern;
	m_nMax =  pControl->nWP;
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
void CHDRPlus_Demosaicing::HRawToHVYUVHLine(unsigned short *pInLine, int *pHGCLine, int nWidth, int bYFlag)
{
	int i, x, CFA[5], tC;
	int bGreenFlag = (m_nCFAPattern & 1) ^ bYFlag;
	CFA[0] = CFA[2] = *(pInLine++);
	CFA[1] = CFA[3] = *(pInLine++);
	int *pGC = pHGCLine;
	for (x = 0; x < nWidth - 2; x++, bGreenFlag ^= 1)
	{
		CFA[4] = *(pInLine++);
		Raw2GC(CFA, pGC, bGreenFlag);
		pGC += 10;
		for (i = 0; i < 4; i++)
		{
			CFA[i] = CFA[i + 1];
		}
	}
	for (; x < nWidth; x++, bGreenFlag ^= 1)
	{
		CFA[4] = CFA[2];
		Raw2GC(CFA, pGC, bGreenFlag);
		pGC += 10;
	}
}
void CHDRPlus_Demosaicing::VRawToHVYUVHLine(unsigned short *pRawLines[], int *pHGCLines[], unsigned short *pOutLine, int nWidth, int bYFlag)
{
	int i, x, CFA[5], nVGCBuf[10], HYUVH[4], VYUVH[4], HVYUVH[6];
	int bXFlag = (m_nCFAPattern & 1);
	int bGreenFlag = bXFlag ^ bYFlag;
	unsigned short *pRaw[5];
	int *pHGC[5];
	int *pVGC[5];
	pVGC[0] = pVGC[2] = nVGCBuf;
	pVGC[1] = pVGC[3] = pVGC[2] + 2;
	pVGC[4] = pVGC[3] + 2;
	for (i = 0; i < 5; i++)
	{
		pHGC[i] = pHGCLines[i];
		pRaw[i] = pRawLines[i];
		CFA[i] = *(pRaw[i]++);
	}
	Raw2GC(CFA, pVGC[2], bGreenFlag);
	for (i = 0; i < 5; i++)
	{
		CFA[i] = *(pRaw[i]++);
	}
	Raw2GC(CFA, pVGC[3], bGreenFlag ^ 1);
	for (x = 0; x < 2; x++, bXFlag ^= 1)
	{
		for (i = 0; i < 5; i++)
		{
			CFA[i] = *(pRaw[i]++);
		}
		Raw2GC(CFA, pVGC[4], (bXFlag^bYFlag));
		GCFilter5(pHGC, HYUVH, bYFlag);
		for (i = 0; i < 5; i++)
		{
			pHGC[i] += 10;
		}
		GCFilter5(pVGC, VYUVH, bXFlag);
		HVYUVH[0] = (HYUVH[0] + VYUVH[0]) / 2;
		HVYUVH[1] = (HYUVH[1] + VYUVH[1]) / 2 + 32768;
		HVYUVH[2] = HYUVH[2] + 32768;
		HVYUVH[3] = VYUVH[2] + 32768;
		HVYUVH[4] = (HYUVH[0] + HYUVH[3] - HVYUVH[0]) + 32768;
		HVYUVH[5] = (VYUVH[0] + VYUVH[3] - HVYUVH[0]) + 32768;
		if (HVYUVH[0] < 0)HVYUVH[0] = 0;	if (HVYUVH[0] > m_nMax)HVYUVH[0] = m_nMax;
		*(pOutLine++) = (unsigned short)HVYUVH[0];
		for (i = 1; i < 6; i++)
		{
			if (HVYUVH[i] < 0)HVYUVH[i] = 0;	if (HVYUVH[i] > 65535)HVYUVH[i] = 65535;
			*(pOutLine++) = (unsigned short)HVYUVH[i];
		}
		for (i = 0; i < 4; i++)
		{
			pVGC[i] = pVGC[i + 1];
		}
		pVGC[4] += 2;
	}
	for (; x < nWidth - 2; x++, bXFlag ^= 1)
	{
		for (i = 0; i < 5; i++)
		{
			CFA[i] = *(pRaw[i]++);
		}
		Raw2GC(CFA, pVGC[4], (bXFlag^bYFlag));
		GCFilter5(pHGC, HYUVH, bYFlag);
		for (i = 0; i < 5; i++)
		{
			pHGC[i] += 10;
		}
		GCFilter5(pVGC, VYUVH, bXFlag);
		HVYUVH[0] = (HYUVH[0] + VYUVH[0]) / 2;
		HVYUVH[1] = (HYUVH[1] + VYUVH[1]) / 2 + 32768;
		HVYUVH[2] = HYUVH[2] + 32768;
		HVYUVH[3] = VYUVH[2] + 32768;
		HVYUVH[4] = (HYUVH[0] + HYUVH[3] - HVYUVH[0]) + 32768;
		HVYUVH[5] = (VYUVH[0] + VYUVH[3] - HVYUVH[0]) + 32768;
		if (HVYUVH[0] < 0)HVYUVH[0] = 0;	if (HVYUVH[0] > m_nMax)HVYUVH[0] = m_nMax;
		*(pOutLine++) = (unsigned short)HVYUVH[0];
		for (i = 1; i < 6; i++)
		{
			if (HVYUVH[i] < 0)HVYUVH[i] = 0;	if (HVYUVH[i] > 65535)HVYUVH[i] = 65535;
			*(pOutLine++) = (unsigned short)HVYUVH[i];
		}
		int *pTemp = pVGC[0];
		for (i = 0; i < 4; i++)
		{
			pVGC[i] = pVGC[i + 1];
		}
		pVGC[4] = pTemp;
	}
	for (; x < nWidth; x++, bXFlag ^= 1)
	{
		pVGC[4] = pVGC[2];
		GCFilter5(pHGC, HYUVH, bYFlag);
		for (i = 0; i < 5; i++)
		{
			pHGC[i] += 10;
		}
		GCFilter5(pVGC, VYUVH, bXFlag);
		HVYUVH[0] = (HYUVH[0] + VYUVH[0]) / 2;
		HVYUVH[1] = (HYUVH[1] + VYUVH[1]) / 2 + 32768;
		HVYUVH[2] = HYUVH[2] + 32768;
		HVYUVH[3] = VYUVH[2] + 32768;
		HVYUVH[4] = (HYUVH[0] + HYUVH[3] - HVYUVH[0]) + 32768;
		HVYUVH[5] = (VYUVH[0] + VYUVH[3] - HVYUVH[0]) + 32768;
		if (HVYUVH[0] < 0)HVYUVH[0] = 0;	if (HVYUVH[0] > m_nMax)HVYUVH[0] = m_nMax;
		*(pOutLine++) = (unsigned short)HVYUVH[0];
		for (i = 1; i < 6; i++)
		{
			if (HVYUVH[i] < 0)HVYUVH[i] = 0;	if (HVYUVH[i] > 65535)HVYUVH[i] = 65535;
			*(pOutLine++) = (unsigned short)HVYUVH[i];
		}
		for (i = 0; i < 4; i++)
		{
			pVGC[i] = pVGC[i + 1];
		}
	}
}
bool CHDRPlus_Demosaicing::RawToHVYUVHImage(MultiUshortImage *pInImage, MultiUshortImage *pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->SetImageSize(nWidth, nHeight,6))return false;
	int *pHGCLines[5];
	unsigned short *pRawLines[5];
	int nProcs = omp_get_num_procs();
	int *pHGCBuffer = new int[nWidth * 2 * 5* nProcs];
	if (pHGCBuffer == NULL)return false;
	int loop = 0;
	#pragma omp parallel for  num_threads(nProcs) firstprivate(loop) private(pHGCLines,pRawLines)
	for (int y=0; y < nHeight; y++)
	{
		int bYFlag = (m_nCFAPattern >> 1) & 1;
		if (y%2!=0)
		{
			bYFlag ^= 1;
		}
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			for (int i = 0; i < 5; i++)
			{
				pHGCLines[i] = pHGCBuffer + 2 * i + nWidth * 2 * 5 * nThreadId;
			}
			pRawLines[0] = pInImage->GetImageLine(y - 2);
			pRawLines[1] = pInImage->GetImageLine(y - 1);
			pRawLines[2] = pInImage->GetImageLine(y + 0);
			pRawLines[3] = pInImage->GetImageLine(y + 1);
			HRawToHVYUVHLine(pInImage->GetImageLine(y - 2), pHGCLines[0], nWidth, bYFlag);
			HRawToHVYUVHLine(pInImage->GetImageLine(y - 1), pHGCLines[1], nWidth, bYFlag ^ 1);
			HRawToHVYUVHLine(pInImage->GetImageLine(y + 0), pHGCLines[2], nWidth, bYFlag);
			HRawToHVYUVHLine(pInImage->GetImageLine(y + 1), pHGCLines[3], nWidth, bYFlag ^ 1);
			loop++;
		}
		pRawLines[4] = pInImage->GetImageLine(y + 2);
		HRawToHVYUVHLine(pInImage->GetImageLine(y + 2), pHGCLines[4], nWidth, bYFlag);
		VRawToHVYUVHLine(pRawLines, pHGCLines, pOutImage->GetImageLine(y), nWidth, bYFlag);
		int *pTemp = pHGCLines[0];
		for (int i = 0; i < 4; i++)
		{
			pRawLines[i] = pRawLines[i + 1];
			pHGCLines[i] = pHGCLines[i + 1];
		}
		pHGCLines[4] = pTemp;
	}
	delete[] pHGCBuffer;
	return true;
}
void CHDRPlus_Demosaicing::HVYUVHToHVBlock(int nOut[], int nHVBlock[3][3][4], int nVDVBuf[3], int nVDHBuf[3])
{
	int i, nHDVBuf[3], nHDHBuf[3];
	nVDVBuf[2] = Positive(DIFF(nHVBlock[0][2][1], nHVBlock[2][2][1]) - DIFF(nHVBlock[0][2][0], nHVBlock[2][2][0]));
	nVDHBuf[2] = Positive(DIFF(nHVBlock[0][2][3], nHVBlock[2][2][3]) - DIFF(nHVBlock[0][2][2], nHVBlock[2][2][2]));
	for (i = 0; i < 3; i++)
	{
		nHDVBuf[i] = Positive(DIFF(nHVBlock[i][0][0], nHVBlock[i][2][0]) - DIFF(nHVBlock[i][0][1], nHVBlock[i][2][1]));
		nHDHBuf[i] = Positive(DIFF(nHVBlock[i][0][2], nHVBlock[i][2][2]) - DIFF(nHVBlock[i][0][3], nHVBlock[i][2][3]));
	}
	nOut[0] = (nVDVBuf[0] + nVDVBuf[1] * 2 + nVDVBuf[2]);	//HW0=VDV
	nOut[0] += (nVDHBuf[0] + nVDHBuf[1] * 2 + nVDHBuf[2]) >> 1; //HW1=VDH
	nOut[1] = (nHDVBuf[0] + nHDVBuf[1] * 2 + nHDVBuf[2]);	//VW0=VDV
	nOut[1] += (nHDHBuf[0] + nHDHBuf[1] * 2 + nHDHBuf[2]) >> 1; //VW1=VDH
	nVDVBuf[0] = nVDVBuf[1];
	nVDHBuf[0] = nVDHBuf[1];
	nVDVBuf[1] = nVDVBuf[2];
	nVDHBuf[1] = nVDHBuf[2];
}
void CHDRPlus_Demosaicing::HVYUVHToHVLine(unsigned short *pInLines[], unsigned int *pOutLine, int nWidth)
{
	int i, k, x, HV[2];
	int nHVBlock[3][3][4];
	int nVDVBuf[3];
	int nVDHBuf[3];
	unsigned short *pIn[3];
	for (i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i];
		for (k = 0; k < 4; k++)
		{
			nHVBlock[i][1][k] = pIn[i][k + 2];
		}
		pIn[i] += 6;
		for (k = 0; k < 4; k++)
		{
			nHVBlock[i][0][k] = pIn[i][k + 2];
		}
	}
	for (int i = 0; i < 2; i++)
	{
		nVDVBuf[i] = Positive(DIFF(nHVBlock[0][i][1], nHVBlock[2][i][1]) - DIFF(nHVBlock[0][i][0], nHVBlock[2][i][0]));
		nVDHBuf[i] = Positive(DIFF(nHVBlock[0][i][3], nHVBlock[2][i][3]) - DIFF(nHVBlock[0][i][2], nHVBlock[2][i][2]));
	}
	for (x = 0; x < nWidth - 1; x++)
	{
		for (i = 0; i < 3; i++)
		{
			for (k = 0; k < 4; k++)
			{
				nHVBlock[i][2][k] = pIn[i][k + 2];
			}
			pIn[i] += 6;
		}
		HVYUVHToHVBlock(HV, nHVBlock, nVDVBuf, nVDHBuf);
		*(pOutLine++) = (unsigned int)HV[0];
		*(pOutLine++) = (unsigned int)HV[1];
		for (i = 0; i < 3; i++)
		{
			for (k = 0; k < 4; k++)
			{
				nHVBlock[i][0][k] = nHVBlock[i][1][k];
				nHVBlock[i][1][k] = nHVBlock[i][2][k];
			}
		}
	}
	{
		for (i = 0; i < 3; i++)
		{
			for (k = 0; k < 4; k++)
			{
				nHVBlock[i][2][k] = nHVBlock[i][0][k];
			}
		}
		HVYUVHToHVBlock(HV, nHVBlock, nVDVBuf, nVDHBuf);
		*(pOutLine++) = (unsigned int)HV[0];
		*(pOutLine++) = (unsigned int)HV[1];
	}
}
bool CHDRPlus_Demosaicing::HVYUVHToHV3x3Image(MultiUshortImage *pInImage, CImageData_UINT32 *pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!pOutImage->SetImageSize(nWidth, nHeight, 2))return false;
	unsigned short *pHLines[3];
	int loop =0;
	#pragma omp parallel for  firstprivate(loop) private(pHLines)
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			pHLines[0] = pInImage->GetImageLine(y - 1);
			pHLines[1] = pInImage->GetImageLine(y);
			loop++;
		}
		pHLines[2] = pInImage->GetImageLine(y+1);
		HVYUVHToHVLine(pHLines, pOutImage->GetImageLine(y), nWidth);
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
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
