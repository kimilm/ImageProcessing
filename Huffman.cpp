#include "comDef.h"

Huffman::Huffman()
{
	inFile = "", outFile = "", BITSsub = "", mn = "";
	origSize = 0;
}

Huffman::~Huffman()
{
	if (inf.is_open())
		inf.close();
	if (outf.is_open())
		outf.close();
}

Node* Huffman::makeNode(char ch, int count)
{
	Node* tmp = new Node;
	tmp->ch = ch;
	tmp->count = count;
	tmp->left = NULL;
	tmp->right = NULL;
	return tmp;
};

void Huffman::trie(mypq& _X)
{
	while (_X.size() > 1)
	{
		Node* holder = new Node;
		holder->left = _X.top(); _X.pop();
		holder->right = _X.top(); _X.pop();
		holder->count = holder->left->count + holder->right->count;
		holder->ch = -1;
		_X.push(holder);
	}
}

//Create bit codes by recursively traversing the trie, adding a 0 for left and 1 for right, the key is to remove the end char when the recursion breaks and you have to go up a level
void Huffman::code(Node* _X)
{
	static std::string bits = "";
	if (_X->right != NULL)
	{
		bits += "1";
		code(_X->right);
		bits = bits.substr(0, bits.size() - 1);
	}
	if (_X->left != NULL)
	{
		bits += "0";
		code(_X->left);
		bits = bits.substr(0, bits.size() - 1);
	}
	if (!_X->left && !_X->right)
	{
		str_code[_X->ch] = bits;
	}
}

void Huffman::count(std::string file, int& _X) {
	unsigned char letter;
	std::ifstream inf(file.c_str());
	inf >> std::noskipws;
	//Clears array
	for (int i = 0; i < PixelSize; ++i)
		lCount[i] = 0;
	//Goes through text and counts
	while (inf >> letter) {
		if (letter >= 0 && letter < PixelSize)
		{
			++lCount[letter];
			++_X;
		}
	}
	inf.close();
}

//Generates a string of the bit codes in the order they appear in the file
//Used during encoding
std::string	Huffman::BITSstring(std::string inFile)
{
	unsigned char input;
	std::string BITS = "";

	//Open input stream and create BITS string of entire file
	std::ifstream inf(inFile);
	while (inf >> input)
	{
		BITS.append(str_code[input]);
	}
	inf.close();
	//Append ascii 3 EOT character to signify end of text
	BITS.append(str_code[3]);
	return BITS;
}

bool Huffman::process(std::string output, std::string input)
{
	if (!DONG::FileExist(input))
		return false;
	inFile = input;
	outFile = output;

	outf.open(outFile.c_str());

	count(inFile, origSize);
	//count and populate array of letter occurrences (lCount) and add one EOT char
	if (lCount[3] == 0)
		lCount[3] = 1;

	//Output compressed file header
	outf << inFile << std::endl;
	for (int i = 0; i < PixelSize; ++i)
	{
		outf << lCount[i] << " ";
	}
	outf << std::endl;

	//Create nodes based on the available ascii characters and push them into the priority queue
	for (int i = 0; i < PixelSize; ++i)
	{
		if (lCount[i] > 0)
		{
			Node* tmp = makeNode(i, lCount[i]);
			pq.push(tmp);
		}
	}

	//Create trie and bit codes
	trie(pq);
	code(pq.top());

	//Create string of bitcodes for actual huffman encoding and do it
	BITS = BITSstring(inFile);
	outf << '#';
	bchar.setBITS(BITS);
	outf << std::noskipws;
	rc = bchar.insertBits(outf);
	std::cout << rc << std::endl;

	return true;
}