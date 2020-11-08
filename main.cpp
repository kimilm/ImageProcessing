#pragma warning (disable:4996)

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#include "comDef.h"

#define RESOLUTION 921600	// (720 * 1280)
#define PICTURE 1382400		// (720 * 1280) + (360 * 640) + (360 * 640)
#define PI 3.14159265359
#define N 8

void DCT(unsigned char ** input, unsigned char ** output, int col, int row, int quant);
unsigned char * Decode(unsigned char ** dct, int col, int row, int quant);

int main (int argc, char** argv) {
	int quant = 0;	// set quantization parameter in command line

	if (argc == 1)
		quant = 1;	// dct variable (dctY, dctU, dctV) | type "char" -> overflow, "int" -> no problem
	else
		quant = atoi(argv[1]);	// less than 15, error (char type)

	//----Read File----
	const char * origFile = "PeopleOnStreet_1280x720_30_Original.yuv";
	FILE * yuvOrig = fopen(origFile, "rb");

	const char * idctFile = "PeopleOnStreet_1280x720_idct.yuv";
	FILE * yuvIDCT = fopen(idctFile, "wb");

	//----for DCT orig----
	unsigned char ** origY = (unsigned char **)malloc(sizeof(char *) * 720);
	for (int i = 0; i < 720; ++i)
		origY[i] = (unsigned char *)malloc(sizeof(char) * 1280);

	unsigned char ** origU = (unsigned char **)malloc(sizeof(char *) * 360);
	for (int i = 0; i < 360; ++i)
		origU[i] = (unsigned char *)malloc(sizeof(char) * 640);

	unsigned char ** origV = (unsigned char **)malloc(sizeof(char *) * 360);
	for (int i = 0; i < 360; ++i)
		origV[i] = (unsigned char *)malloc(sizeof(char) * 640);

	//----for DCT trans---- 
	unsigned char ** dctY = (unsigned char **)malloc(sizeof(char *) * 720);
	for (int i = 0; i < 720; ++i)
		dctY[i] = (unsigned char *)malloc(sizeof(char) * 1280);

	unsigned char ** dctU = (unsigned char **)malloc(sizeof(char *) * 360);
	for (int i = 0; i < 360; ++i)
		dctU[i] = (unsigned char *)malloc(sizeof(char) * 640);

	unsigned char ** dctV = (unsigned char **)malloc(sizeof(char *) * 360);
	for (int i = 0; i < 360; ++i)
		dctV[i] = (unsigned char *)malloc(sizeof(char) * 640);

	//----for MSE----
	unsigned char * origYUV = (unsigned char *)malloc(sizeof(char) * PICTURE);
	unsigned char * idctYUV = (unsigned char *)malloc(sizeof(char) * PICTURE);
	unsigned char * idct;

	//----read one frame for MSE----
	fread(origYUV, 1, PICTURE, yuvOrig);
	fseek(yuvOrig, 0L, SEEK_SET);

	//----huffman----
	std::string HuffmanFile = "huffman_PeopleOnStreet_1280x720.yuv";
	std::string QuantizationFile = "quant_PeopleOnStreet_1280x720.yuv";

	FILE * quantFile = fopen(QuantizationFile.c_str(), "wb");

	//----MSE----
	double mse = 0;
	
	//----read one frame for DCT----
	for (int i = 0; i < 720; ++i)
		fread(origY[i], 1, 1280, yuvOrig);

	for (int i = 0; i < 360; ++i)
		fread(origU[i], 1, 640, yuvOrig);

	for (int i = 0; i < 360; ++i)
		fread(origV[i], 1, 640, yuvOrig);

	//----DCT & Quantization----
	DCT(origY, dctY, 720, 1280, quant);
	DCT(origU, dctU, 360, 640, quant);
	DCT(origV, dctV, 360, 640, quant);

	//----DCT & Quantization file write----
	for (int i = 0; i < 720; ++i)
		fwrite(dctY[i], 1, 1280, quantFile);

	for (int i = 0; i < 360; ++i)
		fwrite(dctU[i], 1, 640, quantFile);

	for (int i = 0; i < 360; ++i)
		fwrite(dctV[i], 1, 640, quantFile);

	//----IDCT----
	idct = Decode(dctY, 720, 1280, quant);
	memcpy(idctYUV, idct, 720 * 1280);
	free(idct);

	idct = Decode(dctU, 360, 640, quant);
	memcpy(&idctYUV[720 * 1280], idct, 360 * 640);
	free(idct);

	idct = Decode(dctV, 360, 640, quant);
	memcpy(&idctYUV[720 * 1280 + 360 * 640], idct, 360 * 640);
	free(idct);

	//----IDCT file write----
	fwrite(idctYUV, 1, PICTURE, yuvIDCT);

	//----MSE----
	for (int i = 0; i < 720; ++i)
		for(int j = 0; j < 1280; ++j)
			mse += pow(origY[i][j] - idctYUV[i * 1280 + j], 2);

	mse /= RESOLUTION;
	printf("1 frame done... (MSE: %lf)\n", mse);

	fclose(quantFile);

	// huffman
	Huffman* pHuffman = new Huffman;

	if (!pHuffman->process(HuffmanFile, QuantizationFile))
		std::cout << "QuantizationFile을 찾을 수 없음" << std::endl;

	//----free memory----
	if (pHuffman)
		delete pHuffman;

	for (int i = 0; i < 720; ++i) {
		free(origY[i]);
		free(dctY[i]);
	}
	free(origY);
	free(dctY);

	for (int i = 0; i < 360; ++i) {
		free(origU[i]);
		free(origV[i]);
		free(dctU[i]);
		free(dctV[i]);
	}
	free(origU);
	free(origV);
	free(dctU);
	free(dctV);

	free(origYUV);
	free(idctYUV);

	fclose(yuvOrig);
	fclose(yuvIDCT);

	return 0;
}

