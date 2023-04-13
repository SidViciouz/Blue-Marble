#include "InventoryNode.h"
#include "Engine.h"
#include "ItemInputComponent.h"

InventoryNode::InventoryNode(string name)
	: MeshNode(name)
{
	for (int i = 0; i < mStoredItems.size(); ++i)
	{
		mStoredItems[i] = make_shared<ItemNode>("box");
		mStoredItems[i]->SetRelativePosition(-0.5f+i*0.1f,0.0f,-1.0f);
		//mStoredItems[i]->SetDraw(true);
		mStoredItems[i]->mInputComponent = Engine::mInputManager->Build<ItemInputComponent>(mStoredItems[i], "MainScene");
		mStoredItems[i]->SetIndex(i);
		AddChild(mStoredItems[i]);
	}

	for (int i = 0; i < mDroppedItems.size(); ++i)
	{
		mDroppedItems[i] = make_shared<ItemNode>("box");
		mDroppedItems[i]->SetRelativePosition(-1.0f + i*0.5f, -1.5f, -1.0f);
		//mDroppedItems[i]->SetDraw(true);
		mDroppedItems[i]->mInputComponent = Engine::mInputManager->Build<ItemInputComponent>(mDroppedItems[i], "MainScene");
		mDroppedItems[i]->SetIndex(i);
		AddChild(mDroppedItems[i]);
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
		Engine::mDescriptorManager->GetGpuHandle(Engine::mTextureManager->GetTextureIndex("backPack"), DescType::SRV));
	Engine::mMeshManager->Draw(mMeshName);


	//draw child nodes
	SceneNode::Draw();
}

void InventoryNode::Update()
{

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

void InventoryNode::SetClickedMesh(const int& index)
{
	mClickedMeshIndex = index;
	mHaveClickedMesh = true;
}

void InventoryNode::UnsetHaveClickedMesh()
{
	mHaveClickedMesh = false;
}

const int& InventoryNode::GetClickedMeshIndex() const
{
	return mClickedMeshIndex;
}

const bool& InventoryNode::HaveClickedMesh() const
{
	return mHaveClickedMesh;
}

bool InventoryNode::StoreItem(const int& index)
{
	if (index <= mDroppedItems.size() && mDroppedItems[index]->GetDraw())
	{
		mDroppedItems[index]->SetDraw(false);

		for (int i = 0; i < mStoredItems.size(); ++i)
		{
			if (!mStoredItems[i]->GetDraw())
			{
				mStoredItems[i]->SetMeshName(mDroppedItems[index]->GetMeshName());
				mStoredItems[i]->SetDraw(true);
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
		mStoredItems[index]->SetDraw(false);

		for (int i = 0; i < mDroppedItems.size(); ++i)
		{
			if (!mDroppedItems[i]->GetDraw())
			{
				mDroppedItems[i]->SetMeshName(mStoredItems[index]->GetMeshName());
				mDroppedItems[i]->SetDraw(true);
				return true;
			}
		}
	}

	return false;
}
