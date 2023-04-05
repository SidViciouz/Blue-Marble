#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"
#include "Clickable.h"
#include "Unclickable.h"
#include "Volume.h"
#include "SpawnSystem.h"
#include "SceneNode.h"

using namespace std;


class Scene
{
public:
												Scene();
	void										Update();
	void										CreateModelSrv(int size);
	/*
	* Spawn system�� �ִ� spwan ��� ���� object���� ��� spwan�Ѵ�.
	*/
	void										Spawn();
	/*
	* Spawn system�� �ִ� destroy ��� ���� object���� ��� destroy�Ѵ�.
	*/
	void										Destroy();
	/*
	* destroy Queue�� ����ִ��� ���θ� ��ȯ�Ѵ�.
	*/
	bool										IsDestroyQueueEmpty() const;
	/*
	* scene ���� object���� ��� draw�Ѵ�.
	*/
	virtual void								Draw();

	shared_ptr<Clickables>						mModels;
	shared_ptr<Unclickables>					mWorld;
	shared_ptr<Volumes>							mVolume;

	unique_ptr<Camera>							mCamera = nullptr;

	env											envFeature;

	unique_ptr<SpawnSystem>						mSpawnSystem;

	unordered_map<string, int>					mSrvIndices;
	unordered_map<string, int>					mInvisibleUavIndices;

	bool										mIsModelSelected = false;
	string										mSelectedModelName;
	shared_ptr<Model>							mSelectedModel;

	shared_ptr<SceneNode>						mSceneRoot;

	virtual void								UpdateScene(float deltaTime);
};