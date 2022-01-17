#ifndef __Subfunction_H__
#define __Subfunction_H__
#include "Basicdef.h"
#define BITVALUE27 ((1<<27)-1)
#define DIFF(x, y) (((x)>(y))?((x)-(y)):((y)-(x)))
#define ABS(x) ((x<0)?-(x):(x))
#define MAXA(x,y) ((ABS(x)>ABS(y))?(x):(y))
#define MIN2(x, y) (((x)<(y))?(x):(y))
#define MAX2(x, y) (((x)>(y))?(x):(y))
#define DIFFSquare(x, y) (((x)-(y))*((x)-(y)))
#define CLIP(x, min, max) ((x)<(min))?(min):((x)>(max)?(max):(x))
#define SAFE_DELETE(ptr) {if(NULL != ptr) delete[] ptr; ptr = NULL;}
#define SWAP2(type,in0,in1) {type pTemp = in0; in0 = in1; in1 = pTemp;}
#define SWAP3(type,in0,in1,in2) {type pTemp = in0;in0 = in1; in1 = in2; in2 = pTemp;}
enum RAWTYPE
{
	GRBG = 0, RGGB, BGGR, GBRG
};
inline int DiffL_Square(int Y0, int Y1)
{
	long long dY = Y0 - Y1;
	dY *= dY;
	if (dY > BITVALUE27)dY = BITVALUE27;
	return (int)dY;
}
inline int DiffS_Square(int Y0, int Y1)
{
	int dY = Y0 - Y1;
	dY *= dY;
	if (dY > BITVALUE27)dY = BITVALUE27;
	return dY;
}
inline int DiffUVL_Square(int Y0, int Y1)
{
	long long dY = Y0 - Y1;
	dY *= dY;
	dY >>= 2;
	if (dY > BITVALUE27)dY = BITVALUE27;
	return (int)dY;
}
inline int DiffUVS_Square(int Y0, int Y1)
{
	int dY = Y0 - Y1;
	dY *= dY;
	dY >>= 2;
	if (dY > BITVALUE27)dY = BITVALUE27;
	return dY;
}
inline int DiffW_Square(unsigned short Y0, unsigned short Y1)
{
	long long dY = Y0 - Y1;
	dY *= dY;
	if (dY > BITVALUE27)dY = BITVALUE27;
	return (int)dY;
}
template <class T>
inline T Min3(T In0, T In1, T In2)
{
	if (In0 < In1)
	{
		if (In2 < In0)
		{
			return In2;
		}
		else
		{
			return In0;
		}
	}
	else
	{
		if (In2 < In1)
		{
			return In2;
		}
		else
		{
			return In1;
		}
	}
}
template <class T>
inline T Max3(T In0, T In1, T In2)
{
	if (In0 < In1)
	{
		if (In2 < In1)
		{
			return In1;
		}
		else
		{
			return In2;
		}
	}
	else
	{
		if (In2 < In0)
		{
			return In0;
		}
		else
		{
			return In2;
		}
	}
}
template <class T>
inline T Max3(T In[])
{
	return MAX2(MAX2(In[0], In[1]), In[2]);
}
template <class T>
inline T Min3(T In[])
{
	return MIN2(MIN2(In[0], In[1]), In[2]);
}
template <class T>
inline T Min4(T C0, T C1, T C2, T C3)
{
	return  MIN2(MIN2(C0, C1), MIN2(C2, C3));
}
template <class T>
inline T Max4(T C0, T C1, T C2, T C3)
{
	return  MAX2(MAX2(C0, C1), MAX2(C2, C3));
}
template <class T>
inline T Max4(T In[])
{
	return MAX2(MAX2(In[0], In[1]), MAX2(In[2], In[3]));
}
template <class T>
inline T Min4(T In[])
{
	return MIN2(MIN2(In[0], In[1]), MIN2(In[2], In[3]));
}
template <class T>
inline T Min5(T C0, T C1, T C2, T C3, T C4)
{
	return  Min3(Min3(C0, C1, C2), C3, C4);
}
template <class T>
inline T Max5(T C0, T C1, T C2, T C3, T C4)
{
	return  Max3(Max3(C0, C1, C2), C3, C4);
}
template <class T>
inline T Avg3(T In0, T In1, T In2)
{
	return (In0 + In1 * 2 + In2 + 2) / 4;
}
template <class T>
inline T Mid(T In[], int Num)
{
	int i, j;
	T  C[81];
	for (i = 0; i < Num; i++)
	{
		C[i] = In[i];
	}
	for (i = 0; i <= Num / 2; i++)
	{
		for (j = i + 1; j < Num; j++)
		{
			if (C[i] > C[j])
			{
				T t = C[i];
				C[i] = C[j];
				C[j] = t;
			}
		}
	}
	return C[Num / 2];
}
template <class T>
void RotateLine(T *pInLine, T *pOutLine, int nWidth, int nPitch)
{
	for (int x = 0; x < nWidth; x++)
	{
		*pOutLine = *(pInLine++);
		pOutLine += nPitch;
	}
}
template <class T>
void FillLine(T *pOutLine, T nVal, int nWidth, int nPitch)
{
	for (int x = 0; x < nWidth; x++)
	{
		*pOutLine = nVal;
		pOutLine += nPitch;
	}
}
inline void RGBToHSV(unsigned char usR, unsigned char usG, unsigned char usB, short &H, short &S, short &V)
{
	unsigned short usMax, usMin, usDx;
	usMax = ((usR > usG) ? usR : usG);
	if (usB > usMax)
	{
		usMax = usB;
	}
	usMin = ((usR < usG) ? usR : usG);
	if (usB < usMin)
	{
		usMin = usB;
	}
	usDx = usMax - usMin;
	if (usDx != 0)
	{
		/* H */
		if (usR == usMax)
		{
			H = 60 * (usG - usB) / usDx;
		}
		else if (usG == usMax)
		{
			H = 60 * (usB - usR) / usDx + 120;
		}
		else
		{
			H = 60 * (usR - usG) / usDx + 240;
		}
		if (H < 0)
		{
			H += 360;
		}
		/* S */
		S = (usDx * 100) / usMax;
	}
	else
	{
		/* H */
		H = 0;
		/* S */
		S = 0;
	}
	/* V */
	V = usMax;
}
inline void RGBToHSVf(float R, float G, float B, float& H, float& S, float&V)
{
	// r,g,b values are from 0 to 1
	// h = [0,360], s = [0,1], v = [0,1]
	// if s == 0, then h = -1 (undefined)
	float min, max, delta, tmp;
	tmp = min(R, G);
	min = min(tmp, B);
	tmp = max(R, G);
	max = max(tmp, B);
	V = max; // v
	delta = max - min;
	if (max != 0)
		S = delta / max; // s
	else
	{
		// r = g = b = 0 // s = 0, v is undefined
		S = 0;
		H = 180;
		//return;
	}
	if (R == max)
		H = (G - B) / delta; // between yellow & magenta
	else if (G == max)
		H = 2 + (B - R) / delta; // between cyan & yellow
	else
		H = 4 + (R - G) / delta; // between magenta & cyan
	H *= 60; // degrees
	if (H < 0)
		H += 360;
}
#endif
