#include "Scene.h"
#include "Engine.h"

Scene::Scene() :
	mSpawnSystem(make_unique<SpawnSystem>()), mSceneRoot(make_shared<SceneNode>())
{

}


void Scene::Update()
{
	for (auto it = mModels->begin(); it != mModels->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}

	for (auto it = mWorld->begin(); it != mWorld->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}

	for (auto it = mVolume->begin(); it != mVolume->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}
}

void Scene::CreateModelSrv(int size)
{
	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		int index = Engine::mDescriptorManager->CreateSrv(model->second->mTexture.mResource.Get(),
			model->second->mTexture.mResource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D);
		
		mSrvIndices[model->first] = index;
	}


	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		int index = Engine::mDescriptorManager->CreateSrv(world->second->mTexture.mResource.Get(),
			world->second->mTexture.mResource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURECUBE);
		mSrvIndices[world->first] = index;
	}
}

void Scene::Spawn()
{
	mSpawnSystem->Spawn(mModels,mWorld,mVolume);
}

void Scene::Destroy()
{
	mSpawnSystem->Destroy(mModels, mWorld, mVolume);
}

bool Scene::IsDestroyQueueEmpty() const
{
	return mSpawnSystem->IsDestroyQueueEmpty();
}

void Scene::Draw()
{
	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		world->second->Draw();
	}

	//선택된 물체에 노란색 테두리 렌더링
	if (mIsModelSelected == true)
	{
		Engine::mCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
		Engine::mCommandList->SetPipelineState(Engine::mPSOs["Selected"].Get());
		Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
			Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
			+ mSelectedModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

		mSelectedModel->Model::Draw();
	}

	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		model->second->Draw();
	}

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		volume->second->Draw();
	}
}

void Scene::UpdateScene(const Timer& timer)
{
	envFeature.view = mCameraNode->GetView();
	envFeature.projection = mCameraNode->GetProjection();
	envFeature.cameraPosition = mCameraNode->GetAccumulatedPosition().Get();
	envFeature.cameraFront = mCameraNode->GetFront();
	envFeature.invViewProjection = mCameraNode->GetInvVIewProjection();
	envFeature.currentTime = timer.GetTime();

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mEnvConstantBufferIdx, &envFeature, sizeof(env), 0);
}