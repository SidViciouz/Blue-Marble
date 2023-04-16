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
	/*
	* scene의 정보들을 업데이트한다.
	*/
	virtual void								UpdateScene(const Timer& timer);
	/*
	* scene을 draw한다.
	*/
	virtual void								DrawScene() const;
	/*
	* shadow Map에 scene을 draw한다.
	*/
	virtual void								RenderShadowMap() const;
	/*
	* 각 scene에서 프레임마다 업로드할 데이터를 나타낸다.
	*/
	env											envFeature;
	/*
	* scene의 root노드를 나타낸다.
	* 이 노드의 자식으로 추가해야 업데이트와 draw가 실행된다.
	*/
	shared_ptr<SceneNode>						mSceneRoot;
	/*
	* 카메라 노드를 가리키는 포인터이다.
	*/
	shared_ptr<CameraNode>						mCameraNode;
	/*
	* 조명 노드를 가리키는 포인터이다.
	*/
	vector<shared_ptr<LightNode>>				mLightNodes;
	/*
	* shadow map을 나타낸다.
	*/
	shared_ptr<ShadowMap>						mShadowMap;

};