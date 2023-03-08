#include "Model.h"
#include "IfError.h"
#include "Game.h"
#include "DDSTextureLoader.h"
#include "RigidBody.h"

string getToken(string& aLine, bool isFirst); 
int getNumber(string& aWord, bool isFirst);

Model::Model(const char* fileName, const wchar_t* name)
{
	mFileName = fileName;
	mName = name;
	if(fileName != nullptr && name != nullptr)
		Load();
	mRigidBody = make_shared<RigidBody>(this);
}

void Model::Update()
{
	XMFLOAT3 pos = mPosition.Get();
	XMFLOAT3 scale = mScale;
	XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);

	XMStoreFloat4x4(&mObjFeature.world, world);
	
	mDirty = false;
}

void Model::Load()
{
	ifstream ifs;
	ifs.open(mFileName, ios_base::in);

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

	mVertexBufferSize = mVertices.size();
	mIndexBufferSize = mIndices.size();

	BoundingOrientedBox::CreateFromPoints(mBound, mVertices.size(), &mVertices[0].position, sizeof(Vertex));

	IfError::Throw(CreateDDSTextureFromFile12(Pipeline::mDevice.Get(), Game::mCommandList.Get(), mName,
		mTexture.mResource, mTexture.mUpload),
		L"load dds texture error!");

	mVertexBuffer = make_unique<Buffer>(Pipeline::mDevice.Get(), sizeof(Vertex) * mVertices.size());
	mIndexBuffer = make_unique<Buffer>(Pipeline::mDevice.Get(), sizeof(uint16_t) * mIndices.size());

	mVertexBuffer->Copy(mVertices.data(), sizeof(Vertex) * mVertices.size(), Game::mCommandList.Get());
	mIndexBuffer->Copy(mIndices.data(), sizeof(uint16_t) * mIndices.size(), Game::mCommandList.Get());
}

// string에서 공백전까지의 token을 반환, 문자열의 끝이라면 " "를 반환
// 입력 문자열의 시작부터 token을 반환하려면 isFirst = true를 입력
// 토큰을 반환한 적 있고, 그 다음부터 이어서 반환하려면 isFirst = false를 입력한다.
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

// a/b/c와 같은 형태에서 '/'전까지 반환 끝이라면 " "를 반환
// 입력 문자열의 시작부터 token을 반환하려면 isFirst = true를 입력
// 토큰을 반환한 적 있고, 그 다음부터 이어서 반환하려면 isFirst = false를 입력한다.
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

D3D12_VERTEX_BUFFER_VIEW* Model::GetVertexBufferView()
{
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGpuAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertices.size();

	return &mVertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* Model::GetIndexBufferView()
{
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGpuAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndices.size();

	return &mIndexBufferView;
}

void Model::Draw()
{
	Game::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());
	Game::mCommandList->IASetIndexBuffer(GetIndexBufferView());
	Game::mCommandList->DrawIndexedInstanced(mIndexBufferSize, 1, 0, 0, 0);
}

int Model::mNextObjConstantIndex = 0;