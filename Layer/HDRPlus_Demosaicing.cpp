#include "HDRPlus_Demosaicing.h"
bool CHDRPlus_Demosaicing::Forward(MultiUshortImage *pInRAWImage, MultiUshortImage *pOutRGBImage, TGlobalControl *pControl)
{
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
			case GRBG:

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
			case BGGR:
				//R--at--B
				//G--at--B
				//B--at--B
				//BGBG
				//GRGR
				//BGBG
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

				break;
			default:
				printf("Please enter correctly RawTYPE\n");
				break;
			}

			pRGBline0[0] = CLIP(RGB0[0], 0, 65535);
			pRGBline0[1] = CLIP(RGB0[1], 0, 65535);
			pRGBline0[2] = CLIP(RGB0[2], 0, 65535);
			pRGBline0[3] = CLIP(RGB0[3], 0, 65535);
			pRGBline0[4] = CLIP(RGB0[4], 0, 65535);
			pRGBline0[5] = CLIP(RGB0[5], 0, 65535);
			pRGBline1[0] = CLIP(RGB1[0], 0, 65535);
			pRGBline1[1] = CLIP(RGB1[1], 0, 65535);
			pRGBline1[2] = CLIP(RGB1[2], 0, 65535);
			pRGBline1[3] = CLIP(RGB1[3], 0, 65535);
			pRGBline1[4] = CLIP(RGB1[4], 0, 65535);
			pRGBline1[5] = CLIP(RGB1[5], 0, 65535);
			pRGBline0 += 6;
			pRGBline1 += 6;
		}
	}
	return true;
}