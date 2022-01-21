#include "MultiUshortImage.h"
#include "SingleUcharImage.h"
MultiUshortImage::MultiUshortImage()
{
	m_nRawBLC = 16;
	m_nRawBits = 10;
	m_nRawMAXS = 1023;
	m_nRawCCM[0][0] = m_nRawCCM[1][1] = m_nRawCCM[2][2] = 1024;
	m_nRawCCM[0][1] = m_nRawCCM[0][2] = m_nRawCCM[1][0] = m_nRawCCM[1][2] = m_nRawCCM[2][0] = m_nRawCCM[2][1] = 0;
	m_nRawISOGain = 128;
	m_nRawAwbGain[0] = m_nRawAwbGain[1] = m_nRawAwbGain[2] = 256;
	m_nRawCFA = 0;
}
bool MultiUshortImage::CreateImage(int width, int height, int bits)
{
	m_nRawBits = bits;
	m_nRawMAXS = (1 << bits) - 1;
	m_nRawBLC = 0;
	return SetImageSize(width, height, 1);
}
bool MultiUshortImage::CreateImage(int width, int height,int dim, int bits)
{
	m_nRawBits = bits;
	m_nRawMAXS = (1 << bits) - 1;
	m_nRawBLC = 0;
	return SetImageSize(width, height, dim);
}
bool MultiUshortImage::Clone(MultiUshortImage *pInputImage)
{
	int nSize = pInputImage->GetImageSize();
	if (!CreateImage(pInputImage->GetImageWidth(), pInputImage->GetImageHeight(), pInputImage->m_nRawBits))
	{
		return false;
	}
	m_nRawMAXS = pInputImage->m_nRawMAXS;
	m_nRawBLC = pInputImage->m_nRawBLC;
	CopyParameters(pInputImage);
	memcpy(m_pImgData, pInputImage->GetImageData(), sizeof(unsigned short)*nSize);
	return true;
}
bool MultiUshortImage::CreateImageWithData(int nWidth, int nHeight,int nDim, unsigned short *pInputData)
{
	if (!SetImageSize(nWidth, nHeight, nDim))return false;
	memcpy(GetImageData(), pInputData, nWidth*nHeight*nDim*sizeof(unsigned short));
	return true;
}
void MultiUshortImage::CopyParameters(MultiUshortImage *pInputImage)
{
	m_nRawCFA = pInputImage->m_nRawCFA;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_nRawCCM[i][j] = pInputImage->m_nRawCCM[i][j];
		}
		m_nRawAwbGain[i] = pInputImage->m_nRawAwbGain[i];
	}
	m_nRawISOGain = pInputImage->m_nRawISOGain;
}
bool MultiUshortImage::BGRHToBGR(MultiUshortImage *pOutBGRImage)
{
	int i, x, y, g;
	if (!pOutBGRImage->CreateImage(m_nWidth, m_nHeight, 3))return false;
	pOutBGRImage->m_nRawMAXS = m_nRawMAXS;
	for (y = 0; y < m_nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned short *pOutLine = pOutBGRImage->GetImageLine(y);
		for (x = 0; x < m_nWidth; x++)
		{
			for (i = 0; i < 3; i++)
			{
				g = pInLine[i];
				if (m_nChannel > 3)
				{
					g += pInLine[3];
				}
				if (g < 0)g = 0;
				if (g > m_nRawMAXS)g = m_nRawMAXS;
				*(pOutLine++) = (unsigned short)g;
			}
			pInLine += m_nChannel;
		}
	}
	return true;
}
bool MultiUshortImage::GetBGGRMean(double fMean[])
{
	int i, j, x, y, Y;
	int C = (m_nWidth*m_nHeight) >> 2;
	if (m_nWidth < 2 || m_nHeight < 2 ||  m_nRawCFA>3)return false;
	unsigned short *pCFA = m_pImgData;
	fMean[0] = fMean[1] = fMean[2] = fMean[3] = 0;
	for (y = 0, i = m_nRawCFA & 2; y < m_nHeight; y++, i ^= 2)
	{
		for (x = 0, j = (m_nRawCFA & 1) | i; x < m_nWidth; x++, j ^= 1)
		{
			Y = *(pCFA++);
			fMean[j] += Y;
		}
	}
	fMean[0] /= C;
	fMean[1] /= C;
	fMean[2] /= C;
	fMean[3] /= C;
	return true;
}
bool MultiUshortImage::Extend2Image(MultiUshortImage *pInImage, MultiUshortImage *pOutImage,int nS)//2的x次方
{
	if (nS < 0)return false;
	int nMask = (1 << nS) - 1;
	int nInWidth = pInImage->GetImageWidth();
	int nInHeight = pInImage->GetImageHeight();
	int nDim = GetImageDim();
	int nOutWidth = nInWidth + (((1 << nS) - (nInWidth&nMask))&nMask);
	int nOutHeight = nInHeight + (((1 << nS) - (nInHeight&nMask))&nMask);
	if (pOutImage->GetImageWidth() != nOutWidth || pOutImage->GetImageHeight() != nOutHeight)
	{
		if (!pOutImage->CreateImage(nOutWidth, nOutHeight, nDim,16))return false;
	}
	FillWordData(pInImage->GetImageData(), pOutImage->GetImageData(), nInWidth, nInHeight, nOutWidth, nOutHeight, nDim);
	return true;
}
bool MultiUshortImage::DownScaleImagex2(MultiUshortImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	MultiUshortImage TempImage;
	MultiUshortImage *pInImage = this;
	if ((nWidth & 1) == 1 || (nHeight & 1) == 1)
	{
		Extend2Image(pInImage, &TempImage, 1);
		pInImage = &TempImage;
		nWidth = pInImage->GetImageWidth();
		nHeight = pInImage->GetImageHeight();
	}
	if (!pOutImage->CreateImage(nWidth >> 1, nHeight >> 1, nDim, 16))return false;
	return DownScaleWordDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, nDim, bDitheringEnable);
}
bool MultiUshortImage::UpScaleImagex2(MultiUshortImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	if (!pOutImage->CreateImage(nWidth * 2, nHeight * 2, nDim,16))return false;
	return UpScaleWordDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, nDim);
}

