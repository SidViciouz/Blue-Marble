#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <DirectXMath.h>
#include "Util.h"
#include "Buffer.h"
#include "Constant.h"
#include "Texture.h"
#include "Entity.h"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

class RigidBody;

struct Vertex
{
	XMFLOAT3									position;
	XMFLOAT2									uv;
	XMFLOAT3									normal;
};

class Model : public Entity
{
public:
												Model(const char* fileName, const wchar_t* name,bool hasRigidBody = true);
												Model(const Model& model) = delete;
												Model(Model&& model) = delete;
	Model&										operator=(const Model& model) = delete;
	Model&										operator=(Model&& model) = delete;

	void										Load();
	virtual void								Update() override;
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();
	virtual void								Draw();

	const char*									mFileName;
	const wchar_t*								mName;
	int											mSceneIndex;

	vector<Vertex>								mVertices;
	vector<uint16_t>							mIndices;

	obj											mObjFeature =
	{ {1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f},
		{0.7f,0.9f,0.75f},
		0.3f,
		{0.1f,0.1f,0.1f}
	};

	XMFLOAT3									mScale = { 1.0f,1.0f,1.0f };

	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	int											mVertexBufferSize = 0;
	int											mIndexBufferSize = 0;

	Texture										mTexture;

	BoundingOrientedBox							mBound;

	int											mObjIndex = mNextObjConstantIndex++;
	static int									mNextObjConstantIndex;

	unique_ptr<Buffer>							mVertexBuffer;
	unique_ptr<Buffer>							mIndexBuffer;

	shared_ptr<RigidBody>						mRigidBody;
	bool										mHasRigidBody = true;

	//
	D3D12_PRIMITIVE_TOPOLOGY					mTopology;
	string										mRootSignature;
	string										mPso;
	string										mId;
};

using Models = unordered_map<string, shared_ptr<Model>>;