#include "comDef.h"

bool DONG::FileExist(std::string fileName)
{
	std::ifstream f(fileName.c_str());
	if (f.good())
	{
		f.close();
		return true;
	}
	else
	{
		f.close();
		return false;
	}
}

bool DONG::Quantization(std::string output, std::string input)
{
	if (!DONG::FileExist(input))
		return false;

	FILE *pIN = nullptr;
	FILE *pOUT = nullptr;

	BYTE inputData[HEIGHT][WIDTH];
	BYTE outputData[HEIGHT][WIDTH];

	ZeroMemory(inputData, HEIGHT*WIDTH);
	ZeroMemory(outputData, HEIGHT*WIDTH);

	fopen_s(&pIN, input.c_str(), "rb");
	fread(inputData, sizeof(BYTE), HEIGHT*WIDTH, pIN);
	fclose(pIN);

	const int r = 40; // 양자화 파라미터!
	for (int col = 0; col < HEIGHT; col++)
		for (int row = 0; row < WIDTH; row++)
			outputData[col][row] = static_cast<BYTE>(static_cast<int>(inputData[col][row]) / r);

	fopen_s(&pOUT, output.c_str(), "wb");
	fwrite(outputData, sizeof(BYTE), HEIGHT*WIDTH, pOUT);
	fclose(pOUT);

	return true;
}