bool MultiUshortImage::SubtractEdgeImage(MultiUshortImage *pInImage, MultiShortImage *pOutImage)
{
	int nWidth0 = GetImageWidth();
	int nHeight0 = GetImageHeight();
	int nDim = GetImageDim();
	int nWidth1 = pInImage->GetImageWidth();
	int nHeight1 = pInImage->GetImageHeight();
	if (nWidth1 < nWidth0 || nHeight1 < nHeight0)return false;
	if (!pOutImage->CreateImage(nWidth0, nHeight0, 1))return false;
	SubtractWordEdgeData(GetImageData(), pInImage->GetImageData(), pOutImage->GetImageData(), nWidth0, nHeight0, nWidth1, nHeight1, nDim);
	return true;
}
bool MultiUshortImage::AddBackEdgeImage(MultiUshortImage *pInputImage, MultiShortImage *pInputEdgeImage)
{
	int nWidth = pInputImage->GetImageWidth();
	int nHeight = pInputImage->GetImageHeight();
	int nDim = pInputImage->GetImageDim();
	int nEdgeWidth = pInputEdgeImage->GetImageWidth();
	int nEdgeHeight = pInputEdgeImage->GetImageHeight();
	if (nEdgeWidth > nWidth || nEdgeHeight > nHeight || pInputEdgeImage->GetImageDim() != 1)return false;
	if (!this->CreateImage(nEdgeWidth, nEdgeHeight, nDim,16))return false;
	AddBackWordEdge(pInputImage->GetImageData(), pInputEdgeImage->GetImageData(), GetImageData(), nWidth, nHeight, nEdgeWidth, nEdgeHeight, nDim);
	return true;
}
bool MultiUshortImage::GaussPyramidImage(MultiUshortImage *pOutPyramid, MultiShortImage *pOutEdgePyramid, int &nPyramidLevel,bool SaveEdge)
{
	int nWidth[12], nHeight[12];
	MultiUshortImage  TempImage;
	printf("input nPyramidLevel=%d\n", nPyramidLevel);
	pOutPyramid[0].Clone(this);
	for (int i = 0; i < nPyramidLevel; i++)
	{
		nWidth[i] = pOutPyramid[i].GetImageWidth();
		nHeight[i] = pOutPyramid[i].GetImageHeight();
		printf("Level=%d Size=[%d,%d]\n", i, nWidth[i], nHeight[i]);
		if (nWidth[i] <= 4 || nHeight[i] <= 4)
		{
			nPyramidLevel = i;
			printf("out PyramidLevel=%d\n", nPyramidLevel);
			break;
		}	
		if ((nWidth[i] & 1) == 1 || (nHeight[i] & 1) == 1)
		{
			MultiUshortImage  tmpExtendImage;
			if (!pOutPyramid[i].Extend2Image(&pOutPyramid[i], &tmpExtendImage, 1))return false;
			pOutPyramid[i].Clone(&tmpExtendImage);
		}
		if (!pOutPyramid[i].DownScaleImagex2(&pOutPyramid[i + 1], false))return false;
		if (SaveEdge==true)
		{
			if (!pOutPyramid[i + 1].UpScaleImagex2(&TempImage, false))return false;
			if (!pOutPyramid[i].SubtractEdgeImage(&TempImage, pOutEdgePyramid + i))return false;
		}
	}
	return true;
}
bool MultiUshortImage::ApplyWeight(MultiUshortImage *pWeightImage,int ScaleBit)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	for (int y=0;y<nHeight;y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned short *pInWeightLine = pWeightImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			for (int c = 0; c < nDim; c++)
			{
				unsigned int tmpOut = pInLine[c] * pInWeightLine[c];//x^16 *x^12=2^28;
				tmpOut >>= ScaleBit;
				pInLine[c] =CLIP(tmpOut, 0, 65535);
			}
			pInLine += nDim;
			pInWeightLine += nDim;
		}
	}
	return true;
}
bool MultiUshortImage::FuseDiffImageWeight(MultiUshortImage *pRefImage,MultiUshortImage *pWeightImage, int ScaleBit)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned short *pRefLine = pRefImage->GetImageLine(y);
		unsigned short *pInWeightLine = pWeightImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			for (int c = 0; c < nDim; c++)
			{
				int tmpOut=(pInLine[c] - pRefLine[c])*pInWeightLine[c];
				tmpOut >>= ScaleBit;
				pInLine[c] = CLIP(tmpOut, 0, 65535);
			}
			pInLine += nDim;
			pRefLine += nDim;
			pInWeightLine += nDim;
		}
	}
	return true;
}
bool MultiUshortImage::AddImage(MultiUshortImage *pRefImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned short *pInRefLine = pRefImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			for (int c = 0; c < nDim; c++)
			{
				int tmpOut= pInLine[0] + pInRefLine[0];
				pInLine[c] = CLIP(tmpOut, 0, 65535);
			}
			pInLine += nDim;
			pInRefLine += nDim;
		}
	}
	return true;
}
bool MultiUshortImage::AddImage(MultiShortImage *pRefImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	#pragma omp parallel for  
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		short *pInRefLine = pRefImage->GetImageLine(y);
		if (nDim==1)
		{
			for (int x = 0; x < nWidth; x++)
			{
				int tmpOut = pInLine[0] + pInRefLine[0];
				pInLine[0] = CLIP(tmpOut, 0, 65535);
				pInLine++;
				pInRefLine++;
			}
		}
		else
		{
			for (int x = 0; x < nWidth; x++)
			{
				for (int c = 0; c < nDim; c++)
				{
					int tmpOut = pInLine[0] + pInRefLine[0];
					pInLine[c] = CLIP(tmpOut, 0, 65535);
				}
				pInLine += nDim;
				pInRefLine += nDim;
			}
		}
	}

	return true;
}
bool MultiUshortImage::NormalizeByOutBlcAndBit(MultiUshortImage *pInImage, int m_nOutputBLC, int m_nOutputBits)
{
	//int x, y, CFA, E[2];
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	int nMAXS = (1 << m_nOutputBits) - 1;
	int nInBLC = pInImage->m_nRawBLC;
	int nGain = ((nMAXS - m_nOutputBLC) << 10) / (pInImage->m_nRawMAXS - nInBLC);
	int nOffset = m_nOutputBLC * 1024 - nInBLC * nGain;

	printf("HDRNormalize: BLC=%d MAXS=%d\n", pInImage->m_nRawBLC, pInImage->m_nRawMAXS);
	pInImage->m_nRawMAXS = nMAXS;
	pInImage->m_nRawBLC = m_nOutputBLC;
	pInImage->m_nRawBits = m_nOutputBits;
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,16)
	for (int y = 0; y < nHeight; y++)
	{
		int x, CFA, E[2];
		unsigned short *pLine = pInImage->GetImageLine(y);
		E[0] = E[1] = 0;
		for (x = 0; x < nWidth; x += 2)
		{
			CFA = *pLine;
			CFA = CFA * nGain + nOffset;
			if (CFA < 0)CFA = 0;
			CFA += E[0];
			E[0] = CFA & 1023;
			CFA >>= 10;
			if (CFA > nMAXS)CFA = nMAXS;
			*(pLine++) = (unsigned short)CFA;

			CFA = *pLine;
			CFA = CFA * nGain + nOffset;
			if (CFA < 0)CFA = 0;
			CFA += E[1];
			E[1] = CFA & 1023;
			CFA >>= 10;
			if (CFA > nMAXS)CFA = nMAXS;
			*(pLine++) = (unsigned short)CFA;
		}
	}
	return true;
}
void MultiUshortImage::NormalizeBit(int nNewBit)
{
	if (m_nRawBits > nNewBit)
	{
		int nShift = m_nRawBits - nNewBit;
		m_nRawMAXS = (1 << nNewBit) - 1;
		m_nRawBLC = m_nRawBLC >> nShift;
		m_nRawBits = nNewBit;
#pragma omp parallel for schedule(dynamic,8)
		for (int y = 0; y < m_nHeight; y++)
		{
			unsigned short *pY = m_pImgData + y * m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int Y = *pY;
				Y >>= nShift;
				if (Y > m_nRawMAXS)Y = m_nRawMAXS;
				*(pY++) = (unsigned short)Y;
			}
		}
	}
	else if (m_nRawBits < nNewBit)
	{
		int nShift = nNewBit - m_nRawBits;
		int nMask = (1 << nShift) - 1;
		m_nRawMAXS = (1 << nNewBit) - 1;
		m_nRawBLC = (m_nRawBLC << nShift) | nMask;
		m_nRawBits = nNewBit;
#ifdef USE_NEON
		uint16x8_t vnMask = vdupq_n_u16(nMask);
		uint16x8_t vnMAXS = vdupq_n_u16(m_nRawMAXS);
#endif 
#pragma omp parallel for  schedule(dynamic,32)
		for (int y = 0; y < m_nHeight; y++)
		{
			unsigned short *pY = m_pImgData + y * m_nWidth;
			int x = 0;
#ifdef USE_NEON
			if (nShift == 2)
			{
				for (; x < m_nWidth - 7; x += 8)
				{
					uint16x8_t vData = vld1q_u16(pY);
					vData = vshlq_n_u16(vData, 2);
					vData = vorrq_u16(vData, vnMask);
					vData = vminq_u16(vData, vnMAXS);
					vst1q_u16(pY, vData);   pY += 8;
				}
			}
			else
			{
				printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx----have error xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
				/*for (; x < m_nWidth - 7; x += 8)
				{
					uint16x8_t vData = vld1q_u16(pY);
					vData = vshlq_n_u16(vData, nShift);
					vData = vorrq_u16(vData, vnMask);
					vData = vminq_u16(vData, vnMAXS);
					vst1q_u16(pY, vData);   pY += 8;
				}*/
			}
#endif 
			for (; x < m_nWidth; x++)
			{
				int Y = *pY;
				Y = (Y << nShift) | nMask;
				if (Y > m_nRawMAXS)Y = m_nRawMAXS;
				*(pY++) = (unsigned short)Y;
			}
		}
	}
}
int MultiUshortImage::GetRAWStride(int nWidth, int nMIPIRAW)
{
	int stride;
	if (nMIPIRAW == 0)
	{
		stride = sizeof(unsigned short) * nWidth;
	}
	else if (nMIPIRAW == 1)
	{
		stride = (nWidth * 5) / 4;
	}
	else if (nMIPIRAW == 2)
	{
		stride = (((nWidth * 5) >> 2) + 8)&(~7);
	}
	else if (nMIPIRAW == 3)
	{
		stride = (nWidth * 5) / 4;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return -1;
	}
	return stride;
}
bool MultiUshortImage::ConverterRawDataToISPRawData(char *pInputRawData, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW)
{
	int stride;
	if (nMIPIRAW == 0)
	{
		stride = sizeof(unsigned short) * nWidth;
	}
	else if (nMIPIRAW == 1)
	{
		stride = (nWidth * 5) / 4;
	}
	else if (nMIPIRAW == 2)
	{
		stride = (((nWidth * 5) >> 2) + 8)&(~7);
	}
	else if (nMIPIRAW == 3)
	{
		stride = (nWidth * 5) / 4;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return false;
	}
	if (nMIPIRAW == 0)
	{
		int x, y, g;
		int mask = (1 << nBits) - 1;
		int shift = (16 - nBits);
		if (!CreateImage(nWidth, nHeight, nBits))return false;
		for (y = 0; y < nHeight; y++)
		{
			unsigned short *RawLine = (unsigned short *)(pInputRawData + y * nWidth);
			unsigned short *wpLine = GetImageLine(y);
			for (x = 0; x < nWidth; x++)
			{
				g = RawLine[x];
				if (!bByteOrder)
				{
					g = ((g >> 8) & 255) | ((g & 255) << 8);
				}
				if (bHighBit)g = g >> shift;
				if (g > mask)
				{
					printf("Pos [%d,%d] Pixel %d>%d\n", x, y, g, mask);
				}
				g = g & mask;
				wpLine[x] = (unsigned short)g;
			}
		}
		return true;
	}
	else if (nMIPIRAW == 1)
	{
		//RAW 10
		int i, j, g[4];
		unsigned char Buffer[5];
		if (!CreateImage(nWidth, nHeight, 10))return false;
		unsigned short *pInputImage = GetImageData();
		unsigned char *RawLine = (unsigned char *)(pInputRawData);
		for (i = 0; i < nWidth*nHeight; i += 4)
		{
			for (int cnt = 0; cnt < 5; cnt++)
			{
				Buffer[cnt] = RawLine[cnt];
			}
			RawLine += 5;
			int nShift = 6;
			for (j = 0; j < 4; j++, nShift -= 2)
			{
				g[j] = Buffer[j];
				g[j] <<= 2;
				g[j] |= (Buffer[4] >> nShift) & 3;

				*(pInputImage++) = (unsigned short)g[j];
			}
		}
		return true;
	}
	else if (nMIPIRAW == 2)
	{
		if (!CreateImage(nWidth, nHeight, 10))return false;
		//QCOM MIPI10
		for (int y = 0; y < nHeight; y++)
		{
			unsigned short *pInputImage = GetImageLine(y);
			int x, g;
			unsigned char *pSrc = (unsigned char *)(pInputRawData + y * stride);
			for (x = 0; x < nWidth / 4; x++)
			{
				g = pSrc[0];
				g = (g << 2) | (pSrc[4] >> 6);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[1];
				g = (g << 2) | ((pSrc[4] >> 4) & 3);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[2];
				g = (g << 2) | ((pSrc[4] >> 2) & 3);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[3];
				g = (g << 2) | (pSrc[4] & 3);
				*(pInputImage++) = (unsigned short)g;
				pSrc += 5;
			}
		}
		return true;
	}
	else if (nMIPIRAW == 3)
	{
		if (!CreateImage(nWidth, nHeight, 10))return false;
		//MTK RAW10
		unsigned short *pInputImage = GetImageData();
		unsigned char *buffer = (unsigned char *)(pInputRawData);
		for (int i = 0; i < (nWidth*nHeight) >> 2; i++)
		{
			int g = buffer[1] & 3;
			g = (g << 8) | buffer[0];
			*(pInputImage++) = (unsigned short)g;
			g = buffer[2] & 15;
			g = (g << 6) | ((buffer[1] >> 2) & 63);
			*(pInputImage++) = (unsigned short)g;
			g = buffer[3] & 63;
			g = (g << 4) | ((buffer[2] >> 4) & 15);
			*(pInputImage++) = (unsigned short)g;
			g = buffer[4];
			g = (g << 2) | ((buffer[3] >> 6) & 3);
			*(pInputImage++) = (unsigned short)g;
			buffer += 5;
		}
		return true;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return false;
	}
}
bool MultiUshortImage::Load16BitRawDataFromBinFile(char *pFileName, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW)//bByteOrder小端正常都是小端模式（小端字节序就是最低有效字节落在低地址上的字节存放方式）bHighBit 高低位有效 nMIPIRAW==0表示不是mipi 其他表示不同mipi
{
	if (nMIPIRAW == 0)//0表示不是mipi其他代表不同mipi
	{
		int x, y, g;
		int mask = (1 << nBits) - 1;
		int shift = (16 - nBits);
		if (!CreateImage(nWidth, nHeight, nBits))return false;
		FILE *fp = fopen(pFileName, "rb");
		if (fp == NULL)return false;
		for (y = 0; y < nHeight; y++)
		{
			unsigned short *wpLine = GetImageLine(y);
			if (fread(wpLine, sizeof(unsigned short), nWidth, fp) != nWidth)
			{
				printf("Read RAW16 File from %s Error!!!\n", pFileName);
				fclose(fp);
				return false;
			}
			for (x = 0; x<nWidth; x++)
			{
				g = wpLine[x];
				if (!bByteOrder)
				{
					g = ((g >> 8) & 255) | ((g & 255) << 8);
				}
				if (bHighBit)g = g >> shift;
				if (g>mask)
				{
					printf("Pos [%d,%d] Pixel %d>%d\n", x, y, g, mask);
				}
				g = g & mask;
				wpLine[x] = (unsigned short)g;
			}
		}
		fclose(fp);
		return true;
	}
	else if (nMIPIRAW == 1)
	{
		//RAW 10
		if (!CreateImage(nWidth, nHeight, 10))return false;
		SingleUcharImage tmpbuf;
		tmpbuf.CreateImage(nWidth, nHeight);
		FILE *fp = fopen(pFileName, "rb");
		if (fp == NULL)return false;
		fread(tmpbuf.GetImageData(), 1, nWidth*nHeight/4*5, fp);
		fclose(fp);
		int nProcs = omp_get_num_procs();
	#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16) 
		for (int y = 0; y < nHeight; y++)
		{
			unsigned short *pInputImage = GetImageLine(y);
			unsigned char *pInput = tmpbuf.GetImageData()+y*nWidth*5/4;
			unsigned char Buffer[5];
			for (int i = 0; i < nWidth; i += 4)
			{
				Buffer[0] = *pInput++;
				Buffer[1] = *pInput++;
				Buffer[2] = *pInput++;
				Buffer[3] = *pInput++;
				Buffer[4] = *pInput++;
				int nShift = 6;
				int g[4];
				for (int j = 0; j < 4; j++, nShift -= 2)
				{
					g[j] = Buffer[j];
					g[j] <<= 2;
					g[j] |= (Buffer[4] >> nShift) & 3;
					*(pInputImage++) = (unsigned short)g[j];
				}
			}
		}
		return true;
	}
	else if (nMIPIRAW == 2)
	{
		//QCOM MIPI10
		int x, y, g;
		int nStride = (((nWidth * 5) >> 2) + 8)&(~7);
		FILE *fp = fopen(pFileName, "rb");
		if (fp == NULL)return false;
		printf("Width=%d Stride=%d\n", nWidth, nStride);
		if (!CreateImage(nWidth, nHeight, 10))
		{
			fclose(fp);
			return false;
		}
		unsigned char *pBuffer = new unsigned char[nStride];
		if (pBuffer == NULL)
		{
			fclose(fp);
			return false;
		}
		unsigned short *pInputImage = GetImageData();
		for (y = 0; y < nHeight; y++)
		{
			if (fread(pBuffer, 1, nStride, fp) < nStride)
			{
				fclose(fp);
				delete[] pBuffer;
				return false;
			}
			unsigned char *pSrc = pBuffer;
			for (x = 0; x < nWidth / 4; x++)
			{
				g = pSrc[0];
				g = (g << 2) | (pSrc[4] >> 6);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[1];
				g = (g << 2) | ((pSrc[4] >> 4) & 3);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[2];
				g = (g << 2) | ((pSrc[4] >> 2) & 3);
				*(pInputImage++) = (unsigned short)g;
				g = pSrc[3];
				g = (g << 2) | (pSrc[4] & 3);
				*(pInputImage++) = (unsigned short)g;
				pSrc += 5;
			}
		}
		fclose(fp);
		delete[] pBuffer;
		return true;
	}
	else if (nMIPIRAW == 3)
	{
		//MTK RAW10
		int i, buffer[5], g;
		FILE *fp = fopen(pFileName, "rb");
		if (fp == NULL)return false;
		if (!CreateImage(nWidth, nHeight, 10))
		{
			fclose(fp);
			return false;
		}
		unsigned short *pInputImage = GetImageData();
		for (i = 0; i < (nWidth*nHeight)>>2; i++)
		{
			if (fread(buffer, 1, 5, fp) != 5)
			{
				printf("Read RAW16 File from %s Error!!!\n", pFileName);
				fclose(fp);
				return false;
			}
			g = buffer[1] & 3;
			g = (g << 8) | buffer[0];
			*(pInputImage++) = (unsigned short)g;
			g = buffer[2] & 15;
			g = (g << 6) | ((buffer[1] >> 2) & 63);
			*(pInputImage++) = (unsigned short)g;
			g = buffer[3] & 63;
			g = (g << 4) | ((buffer[2] >> 4) & 15);
			*(pInputImage++) = (unsigned short)g;
			g = buffer[4];
			g = (g << 2) | ((buffer[3] >> 6) & 3);
			*(pInputImage++) = (unsigned short)g;
		}
		fclose(fp);
		return true;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return false;
	}
}
bool MultiUshortImage::Load16BitRawDataFromISPRawData(unsigned short *pInputData, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW)
{
	//RAW 10
	if (nMIPIRAW==1)
	{
		unsigned char *datain1 = (unsigned char *)pInputData;
		if (!CreateImage(nWidth, nHeight, 10))return false;
		int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16) 
		for (int y = 0; y < nHeight; y++)
		{
			unsigned short *pInputImage = GetImageLine(y);
			unsigned char *pInput = datain1 + y * nWidth * 5 / 4;
			unsigned char Buffer[5];
			for (int i = 0; i < nWidth; i += 4)
			{
				int nShift = 6;
				unsigned short g[4];
				Buffer[0] = *pInput++;
				Buffer[1] = *pInput++;
				Buffer[2] = *pInput++;
				Buffer[3] = *pInput++;
				Buffer[4] = *pInput++;
				for (int j = 0; j < 4; j++, nShift -= 2)
				{
					g[j] = Buffer[j];
					g[j] <<= 2;
					g[j] |= (Buffer[4] >> nShift) & 3;
					*(pInputImage++) = (unsigned short)g[j];
				}
			}
		}
	}
	else if (nMIPIRAW == 0)
	{
		//MTK mipiRAW10
		unsigned short mask = (1 << nBits) - 1;
		int shift = (16 - nBits);
		if (!CreateImage(nWidth, nHeight, nBits))return false;
#ifdef USE_NEON
		uint16x8_t vnMask = vdupq_n_u16(mask);
#endif
		int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16) 
		for (int y = 0; y < nHeight; y++)
		{
			unsigned short *wpLine = GetImageLine(y);
			unsigned short *pdata = pInputData + y * nWidth;
			int x = 0;
#ifdef USE_NEON
			for (; x < nWidth - 7; x += 8)
			{
				uint16x8_t vData = vld1q_u16(pdata);
				vst1q_u16(wpLine, vandq_u16(vData, vnMask));
				pdata += 8;
				wpLine += 8;
			}
#endif
			for (; x < nWidth; x++)
			{
				unsigned short g = *pdata++;
				g = g & mask;
				wpLine[x] = (unsigned short)g;
			}
		}
	}
	return true;
}
bool MultiUshortImage::ConverterISPRawDataToRawData(unsigned char **pOutData, int nBits, bool bHighBit, bool bByteOrder, int &nLen, int nMIPIRAW)
{
	if (nMIPIRAW == 0)
	{
		int x, y, g;
		int nShift = (bHighBit) ? (16 - m_nRawBits) : (m_nRawBits - nBits);
		nLen = m_nWidth * m_nHeight * 2;
		unsigned short *pbuf=new unsigned short[m_nWidth*m_nHeight];
		*pOutData = (unsigned char *)pbuf;
		for (y = 0; y < m_nHeight; y++)
		{
			unsigned short *wpLine = GetImageLine(y);
			unsigned short *pBuffer = pbuf + y * m_nWidth;
			for (x = 0; x < m_nWidth; x++)
			{
				g = wpLine[x];
				if (bHighBit)
				{
					g <<= nShift;
				}
				else if (nShift >= 0)
				{
					g >>= nShift;
				}
				else
				{
					g <<= (-nShift);
				}
				if (!bByteOrder)
				{
					g = ((g >> 8) & 255) | ((g & 255) << 8);
				}
				pBuffer[x] = (unsigned short)g;
			}
		}
		return true;
	}
	else if (nMIPIRAW == 1)
	{
		//RAW 10
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		nLen = m_nWidth * m_nHeight * 5 / 4;
		unsigned char *pbuf = new unsigned char[nLen];
		*pOutData = pbuf;
		int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16) 
		for (int y=0;y< m_nHeight;y++)
		{
			int  g[4];
			unsigned char *Buffer = pbuf+ m_nWidth * y * 5 / 4;
			unsigned short *pInputImage = GetImageLine(y);
			for (int x = 0; x < m_nWidth; x+= 4)
			{
				int nShift = 6;
				Buffer[4] = 0;
				for (int j = 0; j < 4; j++, nShift -= 2)
				{
					g[j] = *(pInputImage++);
					Buffer[j] = (unsigned char)(g[j] >> 2);
					Buffer[4] |= (unsigned char)((g[j] & 3) << nShift);
				}
				Buffer += 5;
			}
		}
		return true;
	}
	else if (nMIPIRAW == 2)
	{
		//QCOM MIPI10
		int x, y;
		unsigned short g[4];
		int nStride = (((m_nWidth * 5) >> 2) + 8)&(~7);
		printf("Width=%d Stride=%d\n", m_nWidth, nStride);
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		nLen = nStride * m_nHeight;
		unsigned char *pbuf = new unsigned char[nStride*m_nHeight];
		*pOutData = pbuf;
		unsigned char *pBuffer = new unsigned char[nStride];
		unsigned short *pInputImage = GetImageData();
		for (y = 0; y < m_nHeight; y++)
		{
			unsigned char *pBuffer = pbuf + y * nStride;
			unsigned char *pSrc = pBuffer;
			for (x = 0; x < m_nWidth / 4; x++)
			{
				g[0] = *(pInputImage++);
				g[1] = *(pInputImage++);
				g[2] = *(pInputImage++);
				g[3] = *(pInputImage++);
				pSrc[0] = (unsigned char)(g[0] >> 2);
				pSrc[1] = (unsigned char)(g[1] >> 2);
				pSrc[2] = (unsigned char)(g[2] >> 2);
				pSrc[3] = (unsigned char)(g[3] >> 2);
				pSrc[4] = (unsigned char)(((g[0] & 3) << 6) | ((g[1] & 3) << 4) | ((g[2] & 3) << 2) | (g[3] & 3));
				pSrc += 5;
			}
		}
		return true;
	}
	else if (nMIPIRAW == 3)
	{
		//MTK RAW10
		int i;
		unsigned short g[4];
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		nLen = m_nWidth * m_nHeight * 5 / 4;
		unsigned char *pbuf = new unsigned char[m_nWidth*m_nHeight * 5 / 4];
		*pOutData = pbuf;
		unsigned char *buffer = pbuf;
		unsigned short *pInputImage = GetImageData();
		for (i = 0; i < (m_nWidth*m_nHeight) >> 2; i++, buffer+=5)
		{
			g[0] = *(pInputImage++);
			g[1] = *(pInputImage++);
			g[2] = *(pInputImage++);
			g[3] = *(pInputImage++);
			buffer[0] = (unsigned char)(g[0] & 255);
			buffer[1] = (unsigned char)((g[0] >> 8) & 3);
			buffer[1] |= (unsigned char)((g[1] & 63) << 2);
			buffer[2] = (unsigned char)((g[1] >> 6) & 15);
			buffer[2] |= (unsigned char)((g[2] & 15) << 4);
			buffer[3] = (unsigned char)((g[2] >> 4) & 63);
			buffer[3] |= (unsigned char)((g[3] & 3));
			buffer[4] = (unsigned char)((g[3] >> 2) & 255);
		}
		return true;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return false;
	}
	return true;
}
bool MultiUshortImage::Save16BitRawToBinFile(char *pFileName, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW)
{
	if (nMIPIRAW == 0)
	{
		int x, y, g;
		int nShift = (bHighBit) ? (16 - m_nRawBits) : (m_nRawBits - nBits);
		unsigned short *pBuffer = new unsigned short[m_nWidth];
		if (pBuffer == NULL)return false;
		FILE *fp = fopen(pFileName, "wb");
		if (fp == NULL)
		{
			delete[] pBuffer;
			return false;
		}
		for (y = 0; y < m_nHeight; y++)
		{
			unsigned short *wpLine = GetImageLine(y);
			for (x = 0; x < m_nWidth; x++)
			{
				g = wpLine[x];
				if (bHighBit)
				{
					g <<= nShift;
				}
				else if (nShift >= 0)
				{
					g >>= nShift;
				}
				else
				{
					g <<= (-nShift);
				}
				if (!bByteOrder)
				{
					g = ((g >> 8) & 255) | ((g & 255) << 8);
				}
				pBuffer[x] = (unsigned short)g;
			}
			fwrite(pBuffer, m_nWidth, sizeof(unsigned short), fp);
		}
		fclose(fp);
		delete[] pBuffer;
		return true;
	}
	else if (nMIPIRAW == 1)
	{
		//RAW 10
		int i, j, g[4];
		unsigned char Buffer[5];
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		FILE *fp = fopen(pFileName, "wb");
		if (fp == NULL)return false;
		unsigned short *pInputImage = GetImageData();
		for (i = 0; i < m_nWidth*m_nHeight; i += 4)
		{
			int nShift = 6;
			Buffer[4] = 0;
			for (j = 0; j < 4; j++, nShift -= 2)
			{
				g[j] = *(pInputImage++);
				Buffer[j] =(unsigned char) (g[j] >> 2);
				Buffer[4] |=(unsigned char) ((g[j] & 3) << nShift);
			}
			if (fwrite(Buffer, 1, 5, fp) != 5)
			{
				printf("Save RAW16 File to %s Error!!!\n", pFileName);
				fclose(fp);
				return false;
			}
		}
		fclose(fp);
		return true;
	}
	else if (nMIPIRAW == 2)
	{
		//QCOM MIPI10
		int x, y;
		unsigned short g[4];
		int nStride = (((m_nWidth * 5) >> 2) + 8)&(~7);
		FILE *fp = fopen(pFileName, "wb");
		if (fp == NULL)return false;
		printf("Width=%d Stride=%d\n", m_nWidth, nStride);
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		unsigned char *pBuffer = new unsigned char[nStride];
		if (pBuffer == NULL)
		{
			fclose(fp);
			return false;
		}
		unsigned short *pInputImage = GetImageData();
		for (y = 0; y < m_nHeight; y++)
		{
			unsigned char *pSrc = pBuffer;
			for (x = 0; x < m_nWidth / 4; x++)
			{
				g[0] = *(pInputImage++);
				g[1] = *(pInputImage++);
				g[2] = *(pInputImage++);
				g[3] = *(pInputImage++);
				pSrc[0] = (unsigned char)(g[0] >> 2);
				pSrc[1] = (unsigned char)(g[1] >> 2);
				pSrc[2] = (unsigned char)(g[2] >> 2);
				pSrc[3] = (unsigned char)(g[3] >> 2);
				pSrc[4] = (unsigned char)(((g[0] & 3) << 6) | ((g[1] & 3) << 4) | ((g[2] & 3) << 2) | (g[3] & 3));	
				pSrc += 5;
			}
			if (fwrite(pBuffer, 1, nStride, fp) < nStride)
			{
				fclose(fp);
				delete[] pBuffer;
				return false;
			}
		}
		fclose(fp);
		delete[] pBuffer;
		return true;
	}
	else if (nMIPIRAW == 3)
	{
		//MTK RAW10
		int i;
		unsigned char buffer[5];
		unsigned short g[4];
		FILE *fp = fopen(pFileName, "wb");
		if (fp == NULL)return false;
		if (m_nRawBits != 10)
		{
			NormalizeBit(10);
		}
		unsigned short *pInputImage = GetImageData();
		for (i = 0; i < (m_nWidth*m_nHeight) >> 2; i++)
		{
			g[0] = *(pInputImage++);
			g[1] = *(pInputImage++);
			g[2] = *(pInputImage++);
			g[3] = *(pInputImage++);
			buffer[0] = (unsigned char) (g[0] & 255);
			buffer[1] = (unsigned char)((g[0] >> 8) & 3);
			buffer[1] |= (unsigned char)((g[1] & 63) << 2);
			buffer[2] = (unsigned char)((g[1] >> 6) & 15);
			buffer[2] |= (unsigned char)((g[2] & 15) << 4);
			buffer[3] = (unsigned char)((g[2] >> 4) & 63);
			buffer[3] |= (unsigned char)((g[3] & 3));
			buffer[4] = (unsigned char)((g[3] >> 2) & 255);
			if (fwrite(buffer, 1, 5, fp) != 5)
			{
				printf("Read RAW16 File from %s Error!!!\n", pFileName);
				fclose(fp);
				return false;
			}
		}
		fclose(fp);
		return true;
	}
	else
	{
		printf("Unknonw MIPI Format=%d!!!\n", nMIPIRAW);
		return false;
	}
}
bool MultiUshortImage::Save16BitRawToBitmapFile(char *pFileName, bool bBlackWhite)
{
	int i, x, y, c, CFA[4], bgr[3];
	int nShift = (m_nRawBits > 8) ? (m_nRawBits - 8) : (0);
	MultiUcharImage *pBGRImage = new MultiUcharImage;
	if (!pBGRImage->CreateImage(m_nWidth, m_nHeight))
	{
		return false;
	}
	if (bBlackWhite)
	{
		for (y = 0; y < m_nHeight; y++)
		{
			unsigned char *pbLine = pBGRImage->GetImageLine(y);
			unsigned short *pwLine = GetImageLine(y);
			for (x = 0; x < m_nWidth; x++)
			{
				c = *(pwLine++);
				c = (c >> nShift);
				if (c > 255)c = 255;
				*(pbLine++) = (unsigned char)c;
				*(pbLine++) = (unsigned char)c;
				*(pbLine++) = (unsigned char)c;
			}
		}
	}
	else
	{
		int nYFlag = (m_nRawCFA & 2);
		int nXFlag = (m_nRawCFA & 1);
		for (y = 0; y < m_nHeight; y += 2)
		{
			unsigned char *pbLine = pBGRImage->GetImageLine(y);
			unsigned short *pwLine = GetImageLine(y);
			for (x = 0; x < m_nWidth; x += 2)
			{
				CFA[0] = pwLine[0];			CFA[1] = pwLine[1];
				CFA[2] = pwLine[m_nWidth];	CFA[3] = pwLine[m_nWidth + 1];
				bgr[0] = CFA[nYFlag + nXFlag];
				bgr[1] = CFA[((nYFlag >> 1) ^ nXFlag) ^ 1];
				bgr[2] = CFA[(nYFlag ^ 2) + (nXFlag ^ 1)];
				for (i = 0; i < 3; i++)
				{
					bgr[i] = (bgr[i] >> nShift);
					if (bgr[i] > 255)bgr[i] = 255;
				}
				for (i = 0; i < 3; i++)
				{
					*(pbLine++) = (unsigned char)bgr[i];
				}
				for (i = 0; i < 3; i++)
				{
					*(pbLine++) = (unsigned char)bgr[i];
				}
				pwLine += 2;
			}
			pbLine = pBGRImage->GetImageLine(y + 1);
			pwLine = GetImageLine(y);
			for (x = 0; x < m_nWidth; x += 2)
			{
				CFA[0] = pwLine[0];			CFA[1] = pwLine[1];
				CFA[2] = pwLine[m_nWidth];	CFA[3] = pwLine[m_nWidth + 1];
				bgr[0] = CFA[nYFlag + nXFlag];
				bgr[1] = CFA[2 + ((nYFlag >> 1) ^ nXFlag)];
				bgr[2] = CFA[(nYFlag ^ 2) + (nXFlag ^ 1)];
				for (i = 0; i < 3; i++)
				{
					bgr[i] = (bgr[i] >> nShift);
					if (bgr[i] > 255)bgr[i] = 255;
				}
				for (i = 0; i < 3; i++)
				{
					*(pbLine++) = (unsigned char)bgr[i];
				}
				for (i = 0; i < 3; i++)
				{
					*(pbLine++) = (unsigned char)bgr[i];
				}
				pwLine += 2;
			}
		}
	}
	if (pBGRImage == NULL)return false;
	if (!pBGRImage->SaveBGRToBitmapFile(pFileName))
	{
		delete pBGRImage;
		return false;
	}
	delete pBGRImage;
	return true;
}
bool MultiUshortImage::MeanImage5x5()
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nChannel = GetImageDim();
	unsigned int *pHLines[5];
	int nThread = omp_get_num_procs();
	unsigned int *pBuffer = new unsigned int[nWidth*nChannel * 5 * nThread];
	if (pBuffer == NULL)return false;
	int  loop = 0;
	int strnWidth = nWidth * nChannel;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 5 * nWidth*nThreadId*nChannel;
			pHLines[1] = pHLines[0] + strnWidth;
			pHLines[2] = pHLines[1] + strnWidth;
			pHLines[3] = pHLines[2] + strnWidth;
			pHLines[4] = pHLines[3] + strnWidth;
			HAvg5Line(GetImageLine(y-2), pHLines[0], nChannel, nWidth);
			HAvg5Line(GetImageLine(y - 1), pHLines[1], nChannel, nWidth);
			HAvg5Line(GetImageLine(y - 0), pHLines[2], nChannel, nWidth);
			HAvg5Line(GetImageLine(y +1), pHLines[3], nChannel, nWidth);
			loop++;
		}
		HAvg5Line(GetImageLine(y +2), pHLines[4], nChannel, nWidth);
		VAvg5Line(pHLines, GetImageLine(y), nChannel, nWidth);
		unsigned int  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pHLines[3];
		pHLines[3] = pHLines[4];
		pHLines[4] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}
