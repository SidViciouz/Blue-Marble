#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <DirectXMath.h>
#include "Util.h"

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
private:
	vector<Vertex> mVertices;
	ComPtr<ID3D12Resource> mVertexBuffer;
	ComPtr<ID3D12Resource> mIndexBuffer;

public:
	Model(const char* fileName);
	Model(const Model& model) = delete;
	Model(Model&& model) = delete;
	Model& operator=(const Model& model) = delete;
	Model& operator=(Model&& model) = delete;

	void Draw();
};