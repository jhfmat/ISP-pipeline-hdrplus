#include "HDRPlus_BlockMatchFusion.h"
#define SCALEBIT 14
#define SCALEVALUE (1<<SCALEBIT)
#define SCALEVALUEHALF (1<<(SCALEBIT-1))
void FastFillvalue(CImageData_UINT32 *pInImage, const int value)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	int nDim = pInImage->GetImageDim();
	int len = nWidth * nDim;
	int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16) 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned int *pline = pInImage->GetImageLine(y);
		int b = len;
		while (b--)
			*pline++ = 0;
	}
}
void CHDRPlus_BlockMatchFusion::FillUnsignedShortImage(unsigned short *pInImage, unsigned short *pOutImage, int nx, int ny, int lenx, int leny)
{
	int nMirrorW = nx + lenx;
	int nMirrorH = ny + leny;
	if (ny < 300)
	{
		for (int y = 0; y < ny; y++)
		{
			unsigned short *pLine = pInImage + y * nx;
			unsigned short *pout = pOutImage + y * nMirrorW;
			memcpy(pout, pLine, nx * 2);
		}
	}
	else
	{
#pragma omp parallel for
		for (int y = 0; y < ny; y++)
		{
			unsigned short *pLine = pInImage + y * nx;
			unsigned short *pout = pOutImage + y * nMirrorW;
			memcpy(pout, pLine, nx * 2);
		}
	}
	/* Bottom */
	for (int y = ny; y < nMirrorH; y += 2)
	{
		unsigned short *pLine0 = pInImage + (nx - 2) * nx;
		unsigned short *pLine1 = pInImage + (nx - 1) * nx;
		unsigned short *pout0 = pOutImage + y * nMirrorW;
		unsigned short *pout1 = pOutImage + (y + 1) * nMirrorW;
		memcpy(pout0, pLine0, nx * 2);
		memcpy(pout1, pLine1, nx * 2);
	}
	/* right */
	for (int y = 0; y < nMirrorH; y++)
	{
		for (int x = nx; x < nMirrorW; x += 2)
		{
			pOutImage[y*nMirrorW + x] = pOutImage[y*nMirrorW + nx - 2];
			pOutImage[y*nMirrorW + x + 1] = pOutImage[y*nMirrorW + nx - 1];
		}
	}
}
bool CHDRPlus_BlockMatchFusion::BoxDownx2(MultiUshortImage *pInImage, MultiUshortImage *pOutImage)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	bool padflag = false;
	if ((nWidth & 1) == 1 || (nHeight & 1) == 1)
	{
		padflag = true;
		if (!pOutImage->CreateImage((nWidth + 1) >> 1, (nHeight + 1) >> 1, 1, 16))return false;
	}
	else
	{
		if (!pOutImage->CreateImage((nWidth) >> 1, (nHeight) >> 1, 1, 16))return false;
	}
	if (padflag == true)
	{
		int nOutWidth = pOutImage->GetImageWidth();
		int nOutHeight = pOutImage->GetImageHeight();
		int padx = nOutWidth * 2 - nWidth;
		int pady = nOutWidth * 2 - nHeight;
		MultiUshortImage tmpPadOut;
		tmpPadOut.CreateImage(nWidth, nHeight, 1, 16);
		tmpPadOut.Extend2Image(pInImage, &tmpPadOut, 1);
		int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16)
		for (int y = 0; y < nOutHeight; y++)
		{
			unsigned short *pline0 = tmpPadOut.GetImageLine(y * 2);
			unsigned short *pline1 = tmpPadOut.GetImageLine(y * 2 + 1);
			unsigned short *pout = pOutImage->GetImageLine(y);
			int tmplen = nOutWidth / 4 * 4;
			int x = 0;
			for (; x < tmplen; x += 4)
			{
				uint32x4_t sum = vpaddlq_u16(vaddq_u16(vld1q_u16(pline0), vld1q_u16(pline1)));
				vst1_u16(pout, vshrn_n_u32(sum, 2));
				pline0 += 8;
				pline1 += 8;
				pout += 4;
			}
			for (; x < nOutWidth; x++)
			{
				pout[0] = ((pline0[x] + pline0[x + 1] + pline1[x] + pline1[x + 1]) >> 2);
				pline0 += 2;
				pline1 += 2;
				pout += 1;
			}
		}
	}
	else
	{
		int nOutWidth = pOutImage->GetImageWidth();
		int nOutHeight = pOutImage->GetImageHeight();
		int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16)
		for (int y = 0; y < nOutHeight; y++)
		{
			unsigned short *pline0 = pInImage->GetImageLine(y * 2);
			unsigned short *pline1 = pInImage->GetImageLine(y * 2 + 1);
			unsigned short *pout = pOutImage->GetImageLine(y);
			int tmplen = nOutWidth / 4 * 4;
			int x = 0;
			for (; x < tmplen; x += 4)
			{
				uint32x4_t sum = vpaddlq_u16(vaddq_u16(vld1q_u16(pline0), vld1q_u16(pline1)));
				vst1_u16(pout, vshrn_n_u32(sum, 2));
				pline0 += 8;
				pline1 += 8;
				pout += 4;
			}
			for (; x < nOutWidth; x++)
			{
				pout[0] = ((pline0[x] + pline0[x + 1] + pline1[x] + pline1[x + 1]) >> 2);
				pline0 += 2;
				pline1 += 2;
				pout += 1;
			}
		}
	}
}
bool CHDRPlus_BlockMatchFusion::EstimatedOffsetNoRef(MultiUshortImage *pInRefImage, MultiUshortImage *pInDebugImage, MultiShortImage *pOutOffsetxImage, MultiShortImage *pOutOffsetyImage, int nMoveRangex, int nMoveRangey)
{
	int Step = 8;
	int Blocksize = 16;
	unsigned int thre = Blocksize * Blocksize * 10;
	const unsigned int InitMinSad = Blocksize * Blocksize*m_nMaxDist * 2;
	int nWidth = pInRefImage->GetImageWidth();
	int nHeight = pInRefImage->GetImageHeight();
	int nWidth8 = nWidth / Step;
	int nHeight8 = nHeight / Step;
	if (pOutOffsetxImage->GetImageWidth() != nWidth8 || pOutOffsetxImage->GetImageHeight() != nHeight8)
	{
		if (!pOutOffsetxImage->CreateImageFillValue(nWidth8, nHeight8, 1, 0))return false;
	}
	if (pOutOffsetyImage->GetImageWidth() != nWidth8 || pOutOffsetyImage->GetImageHeight() != nHeight8)
	{
		if (!pOutOffsetyImage->CreateImageFillValue(nWidth8, nHeight8, 1, 0))return false;
	}
	int NeonBlocksizex = (nWidth - Blocksize);
	int NeonBlocksizey = (nHeight - Blocksize);
	const int Moveystart = nMoveRangey;
	const int Moveyend = nMoveRangey;
	const int Movexstart = nMoveRangex;
	const int Movexend = nMoveRangex;
	int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) //schedule(guided)
	for (int y = 0; y < nHeight; y += Step)
	{
		int Y = y / Step;
		short *NewOffsetxline = pOutOffsetxImage->GetImageLine(Y);
		short *NewOffsetyline = pOutOffsetyImage->GetImageLine(Y);
		for (int x = 0; x < nWidth; x += Step)
		{
			int X = x / Step;
			int Predebugy = y;
			int Predebugx = x;
			short Bestofsetx = 0;
			short Bestofsety = 0;
			unsigned int MinSad = InitMinSad;//
			int summ = 0;
			int sumn = 0;
			int sumnum = 0;
			for (int n = -Moveystart; n <= Moveyend; n++)
			{
				int debugy = Predebugy + n;
				for (int m = -Movexstart; m <= Movexend; m++)
				{
					int debugx = Predebugx + m;
					unsigned int Sad = 0;
					uint32x4_t nsumabs = vdupq_n_u32(0);
					unsigned short *pRef = pInRefImage->GetImagePixel(x, y);
					unsigned short *pDebug = pInDebugImage->GetImagePixel(debugx, debugy);
					if (debugy >= 0 && debugy < NeonBlocksizey && debugx >= 0 && debugx < NeonBlocksizex)
					{
						for (int a = 0; a < Blocksize; a++)
						{
							nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef), vld1q_u16(pDebug))));
							nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef + 8), vld1q_u16(pDebug + 8))));
							pRef += nWidth;
							pDebug += nWidth;
						}
						uint64x2_t nsumsad = vpaddlq_u32(nsumabs);
