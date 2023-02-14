#include "Model.h"
#include "IfError.h"

string getToken(string& aLine, bool isFirst);

Model::Model(ID3D12Device* device,const char* fileName, ID3D12GraphicsCommandList* commandList)
{
	ifstream ifs;
	ifs.open(fileName,ios_base::in);

	ofstream ofs;
	ofs.open("test.txt", ios_base::out);

	if (ifs.fail())
		IfError::Throw(L"model file open error!");

	/*
	float x, y, z;
	unsigned int vi, uvi, ni;
	vector<XMFLOAT3> positions;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> normals;
	uint16_t j = 0;
	*/

	while (!ifs.eof())
	{
		string aLine;
		getline(ifs, aLine);

		string type = getToken(aLine, true);

		if (type.compare("v") == 0 || type.compare("vt") == 0 || type.compare("vn") == 0 || type.compare("f") == 0)
		{
			ofs << getToken(aLine, false) << " ";
		}
		ofs << "\n";
	}

	ifs.close();
	ofs.close();
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