void DCT(unsigned char ** input, unsigned char ** output, int col, int row, int quant) {
	double result;
	double constX;
	double constY;
	int blockX = 0;
	int blockY = 0;

	for (int x = 0; x < col; ++x) {
		if (x % N == 0)
			constX = sqrt((double)1 / N);
		else
			constX = sqrt((double)2 / N);

		if (x == blockX + N)
			blockX += N;

		blockY = 0;

		for (int y = 0; y < row; ++y) {
			if (y % N == 0)
				constY = sqrt((double)1 / N);
			else
				constY = sqrt((double)2 / N);

			result = 0.0;

			if (y == blockY + N)
				blockY += N;

			for (int i = 0; i < N; ++i) {
				for (int j = 0; j < N; ++j) {
					result += input[blockX + i][blockY + j]
						* cos(((2 * j + 1) * (y % N) * PI) / (2 * N))
						* cos(((2 * i + 1) * (x % N) * PI) / (2 * N));
				}
			}
			output[x][y] = (char)((constX * constY * result) / quant) + 128;
		}
	}
}

unsigned char * Decode(unsigned char ** dct, int col, int row, int quant) {
	double result;
	double constX;
	double constY;
	int blockI = 0;
	int blockJ = 0;

	unsigned char * temp = (unsigned char *)malloc(sizeof(char) * col * row);
	memset(temp, 0, sizeof(char) * col * row);

	for (int i = 0; i < col; ++i) {
		if (i == blockI + N)
			blockI += N;

		blockJ = 0;

		for (int j = 0; j < row; ++j) {
			if (j == blockJ + N)
				blockJ += N;

			result = 0;

			for (int x = 0; x < N; ++x) {
				if (x == 0)
					constX = sqrt((double)1 / N);
				else
					constX = sqrt((double)2 / N);

				for (int y = 0; y < N; ++y) {
					if (y == 0)
						constY = sqrt((double)1 / N);
					else
						constY = sqrt((double)2 / N);

					result += constX * constY * 
						((dct[blockI + x][blockJ + y] - 128) * quant)
						* cos(((2 * (j % N) + 1) * y * PI) / (2 * N))
						* cos(((2 * (i % N) + 1) * x * PI) / (2 * N));
				}
			}
			temp[(i * row) + j] = result;
		}
	}
	return temp;
}