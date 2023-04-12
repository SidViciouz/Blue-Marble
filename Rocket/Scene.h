#pragma once
#include "Util.h"
#include "SceneNode.h"
#include "Timer.h"
#include "CameraNode.h"
#include "LightNode.h"
#include "ShadowMap.h"

using namespace std;


class Scene
{
public:
												Scene();

	env											envFeature;

	shared_ptr<SceneNode>						mSceneRoot;

	shared_ptr<CameraNode>						mCameraNode;
	vector<shared_ptr<LightNode>>				mLightNodes;

	shared_ptr<ShadowMap>						mShadowMap;

	virtual void								UpdateScene(const Timer& timer);
	virtual void								DrawScene() const;
	virtual void								RenderShadowMap() const;
};