bool MultiUshortImage::MeanImage3x3()
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nChannel = GetImageDim();
	unsigned short *pHLines[3];
	int nThread = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[nWidth*nChannel * 3 * nThread];
	if (pBuffer == NULL)return false;
	int  loop = 0;
	#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 3 * nWidth*nThreadId*nChannel;
			pHLines[1] = pHLines[0] + nWidth * nChannel;
			pHLines[2] = pHLines[1] + nWidth * nChannel;
			HAvg3Line(GetImageLine(y-1), pHLines[0], nChannel, nWidth);
			HAvg3Line(GetImageLine(y), pHLines[1], nChannel, nWidth);
			loop++;
		}
		HAvg3Line(GetImageLine(y+1), pHLines[2], nChannel, nWidth);
		VAvg3Line(pHLines, GetImageLine(y), nChannel, nWidth);
		unsigned short  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}
bool MultiUshortImage::GetSingleChannelImage(MultiUshortImage * pOutImage,int ch)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight,1,16))return false;
	}
	int nProcs = omp_get_num_procs();
#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned short *pOutLine = pOutImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			*pOutLine++ = pInLine[ch];
			pInLine += nCh;
		}
	}
	return true;
}
bool MultiUshortImage::ReplaceSingleChannelImage(MultiUshortImage * pInputImage,int ch)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pInputImage->GetImageWidth() != nWidth || pInputImage->GetImageHeight() != nHeight)
	{
		if (!pInputImage->CreateImage(nWidth, nHeight,1,16))return false;
	}
	int nProcs = omp_get_num_procs();
