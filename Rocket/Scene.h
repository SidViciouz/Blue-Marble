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
	* Spawn system에 있는 spwan 대기 중인 object들을 모두 spwan한다.
	*/
	void										Spawn();
	/*
	* Spawn system에 있는 destroy 대기 중인 object들을 모두 destroy한다.
	*/
	void										Destroy();
	/*
	* destroy Queue가 비어있는지 여부를 반환한다.
	*/
	bool										IsDestroyQueueEmpty() const;
	/*
	* scene 내의 object들을 모두 draw한다.
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