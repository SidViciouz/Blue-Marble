#include "Mesh.h"
#include "IfError.h"
#include "Engine.h"
#include <fstream>

Mesh::Mesh(string name, const char* fileName)
{
	LoadFromFile(fileName);

	CalculateTB();

	BoundingOrientedBox::CreateFromPoints(mBound, mVertices.size(), &mVertices[0].position, sizeof(Vertex));

	UploadBuffers();
}

D3D12_VERTEX_BUFFER_VIEW* Mesh::GetVertexBufferView()
{
	mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBufferIdx)->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertices.size();

	mTBBufferView.BufferLocation = Engine::mResourceManager->GetResource(mTBBufferIdx)->GetGPUVirtualAddress();
	mTBBufferView.StrideInBytes = sizeof(TB);
	mTBBufferView.SizeInBytes = sizeof(TB) * mTBs.size();

	mBufferViewArray[0] = mVertexBufferView;
	mBufferViewArray[1] = mTBBufferView;

	return mBufferViewArray;
}

D3D12_INDEX_BUFFER_VIEW* Mesh::GetIndexBufferView()
{
	mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexBufferIdx)->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndices.size();

	return &mIndexBufferView;
}

void Mesh::Draw()
{
	Engine::mCommandList->IASetVertexBuffers(0, 2, GetVertexBufferView());
	Engine::mCommandList->IASetIndexBuffer(GetIndexBufferView());
	Engine::mCommandList->DrawIndexedInstanced(mIndices.size() , 1, 0, 0, 0);
}

// string에서 공백전까지의 token을 반환, 문자열의 끝이라면 " "를 반환
// 입력 문자열의 시작부터 token을 반환하려면 isFirst = true를 입력
// 토큰을 반환한 적 있고, 그 다음부터 이어서 반환하려면 isFirst = false를 입력한다.
string Mesh::getToken(string& aLine, bool isFirst)
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

	return aLine.substr(start, length);
}

// a/b/c와 같은 형태에서 '/'전까지 반환 끝이라면 0을 반환
// 입력 문자열의 시작부터 token을 반환하려면 isFirst = true를 입력
// 토큰을 반환한 적 있고, 그 다음부터 이어서 반환하려면 isFirst = false를 입력한다.
int Mesh::getNumber(string& aWord, bool isFirst)
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

void Mesh::LoadFromFile(const char* fileName)
{
	ifstream ifs;
	ifs.open(fileName, ios_base::in);

	if (ifs.fail())
		IfError::Throw(L"model file open error!");

	float x, y, z;
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
			uvs.push_back({ x,y });
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
				index.push_back(getNumber(value, true) - 1);
				index.push_back(getNumber(value, false) - 1);
				index.push_back(getNumber(value, false) - 1);
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
				mIndices.push_back(j + 1);
				mIndices.push_back(j + 2);
				mIndices.push_back(j);
				mIndices.push_back(j + 2);
				mIndices.push_back(j + 3);
				j = j + 4;

			}
		}
	}

	ifs.close();
}

