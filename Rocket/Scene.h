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
	void										Load();
	void										Update();
	void										CreateModelSrv(int size);
	void										CreateVolumeUav(int size);
												
	unique_ptr<Clickables>						mModels;
	unique_ptr<Unclickables>					mWorld;
	unique_ptr<Volumes>							mVolume;

	unique_ptr<Camera>							mCamera = nullptr;

	trans										envFeature;

	ComPtr<ID3D12DescriptorHeap>				mSrvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>				mVolumeUavHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>				mVolumeUavHeapInvisible = nullptr;
};