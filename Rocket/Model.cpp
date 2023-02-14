#include "Model.h"
#include "IfError.h"

Model::Model(ID3D12Device* device,const char* fileName, ID3D12GraphicsCommandList* commandList)
{
	char type[30];

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
			//텍스처 좌표 반대로 저장해야함. y = 1-y
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
			//face 가 점 4개로 구성된 경우도 있음. 이를 처리해야함.
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

				XMFLOAT3 position = { 0.0f,0.0f,0.0f };
				XMFLOAT2 uv = { 0.0f,0.0f };
				XMFLOAT3 normal = { 0.0f,0.0f,0.0f };

				if (uvs.size() >= uvi)
					uv = uvs[uvi - 1];

				if (normals.size() >= ni)
					normal = normals[ni - 1];

				if (positions.size() >= vi)
					position = positions[vi - 1];
				
				mVertices.push_back({ position,uv,normal});
				mIndices.push_back(j);
				++j;
			}
		}
	}
	ifs.close();
}