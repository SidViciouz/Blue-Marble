#include "Model.h"
#include "IfError.h"
#include "Game.h"
#include "DDSTextureLoader.h"

string getToken(string& aLine, bool isFirst); 
int getNumber(string& aWord, bool isFirst);

Model::Model(int sceneIndex,const char* fileName, const wchar_t* name)
{
	mSceneIndex = sceneIndex;
	mFileName = fileName;
	mName = name;
}

//이 메서드 entity로 옮기는게 나을 것 같다.
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

	mVertexBufferOffset = Game::mScenes[mSceneIndex]->mAllVertices.size();
	mIndexBufferOffset = Game::mScenes[mSceneIndex]->mAllIndices.size();

	Game::mScenes[mSceneIndex]->mAllVertices.insert(Game::mScenes[mSceneIndex]->mAllVertices.end(), mVertices.begin(), mVertices.end());
	Game::mScenes[mSceneIndex]->mAllIndices.insert(Game::mScenes[mSceneIndex]->mAllIndices.end(), mIndices.begin(), mIndices.end());

	BoundingOrientedBox::CreateFromPoints(mBound, mVertices.size(), &mVertices[0].position, sizeof(Vertex));

	IfError::Throw(CreateDDSTextureFromFile12(Pipeline::mDevice.Get(), Game::mCommandList.Get(), mName,
		mTexture.mResource, mTexture.mUpload),
		L"load dds texture error!");
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

int Model::mNextObjConstantIndex = 0;