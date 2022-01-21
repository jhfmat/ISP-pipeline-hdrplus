//#include <iostream>
//#include <stdio.h>
//#include "libraw/libraw.h"
//#include <array>
//#include <string>
//#include <algorithm>
//#include <unordered_map>
//std::string GetCfaPatternString(LibRaw *RawProcessor)
//{
//    static const std::unordered_map<char, char> CDESC_TO_CFA = {
//        {'R', 0},
//        {'G', 1},
//        {'B', 2},
//        {'r', 0},
//        {'g', 1},
//        {'b', 2}
//    };
//    const auto &cdesc = RawProcessor->imgdata.idata.cdesc;
//    return {
//        CDESC_TO_CFA.at(cdesc[RawProcessor->COLOR(0, 0)]),
//        CDESC_TO_CFA.at(cdesc[RawProcessor->COLOR(0, 1)]),
//        CDESC_TO_CFA.at(cdesc[RawProcessor->COLOR(1, 0)]),
//        CDESC_TO_CFA.at(cdesc[RawProcessor->COLOR(1, 1)])
//    };
//}
//int GetCFApattern(LibRaw *RawProcessor)
//{
//	const auto cfa_pattern = GetCfaPatternString(RawProcessor);
//	if (cfa_pattern == std::string{ 0, 1, 1, 2 })
//	{
//		//printf("RGGB==1\n");
//		return 3;
//	}
//	else if (cfa_pattern == std::string{ 1, 0, 2, 1 })
//	{
//		//printf("GRBG==0\n");
//		return 2;
//	}
//	else if (cfa_pattern == std::string{ 2, 1, 1, 0 })
//	{
//		//printf("BGGR==2\n");
//		return 0;
//	}
//	else if (cfa_pattern == std::string{ 1, 2, 0, 1 })
//	{
//		//printf("GBRG==3\n");
//		return 1;
//	}
//	return -1;
//}
//void GetAWBgain(LibRaw *RawProcessor,float Awbgain[])
//{
//	const auto coeffs = RawProcessor->imgdata.color.cam_mul;
//	const float r = coeffs[0] / coeffs[1];
//	const float g0 = 1.f;
//	const float g1 = 1.f;
//	const float b = coeffs[2] / coeffs[1];
//	Awbgain[0] = r;
//	Awbgain[1] = g0;
//	Awbgain[3] = b;
//}
//void GetColorMatix(LibRaw *RawProcessor, float ccm[][3])
//{
//	const auto raw_color = RawProcessor->imgdata.color;
//	for (int y = 0; y < 3; y++)
//	{
//		for (int x = 0; x < 3; x++)
//		{
//			ccm[y][x]= raw_color.rgb_cam[y][x];
//		}
//	}
//}
//bool CopyImageRect(unsigned short *pImage,int nWidth,int nHeight,unsigned short *OutImage, unsigned int nLeft, int nTop, int nRight, int nBottom)
//{
//
//	int NewWidth = nRight - nLeft;
//	int NewHeight = nBottom - nTop;
//	int nCutLineLen = NewWidth * sizeof(unsigned short);
//	#pragma omp parallel for
//	for (int y = 0; y < NewHeight; y++)
//	{
//		unsigned short *pIn = pImage+(y+ nTop)*nWidth+ nLeft;
//		unsigned short *pOut = OutImage +y* NewWidth;
//		memcpy(pOut, pIn , nCutLineLen);
//	}
//	return true;
//}
//int main(int argc, char* argv[]) {
//    
//	char Inpathname[255];
//	sprintf(Inpathname, "%s", argv[1]);
//	char Parampathname[255];
//	sprintf(Parampathname, "%s", argv[3]);
//	FILE *fptxt = fopen(Parampathname, "wb");
//	LibRaw RawProcessor;
//	if (int err = RawProcessor.open_file(Inpathname))
//	{
//		printf("fail\n");
//		return 0;
//	}
//	if (int err = RawProcessor.unpack())
//	{
//		printf("fail\n");
//		return 0;
//	}
//	if (int ret = RawProcessor.raw2image()) 
//	{
//		printf("fail\n");
//		return 0;
//	}
//	const auto image_data = (uint16_t*)RawProcessor.imgdata.rawdata.raw_image;
//	const auto raw_width = RawProcessor.imgdata.rawdata.sizes.raw_width;
//	const auto raw_height = RawProcessor.imgdata.rawdata.sizes.raw_height;
//	const auto top = RawProcessor.imgdata.rawdata.sizes.top_margin;
//    const auto left = RawProcessor.imgdata.rawdata.sizes.left_margin;
//	//printf("raw_width, raw_height==%d %d \n", raw_width, raw_height);
//	//printf("left, top==%d %d \n", left, top);
//	const auto raw_color = RawProcessor.imgdata.color;
//	const auto base_black_level = static_cast<float>(raw_color.black);
//	std::array<float, 4> black_level = {
//		base_black_level + static_cast<float>(raw_color.cblack[0]),
//		base_black_level + static_cast<float>(raw_color.cblack[1]),
//		base_black_level + static_cast<float>(raw_color.cblack[2]),
//		base_black_level + static_cast<float>(raw_color.cblack[3])
//	};
//	if (raw_color.cblack[4] == 2 && raw_color.cblack[5] == 2) 
//	{
//		for (int x = 0; x < raw_color.cblack[4]; ++x) 
//		{
//			for (int y = 0; y < raw_color.cblack[5]; ++y) 
//			{
//				const auto index = y * 2 + x;
//				black_level[index] = raw_color.cblack[6 + index];
//			}
//		}
//	}
//	//printf("%f\n",RawProcessor.imgdata.other.aperture);//¹âÈ¦
//	//printf("%f\n", RawProcessor.imgdata.other.iso_speed);
//	//printf("%f\n", RawProcessor.imgdata.other.shutter);
//
//
//	float Awbgain[4];
//	GetAWBgain(&RawProcessor, Awbgain);
//	printf("wb rgb gain(%f,%f,%f)", Awbgain[0], Awbgain[1], Awbgain[3]);
//	fprintf(fptxt, "wb rgb gain(%f,%f,%f)", Awbgain[0], Awbgain[1], Awbgain[3]);
//	float ccm[3][3];
//	for (int y = 0; y < 3; ++y)
//	{
//		printf("\n");
//		printf("[");
//		fprintf(fptxt, "\n");
//		fprintf(fptxt, "[");
//		for (int x = 0; x < 3; ++x)
//		{
//			printf("%f,", raw_color.rgb_cam[y][x]);
//			fprintf(fptxt,"%f,", raw_color.rgb_cam[y][x]);
//		}
//	}
//	printf("\n");
//	printf("ae  sensorSensitivity(iso): %d\n", (int)(RawProcessor.imgdata.other.iso_speed));
//	printf("exposure(ns): %d\n", (unsigned int)(RawProcessor.imgdata.other.shutter* 1000000000));
//	printf("BLC: %d\n", static_cast<int>(*std::min_element(black_level.begin(), black_level.end())));
//	printf("CFAPattern=%d\n", GetCFApattern(&RawProcessor));
//	printf("Saturate=%d\n", RawProcessor.imgdata.color.maximum);
//
//	fprintf(fptxt, "\n");
//	fprintf(fptxt, "ae  sensorSensitivity(iso): %d\n", (int)(RawProcessor.imgdata.other.iso_speed));
//	fprintf(fptxt, "exposure(ns): %d\n", (unsigned int)(RawProcessor.imgdata.other.shutter * 1000000000));
//	fprintf(fptxt, "BLC: %d\n", static_cast<int>(*std::min_element(black_level.begin(), black_level.end())));
//	fprintf(fptxt, "CFAPattern: %d\n", GetCFApattern(&RawProcessor));
//	fprintf(fptxt, "Saturate: %d\n", RawProcessor.imgdata.color.maximum);
//
//	int NewWidth = raw_width - left;
//	int NewHeight = raw_height - top;
//	printf("raw_width,raw_height(%d %d)\n", NewWidth, NewHeight);
//	fprintf(fptxt, "raw_width,raw_height(%d %d)\n", NewWidth, NewHeight);
//	unsigned short *OutImage = new unsigned short[NewWidth*NewHeight];
//	CopyImageRect(image_data, raw_width, raw_height, OutImage, left, top, raw_width, raw_height);
//	char outpathname[255];
//	sprintf(outpathname,"%s", argv[2]);
//	FILE *fp = fopen(outpathname, "wb");
//	fwrite(OutImage, 2, NewWidth*NewHeight, fp);
//	fclose(fp);
//	delete[]OutImage;
//    return 0;
//}
