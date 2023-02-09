#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <DirectXMath.h>
#include "Util.h"
#include "Buffer.h"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

class Model
{
public:
	Model(ID3D12Device* device,const char* fileName, ID3D12GraphicsCommandList* commandList);
	Model(const Model& model) = delete;
	Model(Model&& model) = delete;
	Model& operator=(const Model& model) = delete;
	Model& operator=(Model&& model) = delete;

	void Draw();

private:
	vector<Vertex> mVertices;
	vector<uint16_t> mIndices;
	unique_ptr<Buffer> mVertexBuffer = nullptr;
	unique_ptr<Buffer> mIndexBuffer = nullptr;
};