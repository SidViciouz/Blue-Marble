#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"

using namespace std;

class Scene
{
	unordered_map<string, unique_ptr<Model>> mModels;

	unique_ptr<Camera> mCamera = nullptr;

	trans envFeature;
};