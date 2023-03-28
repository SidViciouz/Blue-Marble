#include "Mesh.h"
#include "IfError.h"
#include "Engine.h"
#include <fstream>

Mesh::Mesh(string name, const char* fileName, bool hasRigidBody) : 
	mHasRigidBody{hasRigidBody}
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

	BoundingOrientedBox::CreateFromPoints(mBound, mVertices.size(), &mVertices[0].position, sizeof(Vertex));

	mVertexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(Vertex) * mVertices.size());
	mIndexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(uint16_t) * mIndices.size());

	mVertexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertices.size());
	mIndexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndices.size());

	Engine::mResourceManager->Upload(mVertexUploadBufferIdx, mVertices.data(), sizeof(Vertex) * mVertices.size(), 0);
	Engine::mResourceManager->Upload(mIndexUploadBufferIdx, mIndices.data(), sizeof(uint16_t) * mIndices.size(), 0);
	
	D3D12_RESOURCE_BARRIER barrier[2];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mVertexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	barrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mIndexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	
	Engine::mCommandList->ResourceBarrier(2, barrier);

	Engine::mResourceManager->CopyUploadToBuffer(mVertexUploadBufferIdx, mVertexBufferIdx);
	Engine::mResourceManager->CopyUploadToBuffer(mIndexUploadBufferIdx, mIndexBufferIdx);

	barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	Engine::mCommandList->ResourceBarrier(2, barrier);
}

D3D12_VERTEX_BUFFER_VIEW* Mesh::GetVertexBufferView()
{
	mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBufferIdx)->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertices.size();

	return &mVertexBufferView;
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
	Engine::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());
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

// a/b/c와 같은 형태에서 '/'전까지 반환 끝이라면 " "를 반환
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