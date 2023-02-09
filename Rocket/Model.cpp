#include "Model.h"
#include "IfError.h"

Model::Model(ID3D12Device* device,const char* fileName, ID3D12GraphicsCommandList* commandList)
{
	char type[10];

	ifstream ifs;
	ifs.open(fileName,ios_base::in);

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
		ifs >> type;

		if (strcmp(type, "v") != 0 && strcmp(type, "vt") != 0 && strcmp(type, "vn") != 0 && strcmp(type, "f") != 0)
		{
			string temp;
			getline(ifs, temp);
			continue;
		}

		if (strcmp(type, "v") == 0) // position
		{
			ifs >> x >> y >> z;
			positions.push_back({ x,y,z });
		}
		else if (strcmp(type, "vt") == 0) // uv texture coordinate
		{
			ifs >> x >> y;
			uvs.push_back({ x,y });
		}
		else if (strcmp(type, "vn") == 0) // normal
		{
			ifs >> x >> y >> z;
			normals.push_back({ x,y,z });
		}
		else if (strcmp(type, "f") == 0) // face indices (position/uv/normal)
		{
			for (int i = 0; i < 3; ++i)
			{
				string temp;
				ifs >> temp;

				stringstream ss;
				ss.str(temp);

				ss >> vi;
				ss.ignore();
				ss >> uvi;
				ss.ignore();
				ss >> ni;

				mVertices.push_back({ positions[vi - 1],uvs[uvi - 1],normals[ni - 1] });
				mIndices.push_back(j);
				++j;
			}
		}
	}
	ifs.close();

	mVertexBuffer = make_unique<Buffer>(device,sizeof(Vertex) * mVertices.size());
	mIndexBuffer = make_unique<Buffer>(device, sizeof(uint16_t) * mIndices.size());

	mVertexBuffer->Copy(mVertices.data(), sizeof(Vertex) * mVertices.size(),commandList);
	mIndexBuffer->Copy(mIndices.data(), sizeof(uint16_t) * mIndices.size(), commandList);
}
