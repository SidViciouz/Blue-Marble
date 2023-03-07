#pragma once
#include "Util.h"
#include "Model.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Clickable.h"
#include "Unclickable.h"
#include "Volume.h"
#include "SpawnSystem.h"

using namespace std;

class Scene
{
public:
												Scene();
	void										Load();
	void										Update();
	void										CreateModelSrv(int size);
	void										CreateVolumeUav(int size);
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

	shared_ptr<Clickables>						mModels;
	shared_ptr<Unclickables>					mWorld;
	shared_ptr<Volumes>							mVolume;

	unique_ptr<Camera>							mCamera = nullptr;

	trans										envFeature;

	ComPtr<ID3D12DescriptorHeap>				mSrvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>				mVolumeUavHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>				mVolumeUavHeapInvisible = nullptr;

	unique_ptr<SpawnSystem>						mSpawnSystem;
};