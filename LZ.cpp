#include <iostream>
#include <string>
#include <fstream>
#include <vector>    //for std::vector
#include <algorithm> //for std::reverse
#include <climits>	//for CHAR_BIT
#include <cmath>
#include <queue>
#include <ctime>

using namespace std;


int N = 11;
int L = 4;
int S = 3;
int numBits = 24;

vector<bool> outputBits;
queue<char> literalChars;
bool debug = false;

bool parseArguments(int argc, char *argv[], int *N, int *L, int *S, string *fileName)
{
	bool fileSet = false;
	for (int i = 1; i < argc; i++)
	{
		string thisArg = argv[i];
		if (argv[i][0] == '-' && argv[i][2] == '=')
		{
			if (argv[i][1] == 'N')
			{
				int tempNum = atoi(thisArg.substr(3).c_str());
				if (tempNum < 9 || tempNum > 14)
					return false;
				*N = tempNum;
			}
			else if (argv[i][1] == 'L')
			{
				int tempNum = atoi(thisArg.substr(3).c_str());
				if (tempNum < 3 || tempNum > 4)
					return false;
				*L = tempNum;
			}
			else if (argv[i][1] == 'S')
			{
				int tempNum = atoi(thisArg.substr(3).c_str());
				if (tempNum < 1 || tempNum > 5)
					return false;
				*S = tempNum;
			}
		}
		else
		{
			if (fileSet)
				return false;
			fileSet = true;
			*fileName = thisArg;
		}
	}

	if (!fileSet)
		return false;

	return true;
}

void convert(int input, int L) {
	vector<bool> ret;
	for (unsigned int i = 0; i < L; ++i, input >>= 1)
		ret.push_back(input & 1);
	
	reverse(ret.begin(), ret.end());

	for (int i = 0; i < ret.size(); i++)
	{
		if (debug) cout << ret[i];
		outputBits.push_back(ret[i]);
	}
}

void buildCharVector(vector<char> *charArray, string fileName)
{
	ifstream fileStream(fileName.c_str(), std::ios::binary | std::ios::in);
	char c;

	// for each char in the file
	while (fileStream.get(c))
		charArray->push_back(c);
}

void outputLiterals()
{
	if(debug) cout << "(0, " << literalChars.size() << ") = "; 
	convert(0, L);
	convert(literalChars.size(), S);
	if(debug) cout << endl;

	numBits += (L + S);	
	numBits += (literalChars.size() * 8);

	while(!literalChars.empty())
	{
		if(debug) cout << literalChars.front() << "-";
		convert(literalChars.front(), 8);
		literalChars.pop();
		if(debug) cout << "	";
	}
	if(debug) cout << ")" << endl;
}

int findNextMatch(vector<char> charArray, int firstIndex, int bufferIndex, int lastIndex)
{
	int largestMatchLength = 0;
	int largestMatchOffset = -1;

	int maxLengthAllowed = pow(2, L) - 1;

	for (int i = 0; i < bufferIndex - firstIndex; i++)
	{	
		int loop = 0;
		while ((bufferIndex + loop) < charArray.size()
			&& charArray[firstIndex + i + loop] == charArray[bufferIndex + loop]
			&& loop < maxLengthAllowed)
			loop++;

		if (loop > largestMatchLength)
		{
			largestMatchLength = loop;
			largestMatchOffset = bufferIndex - firstIndex - i;
		}
	}

	if (largestMatchLength == 0 || largestMatchLength == 1)
	{
		literalChars.push(charArray[bufferIndex]);

		if(literalChars.size() == pow(2, S) - 1)
			outputLiterals();		

		return 1;
	}
	else
	{
		if(literalChars.size() > 0)
			outputLiterals();

		if (debug) cout << "(" << largestMatchLength << ", " << largestMatchOffset << ")		-->		";
		convert(largestMatchLength - 1, L);
		convert(largestMatchOffset, N);
		if (debug) cout << endl;
		numBits += (L + N);

		return largestMatchLength;
	}
}

void startSlidingWindow(vector<char> charArray)
{
	int W = pow(2, N);
	int F = pow(2, L) - 1;
	int LB = W - F;

	int firstIndex = 0;
	int bufferIndex = 0;
	int lastIndex = F-1;
	
	while (bufferIndex < charArray.size())
	{
		int shiftDistance = findNextMatch(charArray, firstIndex, bufferIndex, lastIndex);

		if (lastIndex == charArray.size() - 1)
		{
			firstIndex += shiftDistance;
			bufferIndex += shiftDistance;
		}
		else if (lastIndex - firstIndex + 1 >= W)
		{
			firstIndex += shiftDistance;
			bufferIndex += shiftDistance;
			lastIndex += shiftDistance;
			if (lastIndex > charArray.size() - 1)
				lastIndex = charArray.size() - 1;
		}
		else
		{
			bufferIndex += shiftDistance;
			lastIndex += shiftDistance;
			if (lastIndex > charArray.size() - 1)
				lastIndex = charArray.size() - 1;
			if(bufferIndex - firstIndex > LB)
				firstIndex += (bufferIndex - firstIndex) % LB;
		}
	}

	if(literalChars.size() > 0)
		outputLiterals();

	if (debug) cout << "(0, 0)		-->		";
	convert(0, L);
	convert(0, S);
	if (debug) cout << endl;
	numBits += (L + S);

	if (debug) cout << "Extra:		-->		";
	if(numBits % 8 != 0)
		convert(0, 8 - (numBits % 8));
	if (debug) cout << endl;

}

void outputCharBytes()
{
	int i = 0;
	while (i < outputBits.size())
	{
		string thisByte = "";
		for (int j = i; j < i + 8; j++)
		{
			if (outputBits[j])
				thisByte += "1";
			else
				thisByte += "0";
		}

		if (debug) cout << thisByte << "	-->		";
		
		char c = strtol(thisByte.c_str(), 0, 2);
		cout << c;

		if (debug) cout << endl;
		i += 8;
	}
}

int main(int argc, char* argv[])
{
	clock_t start;
	start = clock();

	string fileName;
	
	if (!parseArguments(argc, argv, &N, &L, &S, &fileName))
	{
		cerr << "Failed to parse..." << endl;
		return 0;
	}

	fprintf(stderr, "\nParamaters -> N: %d   L: %d   S: %d\n", N, L, S);
		
	convert(N, 8);
	if (debug) cout << endl;
	convert(L, 8);
	if (debug) cout << endl;
	convert(S, 8);
	if (debug) cout << endl << endl;

	vector<char> bitArray;
	buildCharVector(&bitArray, fileName);
	if(debug) cout << endl;
	
	double original = ((double) bitArray.size()) / ((double) 1024);
	fprintf(stderr, "Input size: %.2f KB\n", original);

	startSlidingWindow(bitArray);
	if(debug) cout << endl;

	if (debug)
	{
		cout << "Outputting all bits: " << endl;
		for (int i = 0; i < outputBits.size(); i++)
			cout << outputBits[i];
		cout << endl;
	}
	
	outputCharBytes();
	if(debug) cout << endl;

	double compressed = ((double) outputBits.size()) / ((double) 8192);
	fprintf(stderr, "Output size: %.2f KB\n", compressed);
	fprintf(stderr, "Total compression savings: %.2f%%\n", (original - compressed) / original * 100);	
	fprintf (stderr, "Total encoding runtime: %.2f seconds\n\n", (clock() - start) / ((double) CLOCKS_PER_SEC));
		
	return 0;
}
