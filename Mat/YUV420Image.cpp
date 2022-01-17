#include "YUV420Image.h"
bool Yuv420Image::CreateImage(int nWidth, int nHeight)
{
	nWidth = (nWidth >> 1) << 1;
	nHeight = (nHeight >> 1) << 1;
	SetImageSize(nWidth, nHeight, 3);
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_pYImage = m_pImgData;
	m_pUVImage = m_pImgData + m_nWidth * m_nHeight;
	return true;
}
bool Yuv420Image::CreateImageWithData(int nWidth, int nHeight, unsigned char * pInputData)
{
	nWidth = (nWidth >> 1) << 1;
	nHeight = (nHeight >> 1) << 1;
	SetImageSize(nWidth, nHeight, 3);
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_pYImage = m_pImgData;
	m_pUVImage = m_pImgData + m_nWidth * m_nHeight;
	memcpy(m_pImgData, pInputData, nWidth*nHeight * 3 / 2);
	return false;
}
bool Yuv420Image::Clone(Yuv420Image *pInImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	if (!CreateImage(nWidth, nHeight))return false;
	memcpy(m_pYImage, pInImage->GetYData(), nWidth*nHeight);
	memcpy(m_pUVImage, pInImage->GetUVData(), (nWidth >> 1)*(nHeight >> 1) * 2);
	return true;
}
bool Yuv420Image::YUV444ToYUV420(MultiUcharImage *pInputYUV444Image)
{
	if(!CreateImage(pInputYUV444Image->GetImageWidth(), pInputYUV444Image->GetImageHeight()))return false;
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread)
	for(int y=0; y<m_nHeight; y+=2)
	{
		int UV[2];
		unsigned char *pInLine0= pInputYUV444Image->GetImageLine(y);
		unsigned char *pInLine1= pInputYUV444Image->GetImageLine(y+1);
		unsigned char *pYLine0=GetYLine(y);
		unsigned char *pYLine1=GetYLine(y+1);	
		unsigned char *pUVLine=GetUVLine(y);
		for(int x=0; x<m_nWidth; x+=2)
		{
			UV[0]=UV[1]=0;
			*(pYLine0++)=pInLine0[0];
			UV[0]+=pInLine0[1];
			UV[1]+=pInLine0[2];
			pInLine0+=3;
			*(pYLine0++)=pInLine0[0];
			UV[0]+=pInLine0[1];
			UV[1]+=pInLine0[2];
			pInLine0+=3;
			*(pYLine1++)=pInLine1[0];
			UV[0]+=pInLine1[1];
			UV[1]+=pInLine1[2];
			pInLine1+=3;
			*(pYLine1++)=pInLine1[0];
			UV[0]+=pInLine1[1];
			UV[1]+=pInLine1[2];
			pInLine1+=3;
			UV[0]>>=2;	
			UV[1]>>=2;	
			*(pUVLine++)=(unsigned char) UV[0];
			*(pUVLine++)=(unsigned char) UV[1];
		}
	}
	return true;
}
bool Yuv420Image::YUV420ToYUV444(MultiUcharImage *pOutYUV444Image, bool bSimple)
{
	if(!pOutYUV444Image->CreateImage(m_nWidth, m_nHeight))return false;
	unsigned short *pUVLines[2];
	int nThread = omp_get_num_procs();
	unsigned short *pUVLineBuffer=new unsigned short[m_nWidth*4* nThread];
	if(pUVLineBuffer==NULL)return false;
	pUVLines[0]=pUVLineBuffer;
	pUVLines[1]=pUVLineBuffer+m_nWidth*2;
	unsigned char *pUVLine=GetUVLine(0);
	HUV2ToUV4Line(pUVLine, pUVLines[0], m_nWidth);
	{
		unsigned char *pYUVLine= pOutYUV444Image->GetImageLine(0);
		unsigned char *pYLine=GetYLine(0);
		unsigned short *pUV=pUVLines[0];
		for(int x=0; x<m_nWidth; x++)
		{
			pYUVLine[0]=*(pYLine++);
			pYUVLine[1]=(unsigned char) (*(pUV++)>>2);
			pYUVLine[2]=(unsigned char) (*(pUV++)>>2);
			pYUVLine+=3;
		}
	}
	int loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pUVLines) 
	for(int y=1; y<m_nHeight-1; y+=2)
	{
		if (loop==0)
		{
			int nThreadId = omp_get_thread_num();
			pUVLines[0] = pUVLineBuffer + 4 * m_nWidth*nThreadId;
			pUVLines[1] = pUVLines[0] + m_nWidth * 2;
			HUV2ToUV4Line(GetUVLine(y), pUVLines[0], m_nWidth);
			loop++;
		}
		int  UV[2];
		HUV2ToUV4Line(GetUVLine(y + 1), pUVLines[1], m_nWidth);
		unsigned char *pYUVLine= pOutYUV444Image->GetImageLine(y);
		unsigned char *pYLine=GetYLine(y);
		unsigned short *pUV0=pUVLines[0];
		unsigned short *pUV1=pUVLines[1];
		for(int x=0; x<m_nWidth; x++)
		{
			pYUVLine[0]=*(pYLine++);
			UV[0]=*(pUV0++);
			UV[1]=*(pUV1++);
			UV[0]=(UV[0]*3+UV[1]+8)>>4;
			pYUVLine[1]=(unsigned char) UV[0];
			UV[0]=*(pUV0++);
			UV[1]=*(pUV1++);
			UV[0]=(UV[0]*3+UV[1]+8)>>4;
			pYUVLine[2]=(unsigned char) UV[0];
			pYUVLine+=3;
		}
		pYUVLine= pOutYUV444Image->GetImageLine(y+1);
		pYLine=GetYLine(y+1);
		pUV0=pUVLines[0];
		pUV1=pUVLines[1];
		for(int x=0; x<m_nWidth; x++)
		{
			pYUVLine[0]=*(pYLine++);
			UV[0]=*(pUV0++);
			UV[1]=*(pUV1++);
			UV[0]=(UV[0]+UV[1]*3+8)>>4;
			pYUVLine[1]=(unsigned char) UV[0];
			UV[0]=*(pUV0++);
			UV[1]=*(pUV1++);
			UV[0]=(UV[0]+UV[1]*3+8)>>4;
			pYUVLine[2]=(unsigned char) UV[0];
			pYUVLine+=3;
		}
		unsigned short *pTemp=pUVLines[0];
		pUVLines[0]=pUVLines[1];
		pUVLines[1]=pTemp;
	}
	{
		unsigned char *pYUVLine= pOutYUV444Image->GetImageLine(m_nHeight-1);
		unsigned char *pYLine=GetYLine(m_nHeight-1);
		unsigned short *pUV=pUVLines[0];
		for(int x=0; x<m_nWidth; x++)
		{
			pYUVLine[0]=*(pYLine++);
			pYUVLine[1]=(unsigned char) (*(pUV++)>>2);
			pYUVLine[2]=(unsigned char) (*(pUV++)>>2);
			pYUVLine+=3;
		}
	}
	delete[] pUVLineBuffer;
	return true;
}	
void Yuv420Image::HUV2ToUV4Line(unsigned char *pInLine, unsigned short *pOutLine, int nWidth)
{
	int x, U0, V0, U1, V1, UV[2];
	U0=*(pInLine++);
	V0=*(pInLine++);
	pOutLine[0]=(unsigned short) (U0<<2);
	pOutLine[1]=(unsigned short) (V0<<2);
	pOutLine+=2;
	for(x=1; x<nWidth-1; x+=2)
	{
		U1=*(pInLine++);
		V1=*(pInLine++);	
		UV[0]=(U0*3+U1);
		UV[1]=(V0*3+V1);
		pOutLine[0]=(unsigned short) UV[0];
		pOutLine[1]=(unsigned short) UV[1];
		pOutLine+=2;
		UV[0]=(U0+U1*3);
		UV[1]=(V0+V1*3);
		pOutLine[0]=(unsigned short) UV[0];
		pOutLine[1]=(unsigned short) UV[1];
		pOutLine+=2;
		U0=U1;
		V0=V1;
	}
	pOutLine[0]=(unsigned short) (U0<<2);
	pOutLine[1]=(unsigned short) (V0<<2);
	pOutLine+=2;
}
bool Yuv420Image::GetYImage(SingleUcharImage *pYImage)
{
	if (!pYImage->CreateImage(m_nWidth, m_nHeight))return false;
	memcpy(pYImage->GetImageData(), m_pYImage, m_nWidth*m_nHeight);
	return true;
}
bool Yuv420Image::GetUImage(SingleUcharImage *pUImage)
{
	if (!pUImage->CreateImage(m_nWidth>>1, m_nHeight>>1))return false;
	int nWidthUV = (m_nWidth >> 1);
	int nHeightUV = (m_nHeight >> 1);
	#pragma omp parallel for
	for (int y = 0; y < nHeightUV; y++)
	{
		unsigned char *pIn = m_pUVImage + y * m_nWidth;
		unsigned char *pOut = pUImage->GetImageLine(y);
		for (int x = 0; x < nWidthUV; x++)
		{
			*(pOut++) = pIn[0];
			pIn += 2;
		}
	}
	return true;
}
bool Yuv420Image::GetVImage(SingleUcharImage *pVImage)
{
	if (!pVImage->CreateImage(m_nWidth >> 1, m_nHeight >> 1))return false;
	int nWidthUV = (m_nWidth >> 1);
	int nHeightUV = (m_nHeight >> 1);
	#pragma omp parallel for
	for (int y = 0; y < nHeightUV; y++)
	{
		unsigned char *pIn = m_pUVImage + y * m_nWidth;
		unsigned char *pOut = pVImage->GetImageLine(y);
		for (int x = 0; x < nWidthUV; x++)
		{
			*(pOut++) = pIn[1];
			pIn += 2;
		}
	}
	return true;
}
bool Yuv420Image::UpdateUVImage(SingleUcharImage *pUImage, SingleUcharImage *pVImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nWidthUV = (m_nWidth >> 1);
	int nHeightUV = (m_nHeight >> 1);
	#pragma omp parallel for
	for (int y = 0; y < nHeightUV; y++)
	{
		unsigned char *pOut = m_pUVImage + y * m_nWidth;
		unsigned char *pInUline = pUImage->GetImageLine(y);
		unsigned char *pInVline = pVImage->GetImageLine(y);
		for (int x = 0; x < nWidthUV; x++)
		{
			*(pOut++) = *pInUline++;
			*(pOut++) = *pInVline++;
		}
	}
	return true;
}
bool Yuv420Image::UpdateYUVImage(SingleUcharImage *pYImage,SingleUcharImage *pUImage, SingleUcharImage *pVImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nWidthUV = (m_nWidth >> 1);
	int nHeightUV = (m_nHeight >> 1);
	memcpy(m_pImgData, pYImage->GetImageData(), nWidth*nHeight);
#pragma omp parallel for
	for (int y = 0; y < nHeightUV; y++)
	{
		unsigned char *pOutUV = m_pUVImage + y * m_nWidth;
		unsigned char *pInUline = pUImage->GetImageLine(y);
		unsigned char *pInVline = pVImage->GetImageLine(y);
		for (int x = 0; x < nWidthUV; x++)
		{
			*(pOutUV++) = *pInUline++;
			*(pOutUV++) = *pInVline++;
		}
	}
	return true;
}
bool Yuv420Image::UpdateYImage(SingleUcharImage *pYImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	memcpy(m_pImgData, pYImage->GetImageData(), nWidth*nHeight);
	return true;
}
bool Yuv420Image::CombineYCbCr(SingleUcharImage *pInputYImage, MultiUcharImage *pOutYCbCr)
{
	int nWidth = pInputYImage->GetImageWidth();
	int nHeight = pInputYImage->GetImageHeight();
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pYLine = pInputYImage->GetImageLine(y);
		unsigned char *pUVLine = GetUVLine(2 * y);
		unsigned char *pYUVLine = pOutYCbCr->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pYUVLine[0] = pYLine[0];
			pYUVLine[2] = pUVLine[0];
			pYUVLine[1] = pUVLine[1];
			pYLine++;
			pUVLine += 2;
			pYUVLine += 3;
		}
	}
	return true;
}
bool Yuv420Image::LoadBitmapFileToYUV420(char *filename)
{
	MultiUcharImage BGRImage;
	if (!BGRImage.LoadBitmapFileToBGR(filename))
	{
		printf("Can not load from image %s!!!\n", filename);
		return false;
	}
	BGRImage.BGR2YCbCr();
	if (!this->YUV444ToYUV420(&BGRImage))return false;
	return true;
}
bool Yuv420Image::SaveYuvToBitmapFile(char *filename)
{
	MultiUcharImage BGRImage;
	if (!YUV420ToYUV444(&BGRImage))return false;
	BGRImage.YCbCr2BGR();
	if (!BGRImage.SaveBGRToBitmapFile(filename)) return false;
	return true;
}
bool Yuv420Image::SaveYToBitmapFile(char *filename)
{
	SingleUcharImage Img;
	if (!Img.CreateImage(m_nWidth, m_nHeight))return false;
	memcpy(Img.GetImageData(), GetYData(), m_nWidth*m_nHeight);
	if (!Img.SaveGrayToBitmapFile(filename))return false;
	return true;
}
bool Yuv420Image::SaveYuvToJpegFile(char *filename,int Quality)
{
	SingleUcharImage Ydata;
	SingleUcharImage Udata;
	SingleUcharImage Vdata;
	SingleUcharImage jpgdata;
	Ydata.CreateImage(m_nWidth, m_nHeight);
	Udata.CreateImage(m_nWidth, m_nHeight);
	Vdata.CreateImage(m_nWidth, m_nHeight);
	jpgdata.CreateImage(m_nWidth, m_nHeight);
	Yuv420Image YUVImage;
	YUVImage.Clone(this);
	unsigned long jpegSize = 0;
	YUVImage.GetYImage(&Ydata);
	YUVImage.GetUImage(&Udata);
	YUVImage.GetVImage(&Vdata);
	YUV2Jpg(Ydata.GetImageData(), Udata.GetImageData(), Vdata.GetImageData(), m_nWidth, m_nHeight, Quality, jpgdata.GetImageData(), &jpegSize);
	FILE *fp = fopen(filename, "wb");
	fwrite(jpgdata.GetImageData(), jpegSize, 1, fp);
	fclose(fp);
	return true;
}