#ifndef __MAT_DATA_H_
#define __MAT_DATA_H_
#include "SubFunction.h"
#include "Common.h"
#include "MemPool.h"
#include "YUVToJpeg.h"
#ifndef _WIN32
#define BI_RGB        0L
typedef struct __attribute__((packed)) {
	unsigned short    bfType;
	unsigned long     bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long   bfOffBits;
} BITMAPFILEHEADER;
typedef struct __attribute__((packed)) {
	unsigned long       biSize;
	long				biWidth;
	long				biHeight;
	unsigned short      biPlanes;
	unsigned short      biBitCount;
	unsigned long       biCompression;
	unsigned long       biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	unsigned long      biClrUsed;
	unsigned long      biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagRGBQUAD {
	unsigned char    rgbBlue;
	unsigned char    rgbGreen;
	unsigned char    rgbRed;
	unsigned char    rgbReserved;
} RGBQUAD;
typedef struct __attribute__((packed)) {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO;
#endif

template <class T> class CMat
{
protected:
	int m_nWidth;
	int m_nHeight;
	int m_nChannel;
	int m_nSize;
	T *m_pImgData;
	size_t m_nSizeInBytes;
public:	
	CMat()
	{
		m_nSizeInBytes = m_nWidth = m_nHeight = m_nChannel = m_nSize= 0;
		m_pImgData=NULL;
	}
	~CMat()
	{
		if (m_pImgData != NULL)
			MemPool::Deallocate(m_pImgData);
		m_pImgData = NULL;
	}
	bool SetImageSize(int W, int H, int D)
	{
		size_t SizeInBytes = W * H * D * sizeof(T) + 128;
		if (SizeInBytes > m_nSizeInBytes)
		{
			ClearMem();
			m_pImgData = (T*)MemPool::Allocate(SizeInBytes);
			if (m_pImgData == NULL) return false;
			m_nSizeInBytes = SizeInBytes;
		}
		m_nWidth = W;
		m_nHeight = H;
		m_nChannel = D;
		m_nSize = W * H*D;
		return true;
	}
	void ClearMem()
	{
		if (m_pImgData != NULL)
		{
			MemPool::Deallocate(m_pImgData);
		}
		m_pImgData = NULL;
		m_nSizeInBytes = m_nWidth = m_nHeight = m_nChannel = m_nSize= 0;
	}	
	inline int GetImageWidth(){return m_nWidth;}  
	inline int GetImageHeight(){return m_nHeight;}
	inline int GetImagePitch(){return m_nWidth*m_nChannel;}
	inline int GetImageDim(){return m_nChannel;}
	inline int GetImageSize() { return m_nSize; }
	inline T * GetImageData(){return m_pImgData;}
	inline T * GetImageLine(int nY)
	{
		if(nY<0)nY=0;	if(nY>=m_nHeight)nY=m_nHeight-1;
		return m_pImgData+nY*m_nWidth*m_nChannel;
	}
	inline T * GetImagePixel(int nX, int nY)
	{
		if (nY < 0)nY = 0;	if (nY >= m_nHeight)nY = m_nHeight - 1;
		if (nX <0 )nX = 0;	if (nX >= m_nWidth)nX = m_nWidth - 1;
		int pos = (nY*m_nWidth + nX)*m_nChannel;
		return m_pImgData + pos;
	}
	inline T GetImagePosValue(int nY, int nX, int nC)
	{
		if (nY < 0)nY = 0;	if (nY >= m_nHeight)nY = m_nHeight - 1;
		if (nX < 0)nX = 0;	if (nX >= m_nWidth)nX = m_nWidth - 1;
		if (nC < 0)nC = 0;	if (nC >= m_nChannel)nC = m_nChannel - 1;
		int pos = (nY*m_nWidth + nX)*m_nChannel + nC;
		return m_pImgData[pos];
	}
	inline void SetImagePosValue(int nY, int nX, int nC, T Value)
	{
		if (nY < 0)nY = 0;	if (nY >= m_nHeight)nY = m_nHeight - 1;
		if (nX < 0)nX = 0;	if (nX >= m_nWidth)nX = m_nWidth - 1;
		if (nC < 0)nC = 0;	if (nC >= m_nChannel)nC = m_nChannel - 1;
		m_pImgData[(nY*m_nWidth + nX)*m_nChannel + nC] = Value;
	}
	inline void GetImagePixelBlock(int left, int right, int top, int bottom, T* oBuf)
	{
		if (left >= 0 && right < m_nWidth)
		{
			int num = (right - left + 1)*m_nChannel;
			for (int i = top; i <= bottom; i++)
			{
				int ii = i < 0 ? 0 : i >= m_nHeight ? m_nHeight - 1 : i;
				memcpy(oBuf, m_pImgData + (ii*m_nWidth + left)*m_nChannel, num * sizeof(T));
				oBuf += num;
			}
		}
		else
		{
			int num = (right - left + 1)*m_nChannel;
			for (int i = top; i <= bottom; i++)
			{
				int ii = i < 0 ? 0 : i >= m_nHeight ? m_nHeight - 1 : i;
				if (m_nChannel == 1)
				{
					for (int j = left; j <= right; j++)
					{
						int jj = (j < 0) ? 0 : (j >= m_nWidth) ? m_nWidth - 1 : j;
						oBuf[0] = m_pImgData[(ii*m_nWidth + jj)]; oBuf++;
					}
				}
				else
				{
					for (int j = left; j <= right; j++)
					{
						int jj = (j < 0) ? 0 : (j >= m_nWidth) ? m_nWidth - 1 : j;
						for (int n = 0; n < m_nChannel; n++)
						{
							oBuf[0] = m_pImgData[(ii*m_nWidth + jj)*m_nChannel + n]; oBuf++;
						}
					}
				}

			} //for (int i = top; i < bottom; i++)
		}//if (left >= 0 && right < m_nWidth)
		return;
	}
	void FillValue(T Val)
	{
		T *pV = m_pImgData;
		int len = m_nHeight * m_nWidth*m_nChannel;
		int b = len;
		while (b--)
			*pV++ = Val;
	}
	void FillVector(T nChannel[])
	{
		int i, x, y;
		T *pD = m_pImgData;
		if (pD == NULL)return;
		for (y = 0; y < m_nHeight; y++)
		{
			for (x = 0; x < m_nWidth; x++)
			{
				for (i = 0; i < m_nChannel; i++)
				{
					*(pD++) = nChannel[i];
				}
			}
		}
	}
	T GetMaxVal()
	{
		T Max = 0;
		for (int y = 0; y < m_nHeight; y++)
		{
			T *pDline = GetImageLine(y);
			for (int x = 0; x < m_nWidth*m_nChannel; x++)
			{
				if (pDline[0]> Max)
				{
					Max = pDline[0];
				}
				pDline++;
			}
		}
		return Max;
	}
	double GetMeanVal()
	{
		double SumValue = 0;
#pragma omp parallel for reduction(+:SumValue)
		for (int y = 0; y < m_nHeight; y++)
		{
			T *pDline = GetImageLine(y);
			for (int x = 0; x < m_nWidth; x++)
			{
				for (int i = 0; i < m_nChannel; i++)
				{
					SumValue += *(pDline++);
				}
			}
		}
		SumValue = SumValue / (double)(m_nHeight*m_nWidth*m_nChannel);
		return SumValue;
	}
	bool ExtendImage(int nS)
	{
		int nNewWidth = ((m_nWidth + (1 << nS) - 1) >> nS) << nS;
		int nNewHeight = ((m_nHeight + (1 << nS) - 1) >> nS) << nS;
		if (nNewWidth != m_nWidth || nNewHeight != m_nHeight)
		{
			size_t SizeInBytes = nNewWidth * nNewHeight*m_nChannel * sizeof(T) + 128;
			T *pBuffer = (T*)MemPool::Allocate(SizeInBytes);
			if (pBuffer == NULL)return false;
#pragma omp parallel for schedule(dynamic,32)
			for (int y = 0; y < m_nHeight; y++)
			{
				T *pOut = pBuffer + y * nNewWidth*m_nChannel;
				int i, x;
				T *pIn = m_pImgData + y * m_nWidth*m_nChannel;
				int byteCnt = sizeof(T)*m_nWidth*m_nChannel;
				memcpy(pOut, pIn, byteCnt);
				pIn += byteCnt;
				pOut += byteCnt;
				pIn -= 2 * m_nChannel;
				for (x = m_nWidth; x < nNewWidth; x++)
				{
					for (i = 0; i < m_nChannel; i++)
					{
						*(pOut++) = pIn[i];
					}
					pIn -= m_nChannel;
				}
			}
			for (int y = m_nHeight; y < nNewHeight; y++)
			{
				int i, x;
				T *pOut = pBuffer + y * nNewWidth*m_nChannel;
				T *pIn = m_pImgData + (2 * m_nHeight - y - 1)*m_nWidth*m_nChannel;
				for (x = 0; x < m_nWidth; x++)
				{
					for (i = 0; i < m_nChannel; i++)
					{
						*(pOut++) = pIn[i];
					}
					pIn += m_nChannel;
				}
				pIn -= 2 * m_nChannel;
				for (; x < nNewWidth; x++)
				{
					for (i = 0; i < m_nChannel; i++)
					{
						*(pOut++) = pIn[i];
					}
					pIn -= m_nChannel;
				}
			}
			MemPool::Deallocate(m_pImgData);
			m_pImgData = pBuffer;
			m_nWidth = nNewWidth;
			m_nHeight = nNewHeight;
			m_nSizeInBytes = SizeInBytes;
			m_nSize = m_nWidth * m_nHeight*m_nChannel;
		}
		return true;
	}
	bool ReduceImageSize(int nNewWidth, int nNewHeight)
	{
		if (nNewWidth <= m_nWidth && nNewHeight <= m_nHeight)
		{
			if (nNewWidth != m_nWidth || nNewHeight != m_nHeight)
			{
				size_t SizeInBytes = nNewWidth * nNewHeight*m_nChannel * sizeof(T) + 128;
				T *pBuffer = (T*)MemPool::Allocate(SizeInBytes);
				if (pBuffer == NULL)return false;
#pragma omp parallel for schedule(dynamic,32)
				for (int y = 0; y < nNewHeight; y++)
				{
					int i, x;
					T *pOut = pBuffer + y * nNewWidth*m_nChannel;
					T *pIn = m_pImgData + y * m_nWidth*m_nChannel;
					memcpy(pOut, pIn, sizeof(T)*nNewWidth*m_nChannel);
				}
				MemPool::Deallocate(m_pImgData);
				m_pImgData = pBuffer;
				m_nWidth = nNewWidth;
				m_nHeight = nNewHeight;
				m_nSizeInBytes = SizeInBytes;
				m_nSize = m_nWidth * m_nHeight*m_nChannel;
			}
		}
		return true;
	}
	bool CopyImageRect(CMat<T> *pImage, int nLeft, int nTop, int nRight, int nBottom)
	{
		int nWidth = pImage->GetImageWidth();
		int nDim = pImage->GetImageDim();
		if (!SetImageSize(nRight - nLeft, nBottom - nTop, nDim))return false;
#pragma omp parallel for
		for (int y = nTop; y < nBottom; y++)
		{
			int i, x, tx;
			T *pIn = pImage->GetImageLine(y);
			T *pOut = GetImageLine(y - nTop);
			for (x = nLeft; x < nRight; x++)
			{
				if (x < 0)
					tx = 0;
				else if (x >= nWidth)
					tx = nWidth - 1;
				else
					tx = x;
				for (i = 0; i < nDim; i++)
				{
					*(pOut++) = pIn[tx * nDim + i];
				}
			}
		}
		return true;
	}
	bool SetImageRect(CMat<T> *pImage, int nLeft, int nTop)
	{
		int nW = pImage->GetImageWidth();
		int nH = pImage->GetImageHeight();
		if (nLeft + nW > m_nWidth || nTop + nH > m_nHeight)return false;
		if (nLeft < 0 || nTop < 0)return false;
		int i, x, y, tx;
		int nCh = pImage->GetImageDim();
		for (y = nTop; y < nTop + nH; y++)
		{
			T *pIn = pImage->GetImageLine(y - nTop);
			T *pOut = GetImageLine(y) + nCh * nLeft;
			memcpy(pOut, pIn, nW*nCh * sizeof(T));
		}
		return true;
	}
	void HAvgLine(T *pLine, int ch)
	{
		int x;
		T In[3];
		T *pIn = pLine;
		T *pOut = pLine;
		In[0] = In[1] = pIn[ch];	pIn += m_nChannel;
		for (x = 0; x < m_nWidth - 1; x++)
		{
			In[2] = pIn[ch];	pIn += m_nChannel;
			pOut[ch] = Avg3(In[0], In[1], In[2]);
			pOut += m_nChannel;
			In[0] = In[1];
			In[1] = In[2];
		}
		{
			pOut[ch] = Avg3(In[0], In[1], In[2]);
			pOut += m_nChannel;
		}
	}
	bool HAverageOneChannel(int ch)
	{
		int y;
		if (m_pImgData == NULL)return false;
		if (ch < 0)ch = 0;	if (ch >= m_nChannel)ch = m_nChannel - 1;
		for (y = 0; y < m_nHeight; y++)
		{
			T *pLine = GetImageLine(y);
			HAvgLine(pLine, ch);
		}
		return true;
	}
	void GetOneChannelFromLine(T *pInLine, T *pOutLine, int nChannel, int nPitch = 3)
	{
		int x;
		for (x = 0; x < m_nWidth; x++)
		{
			pOutLine[0] = pInLine[nChannel];
			pInLine += m_nChannel;
			pOutLine += nPitch;
		}
	}
	void VAvgLine(T *pInLines[], T *pOutLine, int ch)
	{
		int i, x;
		T In[3];
		T *pIn[3];
		T *pOut = pOutLine;
		for (i = 0; i < 3; i++)
		{
			pIn[i] = pInLines[i];
		}
		for (x = 0; x < m_nWidth; x++)
		{
			for (i = 0; i < 3; i++)
			{
				In[i] = pIn[i][0];
				pIn[i] += 3;
			}
			pOut[ch] = Avg3(In[0], In[1], In[2]);
			pOut += m_nChannel;
		}
	}
	bool VAverageOneChannel(int ch)
	{
		int y;
		if (m_pImgData == NULL || m_nHeight < 3)return false;
		if (ch < 0)ch = 0;	if (ch >= m_nChannel)ch = m_nChannel - 1;
		T *pInLines[3];
		T *pBuffer = new T[m_nWidth * 3];
		if (pBuffer == NULL)return false;
		pInLines[0] = pInLines[1] = pBuffer;
		pInLines[2] = pInLines[1] + 1;
		T *pInLine = GetImageData();
		T *pOutLine = pInLine;
		GetOneChannelFromLine(pInLine, pInLines[1], ch);
		pInLine += m_nWidth * m_nChannel;
		{
			GetOneChannelFromLine(pInLine, pInLines[2], ch);
			pInLine += m_nWidth * m_nChannel;
			VAvgLine(pInLines, pOutLine, ch);
			pOutLine += m_nWidth * m_nChannel;
			pInLines[0] = pInLines[1];
			pInLines[1] = pInLines[2];
			pInLines[2] += 1;
		}
		for (y = 1; y < m_nHeight - 1; y++)
		{
			GetOneChannelFromLine(pInLine, pInLines[2], ch);
			pInLine += m_nWidth * m_nChannel;
			VAvgLine(pInLines, pOutLine, ch);
			pOutLine += m_nWidth * m_nChannel;
			T *pTemp = pInLines[0];
			pInLines[0] = pInLines[1];
			pInLines[1] = pInLines[2];
			pInLines[2] = pTemp;
		}
		{
			pInLines[2] = pInLines[1];
			VAvgLine(pInLines, pOutLine, ch);
			pOutLine += m_nWidth * m_nChannel;
		}
		delete[] pBuffer;
		return true;
	}
	void SaveBIN(char *name)
	{
		FILE *fp = fopen(name, "wb");
		fwrite(m_pImgData, sizeof(T), m_nSize, fp);
		fclose(fp);
	}
	/////matlab functions////////////////
};
typedef CMat<unsigned char> CImageData_UINT8;
typedef CMat<unsigned short> CImageData_UINT16;
typedef CMat<short> CImageData_INT16;
typedef CMat<int> CImageData_INT32;
typedef CMat<unsigned int> CImageData_UINT32;
typedef CMat<uint64_t> CImageData_UINT64;
typedef CMat<int64_t> CImageData_INT64;
typedef CMat<float> CImage_FLOAT;
#endif
