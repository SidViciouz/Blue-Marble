#include "Model.h"
#include "IfError.h"

string getToken(string& aLine, bool isFirst);
int getNumber(string& aWord, bool isFirst);

Model::Model(ID3D12Device* device,const char* fileName, ID3D12GraphicsCommandList* commandList)
{
	ifstream ifs;
	ifs.open(fileName,ios_base::in);

	//ofstream ofs;
	//ofs.open("test.txt", ios_base::out);

	if (ifs.fail())
		IfError::Throw(L"model file open error!");

	float x, y, z;
	unsigned int vi, uvi, ni;
	vector<XMFLOAT3> positions;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> normals;
	uint16_t j = 0;

	while (!ifs.eof())
	{
		string aLine;
		getline(ifs, aLine);

		string type = getToken(aLine, true);

		if (type.compare("v") == 0)
		{
			string value;
			x = stof(getToken(aLine, false));
			y = stof(getToken(aLine, false));
			z = stof(getToken(aLine, false));
			positions.push_back({ x,y,z });
		}
		else if (type.compare("vt") == 0)
		{
			string value;
			x = stof(getToken(aLine, false));
			y = stof(getToken(aLine, false));
			uvs.push_back({ x,y});
		}
		else if (type.compare("vn") == 0)
		{
			string value;
			x = stof(getToken(aLine, false));
			y = stof(getToken(aLine, false));
			z = stof(getToken(aLine, false));
			normals.push_back({ x,y,z });
		}
		else if (type.compare("f") == 0)
		{
			string value;
			vector<int> index;
			while ((value = getToken(aLine, false)).compare(" ") != 0)
			{
				index.push_back(getNumber(value, true)-1);
				index.push_back(getNumber(value, false)-1);
				index.push_back(getNumber(value, false)-1);
			}

			if (index.size() == 9)
			{
				for (int i = 0; i < 9; i = i + 3)
				{
					mVertices.push_back({ positions[index[i]],uvs[index[i + 1]],normals[index[i + 2]] });
					mIndices.push_back(j++);
				}
			}
			else
			{
				for (int i = 0; i < 12; i = i + 3)
					mVertices.push_back({ positions[index[i]],uvs[index[i + 1]],normals[index[i + 2]] });
				mIndices.push_back(j);
				mIndices.push_back(j+1);
				mIndices.push_back(j+2);
				mIndices.push_back(j);
				mIndices.push_back(j+2);
				mIndices.push_back(j+3);
				j = j + 4;
				
			}
		}

		//ofs << "\n";
	}

	ifs.close();
	//ofs.close();
}

string getToken(string& aLine,bool isFirst)
{
	static int startPosition = 0;

	if (isFirst)
		startPosition = 0;

	while (aLine.length() > startPosition &&
		(aLine[startPosition] == ' ' || aLine[startPosition] == '\n' || aLine[startPosition] == '\t'))
		++startPosition;

	if (aLine.length() <= startPosition)
		return " ";

	int endPosition = startPosition;

	while (aLine.length() > endPosition &&
		aLine[endPosition] != ' ' && aLine[endPosition] != '\n' && aLine[endPosition] != '\t')
		++endPosition;

	int length = endPosition - startPosition;
	int start = startPosition;

	startPosition = endPosition + 1;

	return aLine.substr(start,length);
}

int getNumber(string& aWord, bool isFirst)
{
	static int startPosition = 0;

	if (isFirst)
		startPosition = 0;

	if (aWord.length() <= startPosition || aWord[startPosition] == '/')
	{
		startPosition += 1;
		return 0;
	}

	int endPosition = startPosition;

	while (aWord.length() > endPosition && aWord[endPosition] != '/')
		++endPosition;

	string numberString = aWord.substr(startPosition, endPosition - startPosition);
	startPosition = endPosition + 1;

	return stoi(numberString);
}