#include "InventoryNode.h"
#include "Engine.h"
#include "ItemInputComponent.h"

InventoryNode::InventoryNode(string name)
	: MeshNode(name)
{
	for (int i = 0; i < mStoredItems.size(); ++i)
	{
		mStoredItems[i] = make_shared<ItemNode>("box");
		mStoredItems[i]->SetRelativePosition(-0.5f+i*0.25f,0.0f,-1.0f);
		mStoredItems[i]->mInputComponent = Engine::mInputManager->Build<ItemInputComponent>(mStoredItems[i], "MainScene");
		AddChild(mStoredItems[i]);
		mStoredItems[i]->SetIndex(i);
	}

	for (int i = 0; i < mDroppedItems.size(); ++i)
	{
		mDroppedItems[i] = make_shared<ItemNode>("box");
		mDroppedItems[i]->SetRelativePosition(-1.0f + i*0.5f, -1.5f, -1.0f);
		mDroppedItems[i]->mInputComponent = Engine::mInputManager->Build<ItemInputComponent>(mDroppedItems[i], "MainScene");
		AddChild(mDroppedItems[i]);
		mDroppedItems[i]->SetIndex(i);
	}
}

void InventoryNode::Draw()
{
	if (!mIsShowUp)
		return;

	//draw inventory window
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));
	Engine::mMeshManager->Draw(mMeshName);

	//draw child nodes
	SceneNode::Draw();
}

void InventoryNode::DrawWithoutSetting()
{
	SceneNode::DrawWithoutSetting();
}

void InventoryNode::Update()
{
	for (int i = 0; i < mOverlappedMeshes.size(); ++i)
	{
		mOverlappedMeshes[i] = nullptr;
		mDroppedItems[i]->SetDraw(false);
	}


	MeshNode::Update();
}

void InventoryNode::SetIsShowUp(bool value)
{
	mIsShowUp = value;
}

const bool& InventoryNode::GetIsShowUp() const
{
	return mIsShowUp;
}

void InventoryNode::ToggleIsShowUp()
{
	if (mIsShowUp)
		mIsShowUp = false;
	else
		mIsShowUp = true;
}


bool InventoryNode::StoreItem(const int& index)
{
	if (index <= mDroppedItems.size() && mDroppedItems[index]->GetDraw())
	{
		for (int i = 0; i < mStoredItems.size(); ++i)
		{
			if (!mStoredItems[i]->GetDraw())
			{
				mStoredItems[i]->SetMeshName(mDroppedItems[index]->GetMeshName());
				mStoredItems[i]->SetDraw(true);
				mStoredItems[i]->SetIsStored(true);

				mOverlappedMeshes[index]->SetActivated(false);
				mStoredMeshes[i] = mOverlappedMeshes[index];
				return true;
			}
		}
	}

	return false;
}

bool InventoryNode::DropItem(const int& index)
{
	if(index <= mStoredItems.size() && mStoredItems[index]->GetDraw())
	{
		for (int i = 0; i < mDroppedItems.size(); ++i)
		{
			if (!mDroppedItems[i]->GetDraw())
			{
				XMFLOAT3 pos = Engine::mAllScenes[Engine::mCurrentSceneName]->mCameraNode->GetAccumulatedPosition().Get();

				mStoredItems[index]->SetDraw(false);
				mStoredMeshes[index]->SetActivated(true);
				mStoredMeshes[index]->SetAccumulatedPosition(
					pos.x,pos.y,pos.z
				);
				mStoredMeshes[index]->mRigidBodyComponent->mVelocity = { 0.0f,0.0f,0.0f };
				mStoredMeshes[index]->mRigidBodyComponent->mAngularVel = { 0.5f,0.5f,0.5f };

				mDroppedItems[i]->SetMeshName(mStoredItems[index]->GetMeshName());
				mDroppedItems[i]->SetDraw(true);
				mDroppedItems[i]->SetIsStored(false);

				return true;
			}
		}
	}

	return false;
}

void InventoryNode::OverlappedNode(shared_ptr<MeshNode> overlapped)
{
	if (!overlapped->GetActivated())
		return;

	for (int i = 0; i < mOverlappedMeshes.size(); ++i)
	{
		if (mOverlappedMeshes[i] == nullptr)
		{
			mOverlappedMeshes[i] = overlapped;
			mDroppedItems[i]->SetMeshName(overlapped->GetMeshName());
			mDroppedItems[i]->SetDraw(true);
			return;
		}
	}
}