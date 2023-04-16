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
	* scene�� �������� ������Ʈ�Ѵ�.
	*/
	virtual void								UpdateScene(const Timer& timer);
	/*
	* scene�� draw�Ѵ�.
	*/
	virtual void								DrawScene() const;
	/*
	* shadow Map�� scene�� draw�Ѵ�.
	*/
	virtual void								RenderShadowMap() const;
	/*
	* �� scene���� �����Ӹ��� ���ε��� �����͸� ��Ÿ����.
	*/
	env											envFeature;
	/*
	* scene�� root��带 ��Ÿ����.
	* �� ����� �ڽ����� �߰��ؾ� ������Ʈ�� draw�� ����ȴ�.
	*/
	shared_ptr<SceneNode>						mSceneRoot;
	/*
	* ī�޶� ��带 ����Ű�� �������̴�.
	*/
	shared_ptr<CameraNode>						mCameraNode;
	/*
	* ���� ��带 ����Ű�� �������̴�.
	*/
	vector<shared_ptr<LightNode>>				mLightNodes;
	/*
	* shadow map�� ��Ÿ����.
	*/
	shared_ptr<ShadowMap>						mShadowMap;

};