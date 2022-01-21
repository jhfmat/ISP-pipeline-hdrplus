#include "../Layer/HDRPlus_Forward.h"
bool m_bHighBits = false;
bool m_bByteOrder = true;
int m_nWidth = 4608;
int m_nHeight = 3456;
int m_blc = 64;
int m_Saturate = 65535;
int m_Rotation = 0;
int m_FaceNum = 0;
int m_nMIPIRAW = 0;
int m_nFrameNum = 8;
int m_nBits = 16;
int OutBit = 8;
int m_nISO = 2191;
int m_nCFAPattern = 2;
int m_nMinISO = 100;
int m_CameraExposure = 10000000;
float m_fRGain = 1.781603;
float m_fGGain = 1.000000;
float m_fBGain = 1.640738;
float m_f[3][3] = { 1.781250, -0.906250, 0.125000,
	0.007812,1.015625,-0.023438,
	0.218750,-1.007812,1.789062 };
bool LoadMetaDataTxtFile(char *pFileName)
{
	FILE *fp = fopen(pFileName, "rt");
	if (fp == NULL)return false;
	if (fscanf(fp, "wb rgb gain(%f,%f,%f)\n", &m_fRGain, &m_fGGain, &m_fBGain) != 3)return false;
	printf("AWBGain=[%f,%f,%f]\n", m_fRGain, m_fGGain, m_fBGain);
	if (fscanf(fp, "[%f,%f,%f,\n", &m_f[0][0], &m_f[0][1], &m_f[0][2]) != 3)return false;
	if (fscanf(fp, "[%f,%f,%f,\n", &m_f[1][0], &m_f[1][1], &m_f[1][2]) != 3)return false;
	if (fscanf(fp, "[%f,%f,%f,\n", &m_f[2][0], &m_f[2][1], &m_f[2][2]) != 3)return false;
	printf("[%f,%f,%f,\n", m_f[0][0], m_f[0][1], m_f[0][2]);
	printf("[%f,%f,%f,\n", m_f[1][0], m_f[1][1], m_f[1][2]);
	printf("[%f,%f,%f,\n", m_f[2][0], m_f[2][1], m_f[2][2]);

	if (fscanf(fp, "ae  sensorSensitivity(iso): %d\n", &m_nISO) != 1)return false;
	printf("ISO=%d\n", m_nISO);
	if (fscanf(fp, "exposure(ns): %d\n", &m_CameraExposure) != 1)return false;
	printf("Exposure=%d\n", m_CameraExposure);
	if (fscanf(fp, "BLC: %d\n", &m_blc) != 1)return false;
	printf("BLC=%d\n", m_blc);

	if (fscanf(fp, "CFAPattern: %d\n", &m_nCFAPattern) != 1)return false;
	printf("m_nCFAPattern=%d\n", m_nCFAPattern);

	if (fscanf(fp, "Saturate: %d\n", &m_Saturate) != 1)return false;
	printf("m_Saturate=%d\n", m_Saturate);

	if (fscanf(fp, "raw_width,raw_height(%d %d)\n", &m_nWidth, &m_nHeight) != 2)return false;
	printf("raw_width,raw_height=%d %d\n", m_nWidth, m_nHeight);
	//if (fscanf(fp, "Rotation: %d\n", &m_Rotation) != 1)return false;
	//printf("Rotation=%d\n", m_Rotation);
	//if (fscanf(fp, "FaceNum: %d\n", &m_FaceNum) != 1)return false;
	//printf("FaceNum=%d\n", m_FaceNum);
	return true;
}
CHDRPlus_Forward *RAWProcessFlow;
int main(int argc, char *argv[])
{
	TimerStatistics::start();
	MemPool::Create(1500 << 20);
	RAWProcessFlow = new CHDRPlus_Forward;
	TimerStatistics::stop("WTAlgo_ToolsInit ");
	LoadMetaDataTxtFile(argv[1]);
	MultiUshortImage InRawImage[10], OutRGBImage16;
	MultiUcharImage OutRGBImage8;
	TGlobalControl tControl;
	tControl.nBLC = m_blc;
	tControl.nWP = m_Saturate;
	tControl.nCameraGain = m_nISO * 16 / m_nMinISO;
	tControl.nFrameNum = m_nFrameNum;
	tControl.nCFAPattern = m_nCFAPattern;
	tControl.nAWBGain[0] = m_fRGain * 256;
	tControl.nAWBGain[1] = m_fGGain * 256;
	tControl.nAWBGain[2] = m_fGGain * 256;
	tControl.nAWBGain[3] = m_fBGain * 256;
	for (int n = 0; n < 3; n++)
	{
		for (int m = 0; m < 3; m++)
		{
			tControl.nCCM[n][m] = m_f[n][m];
		}
	}
	for (int k = 0; k < tControl.nFrameNum; k++)
	{
		printf("%s\n", argv[2 + k]);
		InRawImage[k].Load16BitRawDataFromBinFile(argv[2+k], m_nWidth, m_nHeight, m_nBits, m_bHighBits, m_bByteOrder, m_nMIPIRAW);
	}
	CHDRPlus_Forward m_HDRPlus_Forward;
	if (!m_HDRPlus_Forward.LoadMultiConfigFile("weight.param"))
	{
		printf("weight.param fail\n");
		m_HDRPlus_Forward.SaveMultiConfigFile("default.param");
		exit(1);
	}
	m_HDRPlus_Forward.Forward(InRawImage, &OutRGBImage8, &tControl);
	if (NULL != RAWProcessFlow)
	{
		delete RAWProcessFlow;
		RAWProcessFlow = NULL;
	}
	MemPool::Release();
	return 0;
}

