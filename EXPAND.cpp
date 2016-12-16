#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <ctime>

using namespace std;

int N = -1;
int L = -1;
int S = -1;

vector<bool> outputBits;
bool debug = true;

void buildBitVector(vector<bool> *bitArray, string fileName)
{
	ifstream fileStream(fileName.c_str(), std::ios::binary | std::ios::in);
	char c;

	while (fileStream.get(c))
	{
		for (int i = 7; i >= 0; i--)
			bitArray->push_back((bool)((c >> i) & 1));
	}
}

void parseFirstThree(vector<bool> bitArray)
{
	int i = 0;
	while (i < 24)
	{
		string thisByte = "";
		
		for(int j = i; j < i + 8; j++)
		{
			if (bitArray[j])
				thisByte += "1";
			else
				thisByte += "0";
		}

		int thisNum = strtol(thisByte.c_str(), 0, 2);
		
		if (N == -1)
			N = thisNum;
		else if (L == -1)
			L = thisNum;
		else
			S = thisNum;

		i += 8;
	}
}

void decodeBits(vector<bool> bitArray)
{
	int i = 24;
	bool done = false;

	while (!done)
	{
		int thisL = 0;
		for (int j = i; j < i + L; j++)
			thisL = (thisL << 1) + bitArray[j];
		i += L;
		if (thisL == 0) //String literal, no match found.
		{
			int thisS = 0;
			for (int j = i; j < i + S; j++)
				thisS = (thisS << 1) + bitArray[j];
			i += S;
			if (thisS == 0)
			{
				if (debug) cout<< "(0, 0)" << endl;
				done = true;
				continue;
			}

			for (int z = i; z < i + (thisS * 8); z++)
			{
				outputBits.push_back(bitArray[z]);
				if (debug) cout<< "(" << thisL << ", " << thisS << ", " << bitArray[z] << ")		-->		";
				if (debug) cout<< bitArray[z] << endl;
			}

			i += (thisS * 8);
		}
		else //Found a match, check for match length and offset.
		{
			int thisN = 0;
			for (int j = i; j < i + N; j++)
				thisN = (thisN << 1) + bitArray[j];
			i += N;

			if (debug) cout<< "(" << thisL + 1 << ", " << thisN << ")			-->		" << endl;

			int currentSize = outputBits.size();
			for (int j = 0; j < (thisL + 1) * 8; j++)
				outputBits.push_back(outputBits[currentSize - (thisN * 8) + j]);

			if (debug) cout<< endl;
		}
	}

	if (debug)
	{
		for (int i = 0; i < outputBits.size(); i++)
			cout << outputBits[i];
		cout << endl;
	}
}

void outputChars()
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

	vector<bool> bitArray;
	if(argc > 1)
	{
		string fileName = argv[1];
		buildBitVector(&bitArray, fileName);
	}
	else
	{
		char c;
		while(cin.get(c))
		{
			for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes, I didn't write this comment
				bitArray.push_back((bool)((c >> i) & 1));
		}
	}
	
	parseFirstThree(bitArray);
	fprintf(stderr, "\nParameters -> N: %d   L: %d   S: %d\n", N, L, S);

	double original = ((double) bitArray.size()) / ((double) 8192);
	fprintf(stderr, "Input size: %.2f KB\n", original);	

	decodeBits(bitArray);
	
	outputChars();

	double expanded = ((double) outputBits.size()) / ((double) 8192);
	fprintf(stderr, "Output size: %.2f KB\n", expanded);
	fprintf(stderr, "Total decoding runtime: %.2f seconds\n\n", (clock() - start) / ((double) CLOCKS_PER_SEC));

	return 0;
}