#ifdef _WIN32
						Sad = nsumsad.m128i_u64[0] + nsumsad.m128i_u64[1];
#else
						Sad = nsumsad[0] + nsumsad[1];
#endif		
					}
					else
					{
						for (int a = 0; a < Blocksize; a++)
						{
							int refy = y + a;
							int debugy = Predebugy + a;
							for (int b = 0; b < Blocksize; b++)
							{
								int refx = x + b;
								int debugx = Predebugx + b;
								Sad += (unsigned int)ABS(pInRefImage->GetImagePixel(refx, refy) - pInDebugImage->GetImagePixel(debugx, debugy));
							}
						}
					}
					if (Sad < thre)
					{
						summ += m;
						sumn += n;
						sumnum++;
					}
					if (Sad < MinSad)
					{
						MinSad = Sad;
						Bestofsetx = m;
						Bestofsety = n;
					}
				}
			}
			if (sumnum != 0)
			{
				Bestofsetx = summ / sumnum;
				Bestofsety = sumn / sumnum;
			}
			*NewOffsetxline++ = Bestofsetx;
			*NewOffsetyline++ = Bestofsety;
		}
	}
	return true;
}
bool CHDRPlus_BlockMatchFusion::EstimatedOffsetAndRef(MultiUshortImage *pInRefImage, MultiUshortImage *pInDebugImage, MultiShortImage *pPreOffsetxImage, MultiShortImage *pPreOffsetyImage, MultiShortImage *pOutOffsetxImage, MultiShortImage *pOutOffsetyImage, int nMoveRangex, int nMoveRangey)
{
	int Step = 8;
	int Blocksize = 16;
	const unsigned int InitMinSad = Blocksize * Blocksize*m_nMaxDist * 2;
	int nWidth = pInRefImage->GetImageWidth();
	int nHeight = pInRefImage->GetImageHeight();
	int nWidth8 = nWidth / Step;
	int nHeight8 = nHeight / Step;
	if (pOutOffsetxImage->GetImageWidth() != nWidth8 || pOutOffsetxImage->GetImageHeight() != nHeight8)
	{
		if (!pOutOffsetxImage->CreateImageFillValue(nWidth8, nHeight8, 1, 0))return false;
	}
	if (pOutOffsetyImage->GetImageWidth() != nWidth8 || pOutOffsetyImage->GetImageHeight() != nHeight8)
	{
		if (!pOutOffsetyImage->CreateImageFillValue(nWidth8, nHeight8, 1, 0))return false;
	}
	int NeonBlocksizex = (nWidth - Blocksize);
	int NeonBlocksizey = (nHeight - Blocksize);
	 int Moveystart = nMoveRangey;
	 int Moveyend = nMoveRangey;
	 int Movexstart = nMoveRangex;
	 int Movexend = nMoveRangex;
	int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs)// schedule(dynamic,16)
	for (int y = 0; y < nHeight; y += Step)
	{
		int Y = y / Step;
		short *PreOffsetxline = pPreOffsetxImage->GetImageLine(Y);
		short *PreOffsetyline = pPreOffsetyImage->GetImageLine(Y);
		short *NewOffsetxline = pOutOffsetxImage->GetImageLine(Y);
		short *NewOffsetyline = pOutOffsetyImage->GetImageLine(Y);
		for (int x = 0; x < nWidth; x += Step)
		{
			int X = x / Step;
			int PreOffsetx = *PreOffsetxline++;
			int PreOffsety = *PreOffsetyline++;
			int Predebugy = y + PreOffsety;
			int Predebugx = x + PreOffsetx;
			//这三个初始值后面重新规划会影响到动态物体的清晰度
			short Bestofsetx = PreOffsety;
			short Bestofsety = PreOffsetx;
			unsigned int MinSad = InitMinSad;//
			for (int n = -Moveystart; n <= Moveyend; n++)
			{
				int debugy = Predebugy + n;
				for (int m = -Movexstart; m <= Movexend; m++)
				{
					int debugx = Predebugx + m;
					unsigned int Sad = 0;
					uint32x4_t nsumabs = vdupq_n_u32(0);
					unsigned short *pRef = pInRefImage->GetImagePixel(x, y);
					unsigned short *pDebug = pInDebugImage->GetImagePixel(debugx, debugy);
					if (debugy >= 0 && debugy < NeonBlocksizey && debugx >= 0 && debugx < NeonBlocksizex)
					{
						for (int a = 0; a < Blocksize; a++)
						{
							nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef), vld1q_u16(pDebug))));
							nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef + 8), vld1q_u16(pDebug + 8))));
							pRef += nWidth;
							pDebug += nWidth;
						}
						uint64x2_t nsumsad = vpaddlq_u32(nsumabs);
