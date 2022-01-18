#include "HDRPlus_Forward.h"
void CHDRPlus_Forward::Forward(MultiUshortImage *InRawImage,MultiUcharImage *OutRGBImage8, TGlobalControl *pControl)
{
	const int Framenum = 8;
	int nFrameID[Framenum];
	for (int k = 0; k < Framenum; k++)
	{
		nFrameID[k] = k;
	}
	MultiUshortImage OutDpcRaw;
	MultiUshortImage OutRGBImage16;
	if (m_nBlockMatchFusionEnable)
	{
		m_HDRPlus_BlockMatchFusion.Forward(InRawImage, nFrameID, Framenum, pControl);
		if (m_HDRPlus_BlockMatchFusion.m_bDumpFileEnable)
		{
			InRawImage[0].SaveSingleChannelToBitmapFile("outbmp/BlockMatchFusion.bmp", 0, InRawImage[0].GetMaxVal(), 256, 0);
		}
	}
	if (m_nDPCorrectionEnable)
	{
		m_HDRPlus_DPCorrection.Forward(&InRawImage[0], &OutDpcRaw, pControl);
		if (m_HDRPlus_DPCorrection.m_bDumpFileEnable)
		{
			OutDpcRaw.SaveSingleChannelToBitmapFile("outbmp/DPCorrection.bmp", 0, OutDpcRaw.GetMaxVal(), 256, 0);
		}
	}
	else
	{
		OutDpcRaw.Clone(&InRawImage[0]);
	}
	if (m_nBlackWhiteLevelEnable)
	{
		m_HDRPlus_BlackWhiteLevel.Forward(&OutDpcRaw, pControl);
		if (m_HDRPlus_BlackWhiteLevel.m_bDumpFileEnable)
		{
			OutDpcRaw.SaveSingleChannelToBitmapFile("outbmp/BlackWhiteLevel.bmp", 0, OutDpcRaw.GetMaxVal(), 256, 0);
		}
	}
	if (m_nWhiteBalanceEnable)
	{
		m_HDRPlus_WhiteBalance.Forward(&OutDpcRaw, pControl);
		if (m_HDRPlus_WhiteBalance.m_bDumpFileEnable)
		{
			OutDpcRaw.SaveSingleChannelToBitmapFile("outbmp/WhiteBalance.bmp", 0, OutDpcRaw.GetMaxVal(), 256, 0);
		}
	}
	if (m_nDemosaicingEnable)
	{
		m_HDRPlus_Demosaicing.Forward(&OutDpcRaw, &OutRGBImage16, pControl);
		if (m_HDRPlus_Demosaicing.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/Demosaicing.bmp");
		}
	}
	if (m_nColorCorectEnable)
	{
		m_HDRPlus_ColorCorect.Forward(&OutRGBImage16, pControl);
		if (m_HDRPlus_ColorCorect.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/ColorCorect.bmp");
		}
	}
	if (m_nTonemappingEnable)
	{
		m_HDRPlus_Tonemapping.Forward(&OutRGBImage16, pControl);
		if (m_HDRPlus_Tonemapping.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/Tonemapping.bmp");
		}
	}
	if (m_nGammaCorrectEnable)
	{
		m_HDRPlus_GammaCorrect.Forward(&OutRGBImage16);
		if (m_HDRPlus_GammaCorrect.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/GammaCorrect.bmp");
		}
	}
	if (m_nChromaDenoiseEnable)
	{
		m_HDRPlus_ChromaDenoise.Forward(&OutRGBImage16, pControl);
		if (m_HDRPlus_ChromaDenoise.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/ChromaDenoise.bmp");
		}
	}
	if (m_nContrastEnable)
	{
		m_HDRPlus_Contrast.Forward(&OutRGBImage16);
		if (m_HDRPlus_Contrast.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/Contrast.bmp");
		}
	}
	if (m_nSharpenEnable)
	{
		m_HDRPlus_Sharpen.Forward(&OutRGBImage16);
		if (m_HDRPlus_Sharpen.m_bDumpFileEnable)
		{
			MultiUcharImage tmpOutRGB;
			m_HDRPlus_Normalize.Forward(&OutRGBImage16, &tmpOutRGB);
			tmpOutRGB.SaveRGBToBitmapFile("outbmp/Sharpen.bmp");
		}
	}
	m_HDRPlus_Normalize.Forward(&OutRGBImage16, OutRGBImage8);
	if (m_bDumpFileEnable)
	{
		OutRGBImage8->SaveRGBToBitmapFile("outbmp/Normalize.bmp");
	}
}
