#pragma once

class bitChar {
public:
	unsigned char* c;
	int shift_count;
	std::string BITS;

	bitChar();
	void setBITS(std::string _X);
	int insertBits(std::ofstream& outf);
	std::string getBits(unsigned char _X);
	void writeBits(std::ofstream& outf);
	~bitChar();
};