#ifdef _WIN32
						Sad = nsumsad.m128i_u64[0] + nsumsad.m128i_u64[1];
#else
						Sad = nsumsad[0] + nsumsad[1];
#endif
					}
					else
					{
						for (int a = 0; a < Blocksize; a++)
						{
							int refy = y + a;
							int debugy = Predebugy + a;
							for (int b = 0; b < Blocksize; b++)
							{
								int refx = x + b;
								int debugx = Predebugx + b;
								Sad += (unsigned int)ABS(pInRefImage->GetImagePixel(refx, refy) - pInDebugImage->GetImagePixel(debugx, debugy));
							}
						}
					}
					if (Sad < MinSad)
					{
						MinSad = Sad;
						Bestofsetx = m;
						Bestofsety = n;
					}
				}
			}
			*NewOffsetxline++ = Bestofsetx + PreOffsetx;
			*NewOffsetyline++ = Bestofsety + PreOffsety;
		}
	}
	return true;
}
bool CHDRPlus_BlockMatchFusion::EstimatedWeight(MultiUshortImage *pInImage, int nFrame, MultiShortImage *pPreOffsetxImage, MultiShortImage *pPreOffsetyImage, CImage_FLOAT *pOutWeightImage)
{
	const int Step = 8;
	const int Blocksize = 16;
	const int Blocksize2 = Blocksize * Blocksize;
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	int NeonBlocksizex = (nWidth - Blocksize);
	int NeonBlocksizey = (nHeight - Blocksize);
	int nProcs = omp_get_num_procs();
	for (int k = 1; k < nFrame; k++)
	{
#pragma omp parallel  for num_threads(nProcs) schedule(dynamic,16)
		for (int y = 0; y < nHeight; y += Step)
		{
			int Y = y / Step;
			short *PreOffsetxline = pPreOffsetxImage[k].GetImageLine(Y);
			short *PreOffsetyline = pPreOffsetyImage[k].GetImageLine(Y);
			float *pOutWeightline = pOutWeightImage[k].GetImageLine(Y);
			float *pOutTotalWeightline = pOutWeightImage[0].GetImageLine(Y);
			for (int x = 0; x < nWidth; x += Step)
			{
				int X = x / Step;
				int PreOffsetx = *PreOffsetxline++;
				int PreOffsety = *PreOffsetyline++;
				int Predebugy = y + PreOffsety;
				int Predebugx = x + PreOffsetx;
				float CurrentAvgSad = 0.0f;
				unsigned int Sad = 0;
				if (Predebugy >= 0 && Predebugy < NeonBlocksizey && Predebugx >= 0 && Predebugx < NeonBlocksizex)
				{
					uint32x4_t nsumabs = vdupq_n_u32(0);
					unsigned short *pRef = pInImage[0].GetImagePixel(x, y);
					unsigned short *pDebug = pInImage[k].GetImagePixel(Predebugx, Predebugy);
					for (unsigned int a = 0; a < Blocksize; a++)
					{
						nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef), vld1q_u16(pDebug))));
						nsumabs = vaddq_u32(nsumabs, vpaddlq_u16(vabdq_u16(vld1q_u16(pRef + 8), vld1q_u16(pDebug + 8))));
						pRef += nWidth;
						pDebug += nWidth;
					}
					uint64x2_t nsad = vpaddlq_u32(nsumabs);
#ifdef _WIN32
					Sad = nsad.m128i_u64[0] + nsad.m128i_u64[1];
#else
					Sad = nsad[0] + nsad[1];
#endif
				}
				else
				{
					for (int a = 0; a < Blocksize; a++)
					{
						int refy = y + a;
						int debugy = Predebugy + a;
						for (int b = 0; b < Blocksize; b++)
						{
							int refx = x + b;
							int debugx = Predebugx + b;
							Sad += (unsigned int)ABS(pInImage[0].GetImagePixel(refx, refy) - pInImage[k].GetImagePixel(Predebugx, Predebugy));
						}
					}
				}
				Sad = Sad >> 8;//Sad / (float)Blocksize2
				CurrentAvgSad = (float)Sad;
				float NormDist = MAX2(1.0f, (float)(CurrentAvgSad - m_nMinDist) / (float)m_nAmountFactor);
				if (NormDist > (m_nMaxDist - m_nMinDist))
				{
					pOutWeightline[X] = 0.f;
				}
				else
				{
					pOutWeightline[X] = 1.f / NormDist;
				}
				pOutTotalWeightline[X] += pOutWeightline[X];
			}
		}
	}
	return true;
}
void CHDRPlus_BlockMatchFusion::MergeTemporal(MultiUshortImage *pRawPadImage, int nFrame, MultiShortImage *pOffsetxImage, MultiShortImage *pOffsetyImage, CImage_FLOAT *pInWeightImage, CImageData_UINT32 *pMergeImage)
{
	const int Step = 16;
	const int Blocksize = 32;
	const int Blocksize2 = Blocksize * Blocksize;
	int nWidth = pRawPadImage->GetImageWidth();
	int nHeight = pRawPadImage->GetImageHeight();
	int NeonBlocksizex = (nWidth - Blocksize);
	int NeonBlocksizey = (nHeight - Blocksize);
	int nProcs = omp_get_num_procs();
#pragma omp parallel  for num_threads(nProcs) schedule(dynamic,16)
	for (int y = 0; y < nHeight; y += Step)
	{
		int Y = y / Step;//
		for (int k = 0; k < nFrame; k++)
		{
			unsigned int *pMergeline = pMergeImage->GetImageLine(Y);
			short *PreOffsetxline = pOffsetxImage[k].GetImageLine(Y);
			short *PreOffsetyline = pOffsetyImage[k].GetImageLine(Y);
			float *pWeightline = pInWeightImage[k].GetImageLine(Y);
			float *pTotalWeight = pInWeightImage[0].GetImageLine(Y);
			for (int x = 0; x < nWidth; x += Step)
			{
				int X = x / Step;
				unsigned short weiget = 0;
				int Newy = 0;
				int Newx = 0;
				if (k == 0)
				{
					weiget = (unsigned short)((float)SCALEVALUE / (*pTotalWeight++));//放大2的14次方
					Newy = y;
					Newx = x;
				}
				else
				{
					weiget = (unsigned short)((float)SCALEVALUE * (*pWeightline++) / (*pTotalWeight++));//放大2的14次方
					Newy = y + (*PreOffsetyline++) * 2;
					Newx = x + (*PreOffsetxline++) * 2;
				}
				if (Newy < NeonBlocksizey && Newx < NeonBlocksizex && Newy >= 0 && Newx >= 0)
				{
					for (int a = 0; a < Blocksize; a++)
					{
						unsigned short *pRawDataline = pRawPadImage[k].GetImageLine(Newy + a);
						for (int b = 0; b < Blocksize; b += 4)
						{
							uint32x4_t npmerge = vld1q_u32(pMergeline);
							npmerge = vmlal_n_u16(npmerge, vld1_u16(&pRawDataline[Newx + b]), weiget);
							vst1q_u32(pMergeline, npmerge);
							pMergeline += 4;
						}
					}
				}
				else
				{
					for (short a = 0; a < Blocksize; a++)
					{
						if (Newx <= NeonBlocksizex && Newx >= 0)
						{
							unsigned short *pRawDataline = pRawPadImage[k].GetImageLine(Newy + a);
							for (int b = 0; b < Blocksize; b += 4)
							{
								uint32x4_t npmerge = vld1q_u32(pMergeline);
								npmerge = vmlal_n_u16(npmerge, vld1_u16(&pRawDataline[Newx + b]), weiget);
								vst1q_u32(pMergeline, npmerge);
								pMergeline += 4;
							}
						}
						else
						{
							unsigned short *pRawDataline = pRawPadImage[k].GetImageLine(Newy + a);
							for (int b = 0; b < Blocksize; b++)
							{
								(*pMergeline++) += weiget * pRawDataline[Newx + b];
							}
						}
					}
				}
			}
		}
	}
}
void CHDRPlus_BlockMatchFusion::MergeWeight(MultiUshortImage *pOutMergeSingleImage, CImageData_UINT32 *pInMergeMultiImage, unsigned int Max)
{
	int nOutWidth = pOutMergeSingleImage->GetImageWidth();
	int nOutHeight = pOutMergeSingleImage->GetImageHeight();
	int nInWidth = pInMergeMultiImage->GetImageWidth();
	int nInHeight = pInMergeMultiImage->GetImageHeight();
	int fang = 32;
	int fangxfang = fang * fang;
	int patch = nInWidth * fangxfang;
	float weight[32];
	for (int v = 0; v < 32; v++)
	{
		weight[v] = 0.5f - 0.5f * cos(2 * 3.141592f * (v + 0.5f) / 32.0f);
	}
	int nProcs = omp_get_num_procs();
#pragma omp parallel  for num_threads(nProcs) schedule(dynamic,16)
	for (int y = 0; y < nOutHeight; y++)
	{
		int lyu16 = y % 16;
		int lyz16 = y / 16;
		int lyz16d1 = (lyz16 - 1);
		if (lyz16d1 < 0)
		{
			lyz16d1 = 0;
		}
		unsigned int *pmergeline0 = pInMergeMultiImage->GetImageLine(lyz16);
		unsigned int *pmergeline1 = pInMergeMultiImage->GetImageLine(lyz16d1);
		unsigned short *pmerged = pOutMergeSingleImage[0].GetImageLine(y);
		for (int x = 0; x < nOutWidth; x++)
		{
			unsigned int lxu16 = x % 16;
			unsigned int lxz16 = x / 16;
			int lxz16d1 = (lxz16 - 1);
			if (lxz16d1 < 0)
			{
				lxz16d1 = 0;
			}
			float weight00 = weight[lxu16 + 16] * weight[lyu16 + 16];
			float weight10 = weight[lxu16] * weight[lyu16 + 16];
			float weight01 = weight[lxu16 + 16] * weight[lyu16];
			float weight11 = weight[lxu16] * weight[lyu16];
			unsigned int val_00 =/* lyz16d1 * patch +*/ lxz16d1 * fangxfang + (lyu16 + 16)*fang + lxu16 + 16;
			unsigned int val_10 =/* lyz16d1 * patch +*/ lxz16 * fangxfang + (lyu16 + 16)*fang + lxu16;
			unsigned int val_01 = /*lyz16 * patch +*/ lxz16d1 * fangxfang + lyu16 * fang + lxu16 + 16;
			unsigned int val_11 =/* lyz16 * patch +*/ lxz16 * fangxfang + lyu16 * fang + lxu16;
			unsigned int tmp = (unsigned int)(weight00 * pmergeline1[val_00] + weight11 * pmergeline0[val_11] + weight01 * pmergeline0[val_01] + weight10 * pmergeline1[val_10] + SCALEVALUEHALF);
			tmp = tmp >> SCALEBIT;
			if (tmp > Max)
			{
				tmp = Max;
			}
			*pmerged++ = (unsigned short)tmp;
		}
	}
}
bool CHDRPlus_BlockMatchFusion::UpScaleOffsetAndValuex2(MultiShortImage *pInImage, MultiShortImage *pOutImage)//小 大
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	int tnx2 = nWidth * 2;
	int tny2 = nHeight * 2;
	if (pOutImage->GetImageWidth() != tnx2 || pOutImage->GetImageHeight() != tny2)
	{
		if (!pOutImage->CreateImageFillValue(tnx2, tny2, 1, 0))return false;
	}
	int ofx = tnx2 * 2 - nWidth;
	int ofy = tny2 * 2 - nHeight;
	int tmptny2 = tny2;
	int tmptnx2 = tnx2;
	if (ofx != 0 || ofy != 0)
	{
		tmptny2 = tmptny2 - 1;
		tmptnx2 = tmptnx2 - 1;
	}
	int nProcs = omp_get_num_procs();
