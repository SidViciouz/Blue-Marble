#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Clickable.h"
#include "Unclickable.h"
#include "Volume.h"

using namespace std;

class Scene
{
public:
	void Load();
	void CreateVertexIndexBuffer();
	void Update();

	unique_ptr<Clickables> mModels;
	unique_ptr<Unclickables> mWorld;
	unique_ptr<Volume> mVolume;

	unique_ptr<Camera> mCamera = nullptr;

	trans envFeature;

	unique_ptr<Buffer> mVertexBuffer;
	unique_ptr<Buffer> mIndexBuffer;

	vector<Vertex> mAllVertices;
	vector<uint16_t> mAllIndices;
};