#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pOutLine = GetImageLine(y);
		unsigned short *pInline = pInputImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pOutLine[ch] = *pInline++;
			pOutLine += nCh;
		}
	}
	return true;
}
bool MultiUshortImage::Bilateral5x5SingleImage(int nThre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	MultiUshortImage pOutImage;
	if (pOutImage.GetImageWidth() != nWidth || pOutImage.GetImageHeight() != nHeight)
	{
		if (!pOutImage.CreateImage(nWidth, nHeight,1,16))return false;
	}
	const int nMask[5][5] =
	{
		{ 1,  4,  6,  4, 1},
		{ 4, 16, 24, 16, 4},
		{ 6, 24, 36, 24, 6},
		{ 4, 16, 24, 16, 4},
		{ 1,  4,  6,  4, 1}
	};
	unsigned int nInvNoise = (1U << 28) / (nThre*nThre);
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) 
	for (int y = 0; y < nHeight; y++)
	{
		int i, j, x, Y0, Y;
		unsigned short *pIn = GetImageLine(y);
		unsigned short *pOut = pOutImage.GetImageLine(y);
		for (x = 0; x < nWidth; x++)
		{
			long long sumY, dY;
			int W, sumW;
			Y0 = *pIn;
			sumY = 0;
			sumW = 0;
			for (i = -2; i <= 2; i++)
			{
				if (y + i < 0 || y + i >= nHeight)continue;
				for (j = -2; j <= 2; j++)
				{
					if (x + j < 0 || x + j >= nWidth)continue;
					Y = pIn[i*nWidth + j];
					dY = Y - Y0;
					dY *= dY;
					W = (int)(8 - ((dY*nInvNoise) >> 28));
					if (W >= 0)
					{
						W = nMask[i + 2][j + 2] << W;
						sumY += Y * W;
						sumW += W;
					}
				}
			}
			*(pOut++) = (int)(sumY / sumW);
			pIn++;
		}
	}
	memcpy(GetImageData(), pOutImage.GetImageData(),nWidth*nHeight*2);
	return true;
}
void MultiUshortImage::GetMultiImageIntegralUSData(CImageData_UINT32 *Integral, int Width, int Height,int dim)
{
	unsigned int *ColSum = (unsigned int *)calloc(Width*dim, sizeof(unsigned int));        //    用的calloc函数哦，自动内存清0
	memset(Integral->GetImageData(), 0, (Width + 1)*dim * sizeof(unsigned int));
	for (int y = 0; y < Height; y++)
	{
		unsigned short *LinePS = GetImageLine(y);// pInData + y * Stride;
		unsigned int *LinePD = Integral->GetImageLine(y + 1) + dim;// +(y + 1) * (Width + 1) + 1;//积分图1 1开始一行起始
		for (int k=0;k<dim;k++)
		{
			LinePD[-dim+k] = 0;
		}
		for (int x = 0; x < Width; x++)//行方向的积分累加
		{
			for (int k=0;k<dim;k++)
			{
				ColSum[x*dim+k] += LinePS[x*dim+k];//ColSum统计的是的累加LinePS统计行的累加
				LinePD[x*dim+k] = LinePD[(x - 1)*dim+k] + ColSum[x*dim+k];
			}
		}
	}
	free(ColSum);
}
bool MultiUshortImage::GetEachBlockAverageValue(int nRadius)
{
	int Width = GetImageWidth();
	int Height = GetImageHeight();
	int dim = GetImageDim();
	int Imagesize = Width * Height*dim * sizeof(unsigned short);
	CImageData_UINT32 Integral;
	Integral.SetImageSize((Width + 1), (Height + 1), dim);
	this->GetMultiImageIntegralUSData(&Integral, Width, Height, dim);
	int nProcs = omp_get_num_procs();
#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int Y = 0; Y < Height; Y++)
	{
		int Y1 = max(Y - nRadius, 0);
		int Y2 = min(Y + nRadius + 1, Height - 1);
		unsigned int *LineP1 = Integral.GetImageLine(Y1);
		unsigned int *LineP2 = Integral.GetImageLine(Y2);
		unsigned short *LinePD = GetImageLine(Y);
		for (int X = 0; X < Width; X++)
		{
			int X1 = max(X - nRadius, 0);
			int X2 = min(X + nRadius + 1, Width);
			unsigned int PixelCount = (X2 - X1) * (Y2 - Y1);
			for (int k=0;k< dim;k++)
			{
				unsigned int Sum = LineP2[X2*dim+k] - LineP1[X2*dim+k] - LineP2[X1*dim + k] + LineP1[X1*dim + k];
				LinePD[X*dim + k] = (Sum + (PixelCount >> 1)) / PixelCount;
			}
		}
	}
	return true;
}
bool MultiUshortImage::SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset)
{
	int x, y, Y, tY, E;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	SingleUcharImage YImage;
	if (!YImage.CreateImage(nWidth, nHeight))return false;
	if (nChannel < 0)nChannel = 0;
	if (nChannel >= nCh)nChannel = nCh - 1;
	E = 0;
	for (y = 0; y < nHeight; y++)
	{
		unsigned short *pInLine = GetImageLine(y);
		unsigned char *pOutLine = YImage.GetImageLine(y);
		for (x = 0; x < nWidth; x++)
		{
			Y = pInLine[nChannel];	
			Y = Y * nOutScale + E;
			tY = Y / nInScale;
			E = Y - tY * nInScale;
			Y = tY + nOffset;
			if (Y < 0)Y = 0;	if (Y > 255)Y = 255;
			*(pOutLine++) = (unsigned char)Y;
			pInLine += nCh;
		}
	}
	return YImage.SaveGrayToBitmapFile(pFileName);
}