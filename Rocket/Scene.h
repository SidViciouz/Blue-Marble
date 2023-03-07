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