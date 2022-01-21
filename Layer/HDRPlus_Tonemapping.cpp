#include "HDRPlus_Tonemapping.h"
bool CHDRPlus_Tonemapping::ConvertoGray(MultiUshortImage *pRGBImage, MultiUshortImage *pGrayImage)
{
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	if (pGrayImage->GetImageWidth() != nWidth || pGrayImage->GetImageHeight() != nHeight)
	{
		if (!pGrayImage->CreateImage(nWidth, nHeight, 1, 16))return false;
	}
	unsigned short *table = new unsigned short[m_nMax*3+1];
	for (int k = 0; k < m_nMax * 3 + 1; k++)
	{
		table[k] = (k + 2) / 3;
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRGBLine = pRGBImage->GetImageLine(y);
		unsigned short *pGrayLine = pGrayImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			//pGrayLine[0] = (pRGBLine[0] + (pRGBLine[1]<<1) + pRGBLine[2])>>2;//这种平均会引起高光微软过曝
			unsigned int tmp = (pRGBLine[0] + pRGBLine[1] + pRGBLine[2]);
			pGrayLine[0] = table[tmp];
			pGrayLine++;
			pRGBLine += 3;
		}
	}
	delete[]table;
}
bool CHDRPlus_Tonemapping::Brighten(MultiUshortImage *pInDarkImage, float gain, MultiUshortImage *pOutBrightImage)
{
	int nWidth = pInDarkImage->GetImageWidth();
	int nHeight = pInDarkImage->GetImageHeight();
	if (pOutBrightImage->GetImageWidth() != nWidth || pOutBrightImage->GetImageHeight() != nHeight)
	{
		if (!pOutBrightImage->CreateImage(nWidth, nHeight, 1, 16))return false;
	}
	unsigned short *table=new unsigned short[m_nMax+1];
	for (int k = 0; k < m_nMax+1; k++)
	{
		int tmp = k * gain;
		table[k] = CLIP(tmp, m_nMin, m_nMax);
	}
	int tmpWidth = nWidth / 4 * 4;
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInDarkLine = pInDarkImage->GetImageLine(y);
		unsigned short *pOutBrightLine = pOutBrightImage->GetImageLine(y);
		int x = 0;
		for (; x < tmpWidth; x += 4)
		{
			pOutBrightLine[0] = table[pInDarkLine[0]];
			pOutBrightLine[1] = table[pInDarkLine[1]];
			pOutBrightLine[2] = table[pInDarkLine[2]];
			pOutBrightLine[3] = table[pInDarkLine[3]];
			pOutBrightLine += 4;
			pInDarkLine += 4;
		}
		for (; x < nWidth; x++)
		{
			pOutBrightLine[0] = table[pInDarkLine[0]];
			pOutBrightLine++;
			pInDarkLine++;
		}
	}
	delete[]table;
	return true;
}
bool CHDRPlus_Tonemapping::GrayGammaCorrect(MultiUshortImage *pInGrayImage, MultiUshortImage *pOutGammaImage)
{
	int nWidth = pInGrayImage->GetImageWidth();
	int nHeight = pInGrayImage->GetImageHeight();
	if (pOutGammaImage->GetImageWidth() != nWidth || pOutGammaImage->GetImageHeight() != nHeight)
	{
		if (!pOutGammaImage->CreateImage(nWidth, nHeight, 1, 16))return false;
	}
	int cutoff = 200;                   // ceil(0.00304 * UINT16_MAX)
	float gamma_toe = 12.92;
	float gamma_pow = 0.416667;         // 1 / 2.4
	float gamma_fac = 680.552897;       // 1.055 * UINT16_MAX ^ (1 - gamma_pow);
	float gamma_con = -3604.425;        // -0.055 * UINT16_MAX
	unsigned short *table=new unsigned short[m_nMax+1];
	for (int k = 0; k < m_nMax+1; k++)
	{
		long long int tmp;
		if (k < cutoff)
		{
			tmp = gamma_toe * k;
		}
		else
		{
			tmp = gamma_fac * pow(k, gamma_pow) + gamma_con;
		}
		table[k] = CLIP(tmp, m_nMin, m_nMax);
	}
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInline = pInGrayImage->GetImageLine(y);
		unsigned short *pOutline = pOutGammaImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pOutline[0] = table[pInline[0]];
			pOutline++;
			pInline++;
		}
	}
	delete[]table;
	return true;
}
bool CHDRPlus_Tonemapping::GammaInverse(MultiUshortImage *pInGrayImage, MultiUshortImage *pOutInverseImage)
{
	int nWidth = pInGrayImage->GetImageWidth();
	int nHeight = pInGrayImage->GetImageHeight();
	if (pOutInverseImage->GetImageWidth() != nWidth || pOutInverseImage->GetImageHeight() != nHeight)
	{
		if (!pOutInverseImage->CreateImage(nWidth, nHeight, 1, 16))return false;
	}
	unsigned int cutoff = 2575;                   // ceil(1/0.00304 * UINT16_MAX)
	float gamma_toe = 0.0774f;            // 1 / 12.92
	float gamma_pow = 2.4f;
	float gamma_fac = 57632.49226f;       // 1 / 1.055 ^ gamma_pow * U_INT16_MAX;
	float gamma_con = 0.055f;
	unsigned short *table=new unsigned short[m_nMax+1];
	for (int k = 0; k < m_nMax+1; k++)
	{
		long int tmp;
		if (k < cutoff)
		{
			tmp = gamma_toe * k;
		}
		else
		{
			tmp = (pow((float)k / (float)m_nMax + gamma_con, gamma_pow) * gamma_fac);
		}
		table[k] = CLIP(tmp, m_nMin, m_nMax);
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInline = pInGrayImage->GetImageLine(y);
		unsigned short *pOutline = pOutInverseImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pOutline[0] = table[pInline[0]];
			pOutline++;
			pInline++;
		}
	}
	delete[]table;
}
bool CHDRPlus_Tonemapping::BuildWeight(MultiUshortImage *pDarkGammaImage, MultiUshortImage *pBrightGammaImage, MultiUshortImage *DarkWeightImage, MultiUshortImage *BrightWeightImage, int ScaleBit)
{
	int ScaleValue = 1 << ScaleBit;
	int nWidth = pDarkGammaImage->GetImageWidth();
	int nHeight = pDarkGammaImage->GetImageHeight();
	int nDim = pDarkGammaImage->GetImageDim();
	if (DarkWeightImage->GetImageWidth() != nWidth || DarkWeightImage->GetImageHeight() != nHeight)
	{
		if (!DarkWeightImage->CreateImage(nWidth, nHeight, nDim, 16))return false;
	}
	if (BrightWeightImage->GetImageWidth() != nWidth || BrightWeightImage->GetImageHeight() != nHeight)
	{
		if (!BrightWeightImage->CreateImage(nWidth, nHeight, nDim, 16))return false;
	}
	float *WeightTable=new float[m_nMax+1];
	for (int k = 0; k < m_nMax+1; k++)
	{
		float darks = ((float)k / (float)m_nMax - 0.5f);
		WeightTable[k] = exp(-12.5f *(darks*darks));
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pDarkgammaline = pDarkGammaImage->GetImageLine(y);
		unsigned short *pBrightgammaline = pBrightGammaImage->GetImageLine(y);
		unsigned short *pDarkWeightline = DarkWeightImage->GetImageLine(y);
		unsigned short *pBrightWeightline = BrightWeightImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			float DarkCurve = WeightTable[pDarkgammaline[0]];//暗图曲线
			float BrightCurve = WeightTable[pBrightgammaline[0]];//亮图曲线
			float DarkWeight = (DarkCurve / (DarkCurve + BrightCurve));//暗图权重
			float BrightWeight = (1.f - DarkWeight);//亮图权重=BrightCurve / (DarkCurve + BrightCurve)
			pDarkWeightline[0] = DarkWeight * ScaleValue;
			pBrightWeightline[0] = BrightWeight * ScaleValue;
			pDarkgammaline++;
			pBrightgammaline++;
			pDarkWeightline++;
			pBrightWeightline++;
		}
	}
	delete[]WeightTable;
	return true;
}
bool CHDRPlus_Tonemapping::CombineDarkAndBrightImage(MultiUshortImage *pDarkGammaImage, MultiUshortImage *pBrightGammaImage, MultiUshortImage *pOutCombineImage)
{
	int ScaleBit = 12;
	int nWidth = pDarkGammaImage->GetImageWidth();
	int nHeight = pDarkGammaImage->GetImageHeight();
	int nDim = pDarkGammaImage->GetImageDim();
	MultiUshortImage DarkWeightImage, BrightWeightImage;
	MultiUshortImage DarkImagePyramid[12], BrightImagePyramid[12], DarkWeightImagePyramid[12], BrightWeightImagePyramid[12];
	MultiShortImage DarkImagePyramidEdge[12], BrightImagePyramidEdge[12];
	int nPyramidLevel = 12;
	pDarkGammaImage->GaussPyramidImage(DarkImagePyramid, DarkImagePyramidEdge, nPyramidLevel, true);
	pBrightGammaImage->GaussPyramidImage(BrightImagePyramid, BrightImagePyramidEdge, nPyramidLevel, true);
	BuildWeight(pDarkGammaImage, pBrightGammaImage, &DarkWeightImage, &BrightWeightImage, ScaleBit);
	DarkWeightImage.GaussPyramidImage(DarkWeightImagePyramid, NULL, nPyramidLevel, false);//最大值是4096
	BrightWeightImage.GaussPyramidImage(BrightWeightImagePyramid, NULL, nPyramidLevel, false);
	MultiUshortImage TempImage;
	DarkImagePyramid[nPyramidLevel].ApplyWeight(&DarkWeightImagePyramid[nPyramidLevel], ScaleBit);
	BrightImagePyramid[nPyramidLevel].ApplyWeight(&BrightWeightImagePyramid[nPyramidLevel], ScaleBit);//已经除了4096
	DarkImagePyramid[nPyramidLevel].AddImage(&BrightImagePyramid[nPyramidLevel]);
	pOutCombineImage->Clone(&DarkImagePyramid[nPyramidLevel]);
	for (int i = nPyramidLevel - 1; i >= 0; i--)
	{
		if (!pOutCombineImage->UpScaleImagex2(&TempImage, false))return false;
		if (!pOutCombineImage->CopyImageRect(&TempImage, 0, 0, DarkImagePyramidEdge[i].GetImageWidth(), DarkImagePyramidEdge[i].GetImageHeight()))return false;
		DarkImagePyramidEdge[i].ApplyWeight(&DarkWeightImagePyramid[i], ScaleBit);
		BrightImagePyramidEdge[i].ApplyWeight(&BrightWeightImagePyramid[i], ScaleBit);
		DarkImagePyramidEdge[i].AddImage(&BrightImagePyramidEdge[i]);
		pOutCombineImage->AddImage(&DarkImagePyramidEdge[i]);
	}
	return true;
}
void CHDRPlus_Tonemapping::GammaCombinRGB(MultiUshortImage *pRGBImage, MultiUshortImage *pGrayImage, MultiUshortImage *pDarkImage)
{
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInRGBLine = pRGBImage->GetImageLine(y);
		unsigned short *pInGrayLine = pGrayImage->GetImageLine(y);
		unsigned short *pInDarkLine = pDarkImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			unsigned int R, G, B, Gain;
			unsigned short Max = *pInGrayLine++;
			Gain = pInDarkLine[0];
			if (Max == 0)
			{
				Max = 1;
			}
			if (Gain == 0)
			{
				Gain = 1;
			}
			Gain = Gain << 12;
			Gain = Gain / Max;
			if (Gain > 4096)
			{
				R = pInRGBLine[0] * Gain;
				G = pInRGBLine[1] * Gain;
				B = pInRGBLine[2] * Gain;
				R >>= 12;
				G >>= 12;
				B >>= 12;
				pInRGBLine[0] = CLIP(R, m_nMin, m_nMax);
				pInRGBLine[1] = CLIP(G, m_nMin, m_nMax);
				pInRGBLine[2] = CLIP(B, m_nMin, m_nMax);
			}
			pInRGBLine += 3;
			pInDarkLine++;
		}
	}
}
bool CHDRPlus_Tonemapping::BilateralSmoothYImagenew(MultiUshortImage *pInImage,/* SingleUcharImage *pMaskImage,*/ MultiUshortImage *pOutImage, int nThreP, int nThreM, int nMaskThreP, int nMaskThreM)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	const int nMask[5][5] =
	{
		{ 1,  4,  6,  4, 1},
		{ 4, 16, 24, 16, 4},
		{ 6, 24, 36, 24, 6},
		{ 4, 16, 24, 16, 4},
		{ 1,  4,  6,  4, 1}
	};
	if (!pOutImage->CreateImage(nWidth, nHeight, 1,16))return false;
	int nInvNoiseP = (1U << 28) / (nThreP*nThreP);
	int nInvNoiseM = (1U << 28) / (nThreM*nThreM);
	int nInvNoiseP_Mask = (1U << 28) / (nMaskThreP*nMaskThreP);
	int nInvNoiseM_Mask = (1U << 28) / (nMaskThreM*nMaskThreM);
	int nThreP2 = nThreP * nThreP;
	int nThreM2 = nThreM * nThreM;
	int nMaskThreP2 = nMaskThreP * nMaskThreP;
	int nMaskThreM2 = nMaskThreM * nMaskThreM;
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,16)
	for (int y = 0; y < nHeight; y++)
	{
		int i, j, Y0, Y, M;
		unsigned short *pIn = pInImage->GetImageLine(y);
		unsigned short *pOut = pOutImage->GetImageLine(y);
		//unsigned char *pMask = pMaskImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			long long sumY, dY;
			int W, sumW;
			M = 0;// *(pMask++);
			Y0 = *pIn;
			sumY = 0;
			sumW = 0;
			int tThreP = (nMaskThreP*M + (256 - M)*nThreP) >> 8;
			int tThreM = (nMaskThreM*M + (256 - M)*nThreM) >> 8;
			int tThreP2 = int(((long long)nMaskThreP2*M + (long long)nThreP2*(256 - M)) >> 8);
			int tThreM2 = int(((long long)nMaskThreM2*M + (long long)nThreM2*(256 - M)) >> 8);
			int tInvNoiseP = ((1 << 28) / tThreP2);
			int tInvNoiseM = ((1 << 28) / tThreM2);
			tInvNoiseP *= 7;
			tInvNoiseM *= 7;
			tInvNoiseP += (int)(((long long)nInvNoiseP_Mask*M + (long long)nInvNoiseP*(256 - M)) >> 8);
			tInvNoiseM += (int)(((long long)nInvNoiseM_Mask*M + (long long)nInvNoiseM*(256 - M)) >> 8);
			tInvNoiseP >>= 3;
			tInvNoiseM >>= 3;
			if (Y0 < m_nMax - 4096)
			{
				for (i = -2; i <= 2; i++)
				{
					if (y + i < 0 || y + i >= nHeight)continue;
					for (j = -2; j <= 2; j++)
					{
						if (x + j < 0 || x + j >= nWidth)continue;
						Y = pIn[i*nWidth + j];
						dY = Y - Y0;
						if (dY < 0)
						{
							dY *= dY;
							W = (int)(8 - ((dY*tInvNoiseP) >> 28));
						}
						else
						{
							dY *= dY;
							W = (int)(8 - ((dY*tInvNoiseM) >> 28));
						}
						if (W >= 0)
						{
							W = nMask[i + 2][j + 2] << W;
							sumY += Y * W;
							sumW += W;
						}
					}
				}
			}
			else
			{
				int tW = m_nMax - Y0;
				if (tW < 0)tW = 0;
				int TP = (tThreP*tW + 2048) >> 12;
				int TM = (tThreM*tW + 2048) >> 12;
				if (TP < 1)TP = 1;
				if (TM < 1)TM = 1;
				unsigned int nInvNoiseP1 = (1U << 28) / (TP*TP);
				unsigned int nInvNoiseM1 = (1U << 28) / (TM*TM);
				for (i = -2; i <= 2; i++)
				{
					if (y + i < 0 || y + i >= nHeight)continue;
					for (j = -2; j <= 2; j++)
					{
						if (x + j < 0 || x + j >= nWidth)continue;
						Y = pIn[i*nWidth + j];
						dY = Y - Y0;
						if (dY < 0)
						{
							dY *= dY;
							W = (int)(8 - ((dY*nInvNoiseP1) >> 28));
						}
						else
						{
							dY *= dY;
							W = (int)(8 - ((dY*nInvNoiseM1) >> 28));
						}
						if (W >= 0)
						{
							W = nMask[i + 2][j + 2] << W;
							sumY += Y * W;
							sumW += W;
						}
					}
				}
			}
			*(pOut++) = (int)(sumY / sumW);
			pIn++;
		}
	}
	return true;
}
bool CHDRPlus_Tonemapping::SmoothGammaYImage(MultiUshortImage *pInImage, MultiUshortImage *pOutImage)
{
	int i, nWidth[12], nHeight[12];
	MultiUshortImage InImagePyramid[12];
	MultiShortImage InImagePyramidEdge[12];
	int m_nPyramidLevel = 10;
	pInImage->GaussPyramidImage(InImagePyramid, InImagePyramidEdge, m_nPyramidLevel, true);
	if (!BilateralSmoothYImagenew(&InImagePyramid[m_nPyramidLevel], pOutImage, m_nSmoothYThreP[m_nPyramidLevel], m_nSmoothYThreM[m_nPyramidLevel], 1, 1))return false;
	MultiUshortImage TempImage;
	for (i = m_nPyramidLevel - 1; i >= 0; i--)
	{
		if (!pOutImage->UpScaleImagex2(&TempImage, false))return false;
		if (!pOutImage->CopyImageRect(&TempImage, 0, 0, InImagePyramidEdge[i].GetImageWidth(), InImagePyramidEdge[i].GetImageHeight()))return false;
		if (!InImagePyramid[i].AddBackEdgeImage(pOutImage, &InImagePyramidEdge[i]))return false;
		if (!BilateralSmoothYImagenew(&InImagePyramid[i], pOutImage, m_nSmoothYThreP[i], m_nSmoothYThreM[i], 1, 1))return false;
	}
	return true;
}
bool CHDRPlus_Tonemapping::EstimateDigiGain(MultiUshortImage *pInImage, TGlobalControl *pControl)
{
	int m, n, W, x, y, x1, y1, Y, C[1];
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	double fMean[1], fMeanV;
	long long int intMean[1], intMeanV;
	int nWinW = nWidth >> 2;
	int nWinH = nHeight >> 2;
	unsigned int *pHist = new unsigned int[m_nMax + 1];
	if (pHist == NULL)return false;
	memset(pHist, 0, sizeof(int)*(m_nMax + 1));
	fMean[0]=0;
	intMean[0]= 0;
	C[0]= 0;
	for (y = y1 = 0, m = 0; y < nHeight; y++)
	{
		WORD *pCFA = pInImage->GetImageLine(y);
		W = m_nBlockWeightMap[m][0];
		for (x = x1 = 0,  n = 0; x < nWidth; x++)
		{
			unsigned short Y = pCFA[0];
			intMean[0] += Y * W;
			C[0] += W;
			pHist[Y] += W;
			x1++;
			if (x1 == nWinW)
			{
				n++;
				W = m_nBlockWeightMap[m][n];
				x1 = 0;
			}
			pCFA++;
		}
		y1++;
		if (y1 == nWinH)
		{
			m++;
			y1 = 0;
		}
	}
	fMean[0] = (double)intMean[0];
	fMean[0] /= C[0];
	printf("GRAY Mean:[%f]\n", fMean[0]);
	if (fMean[0] < 0)fMean[0] = 0;
	fMeanV = fMean[0];
	if (fMeanV < 1.0)fMeanV = 1.0;
	unsigned int nPs = 0;
	unsigned int nThre1 = (C[0] * m_nHighLevelPtsPercent[0]) >> 16;//Ratio of histogram between 0 and X
	unsigned int nThre2 = (C[0] * m_nHighLevelPtsPercent[1]) >> 16;//Ratio of histogram between 0 and X
	if (nThre2 < nThre1)nThre2 = nThre1;
	for (y = m_nMax - 1; y > 0; y--)
	{
		nPs += pHist[y];
		if (nPs >= nThre1)
		{
			y++;
			break;
		}
	}
	if (y <= 0)y = 0 + 1;
	int m_nHighLevelGain[2];
	m_nHighLevelGain[0] = (1 << 23) / (y);
	printf("HighLevel0=%d HighGain0=%d\n", y, m_nHighLevelGain[0]);
	if (nThre2 > nThre1)
	{
		for (; y > 0; y--)
		{
			nPs += pHist[y];
			if (nPs >= nThre2)
			{
				y++;
				break;
			}
		}
		if (y <= 0)y = 1;
		m_nHighLevelGain[1] = (1 << 23) / (y);
	}
	else
	{
		m_nHighLevelGain[1] = m_nHighLevelGain[0];
	}
	printf("HighLevel1=%d HighGain1=%d\n", y, m_nHighLevelGain[1]);
	delete[] pHist;
	int m_nMeanY;
	if (m_bAutoDigiGainEnable == 1)
	{
		/*if (fMeanV < m_nTargetY)
		{*/
			pControl->nDigiGain = (int)(m_nTargetY*128.0 / fMeanV);
			if (pControl->nDigiGain > m_nMaxDigiGain)pControl->nDigiGain = m_nMaxDigiGain;
			if (m_bKeepHighLightEnable == 1)
			{
				if (pControl->nDigiGain > m_nHighLevelGain[0])pControl->nDigiGain = m_nHighLevelGain[0];
			}
		/*}
		else
		{
			pControl->nDigiGain = 128;
		}
		m_nMeanY = (int)(fMeanV*pControl->nDigiGain) / 128.0;*/
	}
	else
	{
		m_nMeanY = (int)(fMeanV*pControl->nDigiGain) / 128.0;
	}
	int nTotalGain = (pControl->nCameraGain*pControl->nDigiGain + 2047) / 2048;
	if (nTotalGain > m_nMaxTotalGain)
	{
		pControl->nDigiGain = (m_nMaxTotalGain * 2048) / pControl->nCameraGain;
		//if (pControl->nDigiGain < 128)pControl->nDigiGain = 128;
	}
	m_nMeanY = (int)(fMeanV*pControl->nDigiGain) / 128.0;
	printf("MeanV=%f Target=%d DigiGain=%d AftGain MeanY=%d\n", fMeanV, m_nTargetY, pControl->nDigiGain, m_nMeanY);
	printf("TotalGain=%.2f\n", (float)(pControl->nCameraGain*pControl->nDigiGain) / 2048.0);
	return true;
}
bool CHDRPlus_Tonemapping::Forward(MultiUshortImage *pInRGBImage,TGlobalControl *pControl)
{
	m_nMin = pControl->nBLC;
	m_nMax = pControl->nWP;
	int nGain = pControl->nCameraGain;//nGain x128
	float Amount;
	if (m_bAutoAmount)
	{
		if (m_nGainOption == 0)
		{
			nGain = pControl->nCameraGain;//iso gain
		}
		else if (m_nGainOption == 1)
		{
			nGain = (pControl->nCameraGain *pControl->nDigiGain + 64) >> 7;//iso+digigain
		}
		else
		{
			nGain = pControl->nEQGain;//lenshading之后的
		}
		if (nGain < m_nGainList[0])
		{
			Amount = m_nAmountGainList[0];
		}
		else if (nGain < m_nGainList[1])
		{
			int dG = (nGain - m_nGainList[0]);
			Amount = m_nAmountGainList[0];
			Amount += (m_nAmountGainList[1] - m_nAmountGainList[0])*dG / (m_nGainList[1] - m_nGainList[0]);
		}
		else if (nGain < m_nGainList[2])
		{
			int dG = (nGain - m_nGainList[1]);
			Amount = m_nAmountGainList[1];
			Amount += (m_nAmountGainList[2] - m_nAmountGainList[1])*dG / (m_nGainList[2] - m_nGainList[1]);
		}
		else
		{
			Amount = m_nAmountGainList[2];
		}
	}
	else
	{
		Amount = m_nManualAmount;
	}
	Amount = Amount / 16.0;
	////////////////////////////
	MultiUshortImage GrayImage;
	MultiUshortImage DarkImage;
	MultiUshortImage DarkGammaImage;
	MultiUshortImage BrightImage;
	MultiUshortImage BrightGammaImage;
	MultiUshortImage DarkOutImage;
	ConvertoGray(pInRGBImage, &GrayImage);
	if (!EstimateDigiGain(&GrayImage, pControl))return false;
	float comp = (float)m_nDynamicCompression/(float)128;// 5.8f;//暗光提亮
	float gain =  (float)pControl->nDigiGain / (float)128;// 0.5f;//高光压制
	float comp_const = 1.f + comp / m_nVirtualExposureNum;
	float gain_const = 1.f + gain / m_nVirtualExposureNum;
	float comp_slope = (comp - comp_const) / (float)(m_nVirtualExposureNum - 1);
	float gain_slope = (gain - gain_const) / (float)(m_nVirtualExposureNum - 1);
	DarkImage.Clone(&GrayImage);
	for (int pass = 0; pass < m_nVirtualExposureNum; pass++)
	{
		//float norm_comp = (pass + 1) * comp_slope + 1.0;
		//float norm_gain = (pass+1) * gain_slope + 1.0;
		float norm_comp = pass * comp_slope + comp_const;//增大
		float norm_gain = pass * gain_slope + gain_const;//减小
		printf("norm_comp=%f norm_gain=%f \n", norm_comp, norm_gain);
		Brighten(&DarkImage, norm_comp, &BrightImage);
		GrayGammaCorrect(&DarkImage, &DarkGammaImage);
		GrayGammaCorrect(&BrightImage, &BrightGammaImage);
		CombineDarkAndBrightImage(&DarkGammaImage, &BrightGammaImage, &DarkOutImage);
		GammaInverse(&DarkOutImage, &DarkGammaImage);
		Brighten(&DarkGammaImage, norm_gain, &DarkImage);
	}
	MultiUshortImage GrayImage1;
	//GrayImage1.Clone(&GrayImage);
	//SmoothGammaYImage(&GrayImage, &GrayImage1);
	GammaCombinRGB(pInRGBImage, &GrayImage, &DarkImage);
	return true;
}