#pragma omp parallel for num_threads(nProcs) schedule(dynamic,16)
	for (int y = 0; y < tmptny2; y++)
	{
		short *pline0 = pOutImage->GetImageLine(y * 2);
		short *pline1 = pOutImage->GetImageLine(y * 2 + 1);
		short *pin = pInImage->GetImageLine(y);
		for (int x = 0; x < tmptnx2; x++)
		{
			short tmp = (*pin++);
			tmp = tmp * 2;
			*pline0++ = tmp;
			*pline0++ = tmp;
			*pline1++ = tmp;
			*pline1++ = tmp;
		}
	}
}
void CHDRPlus_BlockMatchFusion::Forward(MultiUshortImage *pInImages, int nFrameID[], int Framenum, TGlobalControl *pControl)
{
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
	printf("%d %f\n", nGain, Amount);
	m_nAmountFactor = m_nAmountFactor * Amount;
	unsigned short *pInraw[10];
	for (int k = 0; k < Framenum; k++)
	{
		pInraw[k] = pInImages[nFrameID[k]].GetImageData();
	}
	//最大支持10帧输入
	/////swap first frame and last frame////
	int div = 128;
	/*unsigned short *ptmpraw = pInraw[0];
	pInraw[0] = pInraw[Framenum - 1];
	pInraw[Framenum - 1] = ptmpraw;*/
	int nMax = pInImages[0].m_nRawMAXS;
	int nWidth = pInImages[0].GetImageWidth();
	int nHeight = pInImages[0].GetImageHeight();
	int NewnWidth = ((nWidth + div) / div) * div;
	int NewnHeight = ((nHeight + div) / div) * div;
	int padx = (NewnWidth - nWidth) / 2;
	int pady = (NewnHeight - nHeight) / 2;
	MultiUshortImage RawPadImage[12];
	for (int k = 0; k < Framenum; k++)
	{
		pInImages[k].FillImageAround(&RawPadImage[k], padx, pady);
	}
	MultiUshortImage RawDatax2[12];//
	MultiUshortImage RawDatax4[12];//
	MultiUshortImage RawDatax8[12];//
	MultiUshortImage RawDatax16[12];//
	MultiShortImage OffsetxImage[12], OffsetyImage[12];
	MultiShortImage tmpOffsetxImage[12], tmpOffsetyImage[12];
	CImage_FLOAT WeightImage[12];
	CImageData_UINT32 SumblockMergedata;
	for (int k = 0; k < Framenum; k++)
	{
		BoxDownx2(&RawPadImage[k], &RawDatax2[k]);
		RawDatax2[k].DownScaleImagex2(&RawDatax4[k], false);
		RawDatax4[k].DownScaleImagex2(&RawDatax8[k], false);
		RawDatax8[k].DownScaleImagex2(&RawDatax16[k], false);
		/*BoxDownx2(&RawPadImage[k], &RawDatax2[k]);
		BoxDownx2(&RawDatax2[k], &RawDatax4[k]);
		BoxDownx2(&RawDatax4[k], &RawDatax8[k]);
		BoxDownx2(&RawDatax8[k], &RawDatax16[k]);*/
	}
	for (int k = 1; k < Framenum; k++)
	{
		EstimatedOffsetNoRef(RawDatax16, &RawDatax16[k], &OffsetxImage[k], &OffsetyImage[k], m_nOffsetxLevel[0], m_nOffsetyLevel[0]);
		UpScaleOffsetAndValuex2(&OffsetxImage[k], &tmpOffsetxImage[k]);
		UpScaleOffsetAndValuex2(&OffsetyImage[k], &tmpOffsetyImage[k]);
	}
	for (int k = 1; k < Framenum; k++)
	{
		EstimatedOffsetAndRef(RawDatax8, &RawDatax8[k], &tmpOffsetxImage[k], &tmpOffsetyImage[k], &OffsetxImage[k], &OffsetyImage[k], m_nOffsetxLevel[1], m_nOffsetyLevel[1]);
		UpScaleOffsetAndValuex2(&OffsetxImage[k], &tmpOffsetxImage[k]);
		UpScaleOffsetAndValuex2(&OffsetyImage[k], &tmpOffsetyImage[k]);
	}
	for (int k = 1; k < Framenum; k++)
	{
		EstimatedOffsetAndRef(RawDatax4, &RawDatax4[k], &tmpOffsetxImage[k], &tmpOffsetyImage[k], &OffsetxImage[k], &OffsetyImage[k], m_nOffsetxLevel[2], m_nOffsetyLevel[2]);
		UpScaleOffsetAndValuex2(&OffsetxImage[k], &tmpOffsetxImage[k]);
		UpScaleOffsetAndValuex2(&OffsetyImage[k], &tmpOffsetyImage[k]);
	}
	for (int k = 1; k < Framenum; k++)
	{
		EstimatedOffsetAndRef(RawDatax2, &RawDatax2[k], &tmpOffsetxImage[k], &tmpOffsetyImage[k], &OffsetxImage[k], &OffsetyImage[k], m_nOffsetxLevel[3], m_nOffsetyLevel[3]);
		if (m_bDumpFileEnable)
		{
			char name[255];
			sprintf(name, "outbmp/ofx%d.bmp", k);
			OffsetxImage[k].SaveSingleChannelToBitmapFile(name, 0, OffsetxImage[k].GetMaxVal(), 256, 0);
			sprintf(name, "outbmp/ofy%d.bmp", k);
			OffsetyImage[k].SaveSingleChannelToBitmapFile(name, 0, OffsetyImage[k].GetMaxVal(), 256, 0);
		}
	}
	OffsetxImage[0].CreateImageFillValue(OffsetyImage[1].GetImageWidth(), OffsetyImage[1].GetImageHeight(), 1, 0);
	OffsetyImage[0].CreateImageFillValue(OffsetyImage[1].GetImageWidth(), OffsetyImage[1].GetImageHeight(), 1, 0);
	for (int k = 0; k < Framenum; k++)
	{
		WeightImage[k].SetImageSize(OffsetyImage[1].GetImageWidth(), OffsetyImage[1].GetImageHeight(), 1);
		if (k == 0)
		{
			WeightImage[k].FillValue(1);
		}
		else
		{
			WeightImage[k].FillValue(0);
		}
	}
	EstimatedWeight(RawDatax2, Framenum, OffsetxImage, OffsetyImage, WeightImage);
	SumblockMergedata.SetImageSize(OffsetyImage[1].GetImageWidth(), OffsetyImage[1].GetImageHeight(), 1024);
	MergeTemporal(RawPadImage, Framenum, OffsetxImage, OffsetyImage, WeightImage, &SumblockMergedata);
	MergeWeight(RawPadImage, &SumblockMergedata, nMax);
	pInImages[0].CopyImageRect(&RawPadImage[0], padx, pady, RawPadImage[0].GetImageWidth() - padx, RawPadImage[0].GetImageHeight() - pady);
}