void Mesh::CalculateTB()
{
	mTBs = vector<TB>(mVertices.size(), { {0,0,0},{0,0,0} });
	vector<int> averageCoef(mVertices.size(), 0);

	for (int i=0; i< mIndices.size(); i += 3)
	{
		int a[3];
		for (int j = 0; j < 3; ++j)
			a[j] = mIndices[i + j];

		float u0 = mVertices[a[1]].uv.x - mVertices[a[0]].uv.x;
		float v0 = mVertices[a[1]].uv.y - mVertices[a[0]].uv.y;
		float u1 = mVertices[a[2]].uv.x - mVertices[a[0]].uv.x;
		float v1 = mVertices[a[2]].uv.y - mVertices[a[0]].uv.y;

		XMVECTOR xma = XMLoadFloat3(&mVertices[a[0]].position);
		XMVECTOR xmb = XMLoadFloat3(&mVertices[a[1]].position);
		XMVECTOR xmc = XMLoadFloat3(&mVertices[a[2]].position);

		XMVECTOR xme0 = xmb - xma;
		XMVECTOR xme1 = xmc - xma;

		XMFLOAT3 e0, e1;
		XMStoreFloat3(&e0, xme0);
		XMStoreFloat3(&e1, xme1);

		float coef = 1.0f / ((u0 * v1) - (v0 * u1));
		float M1[2][2] = {
		v1, -v0,
		-u1, u0
		};

		float M2[2][3] = {
			e0.x, e0.y, e0.z,
			e1.x, e1.y, e1.z
		};

		float TB[2][3] =
		{
			M1[0][0] * M2[0][0] + M1[0][1] * M2[1][0],
			M1[0][0] * M2[0][1] + M1[0][1] * M2[1][1],
			M1[0][0] * M2[0][2] + M1[0][1] * M2[1][2],

			M1[1][0] * M2[0][0] + M1[1][1] * M2[1][0],
			M1[1][0] * M2[0][1] + M1[1][1] * M2[1][1],
			M1[1][0] * M2[0][2] + M1[1][1] * M2[1][2],
		};
		XMFLOAT3 T = { coef* TB[0][0], coef*TB[0][1], coef*TB[0][2] };
		XMFLOAT3 B = { coef* TB[1][0], coef*TB[1][1], coef*TB[1][2] };
		/*
		XMVECTOR xmtangent = XMLoadFloat3(&T);
		XMStoreFloat3(&T, XMVector3Normalize(xmtangent));

		XMVECTOR xmbitangent = XMLoadFloat3(&B);
		XMStoreFloat3(&B, XMVector3Normalize(xmbitangent));
		*/
		for (int j = 0; j < 3; ++j)
		{
			averageCoef[a[j]]++;

			XMVECTOR xmnormal = XMLoadFloat3(&mVertices[a[j]].normal);

			XMVECTOR xmtangent = XMLoadFloat3(&T);
			xmtangent = xmtangent - XMVector3Dot(xmtangent, xmnormal) * xmnormal;

			XMVECTOR xmbitangent = XMLoadFloat3(&B);
			xmbitangent = xmbitangent - XMVector3Dot(xmbitangent, xmnormal) * xmnormal
				- XMVector3Dot(xmbitangent, xmtangent) * xmtangent;

			XMStoreFloat3(&T, xmtangent);
			XMStoreFloat3(&B, xmbitangent);

			mTBs[a[j]].tangent.x += T.x;
			mTBs[a[j]].tangent.y += T.y;
			mTBs[a[j]].tangent.z += T.z;

			mTBs[a[j]].bitangent.x += B.x;
			mTBs[a[j]].bitangent.y += B.y;
			mTBs[a[j]].bitangent.z += B.z;
		}
		/*
		printf("%f %f %f, %f %f %f\n",
			mTBs[mIndices[i]].tangent.x,
			mTBs[mIndices[i]].tangent.y,
			mTBs[mIndices[i]].tangent.z,
			mTBs[mIndices[i]].bitangent.x,
			mTBs[mIndices[i]].bitangent.y,
			mTBs[mIndices[i]].bitangent.z);
			*/
	}

	for (auto index : mIndices)
	{
		XMVECTOR xmnormal = XMLoadFloat3(&mVertices[index].normal);

		XMVECTOR xmtangent = XMLoadFloat3(&mTBs[index].tangent);
		xmtangent /= averageCoef[index];
		xmtangent = xmtangent - XMVector3Dot(xmtangent, xmnormal) * xmnormal;
		XMStoreFloat3(&mTBs[index].tangent,xmtangent);

		XMVECTOR xmbitangent = XMLoadFloat3(&mTBs[index].bitangent);
		xmbitangent /= averageCoef[index];
		xmbitangent = xmbitangent - XMVector3Dot(xmbitangent, xmnormal) * xmnormal
			- XMVector3Dot(xmbitangent, xmtangent) * xmtangent;
		XMStoreFloat3(&mTBs[index].bitangent, xmbitangent);
		
		printf("%f %f %f, %f %f %f\n",
			mTBs[index].tangent.x,
			mTBs[index].tangent.y,
			mTBs[index].tangent.z,
			mTBs[index].bitangent.x,
			mTBs[index].bitangent.y,
			mTBs[index].bitangent.z);
			
	}
}

void Mesh::UploadBuffers()
{
	mVertexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(Vertex) * mVertices.size());
	mIndexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(uint16_t) * mIndices.size());
	mTBBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(TB) * mTBs.size());

	mVertexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertices.size());
	mIndexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndices.size());
	mTBUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(TB) * mTBs.size());

	Engine::mResourceManager->Upload(mVertexUploadBufferIdx, mVertices.data(), sizeof(Vertex) * mVertices.size(), 0);
	Engine::mResourceManager->Upload(mIndexUploadBufferIdx, mIndices.data(), sizeof(uint16_t) * mIndices.size(), 0);
	Engine::mResourceManager->Upload(mTBUploadBufferIdx, mTBs.data(), sizeof(TB) * mTBs.size(), 0);

	D3D12_RESOURCE_BARRIER barrier[3];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mVertexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	barrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mIndexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	barrier[2] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mTBBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	Engine::mCommandList->ResourceBarrier(3, barrier);

	Engine::mResourceManager->CopyUploadToBuffer(mVertexUploadBufferIdx, mVertexBufferIdx);
	Engine::mResourceManager->CopyUploadToBuffer(mIndexUploadBufferIdx, mIndexBufferIdx);
	Engine::mResourceManager->CopyUploadToBuffer(mTBUploadBufferIdx, mTBBufferIdx);

	barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier[2].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[2].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	Engine::mCommandList->ResourceBarrier(3, barrier);
}