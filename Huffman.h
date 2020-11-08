#pragma once

struct Node {
	unsigned char ch;
	int count;
	Node* left;
	Node* right;
	~Node() {
		if (left != NULL)
			delete left;
		if (right != NULL)
			delete right;
	}
};

class cmp {
public:
	bool operator()(const Node* lhs, const Node* rhs) const
	{
		return lhs->count > rhs->count;
	}
};

typedef std::priority_queue<Node*, std::vector<Node*>, cmp> mypq;

class Huffman
{
private:
	int PixelSize = 256;
	int lCount[256];
	std::string str_code[256];


	int rc;
	unsigned char inChar;
	std::string inFile, outFile, BITS, BITSsub, mn;
	std::ofstream outf;
	std::ifstream inf;
	mypq pq;
	bitChar bchar;
	int origSize;
public:
	Huffman();
	~Huffman();

	Node* makeNode(char ch, int count);
	void trie(mypq& _X);
	void code(Node* _X);
	void count(std::string file, int& _X);
	std::string BITSstring(std::string inFile);
	bool process(std::string output, std::string input);
};

