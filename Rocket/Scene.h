#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"

using namespace std;

class Scene
{
public:
	unique_ptr<Models> mModels;

	unique_ptr<Camera> mCamera = nullptr;

	trans envFeature;
};