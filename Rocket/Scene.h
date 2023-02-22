#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"
#include "Pipeline.h"

using namespace std;

class Scene
{
public:
	void LoadModels();
	void CreateVertexIndexBuffer();

	unique_ptr<Models> mModels;

	unique_ptr<Camera> mCamera = nullptr;

	trans envFeature;

	unique_ptr<Buffer> mVertexBuffer;
	unique_ptr<Buffer> mIndexBuffer;

	vector<Vertex> mAllVertices;
	vector<uint16_t> mAllIndices;
};