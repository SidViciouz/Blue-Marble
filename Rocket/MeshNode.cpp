#include "MeshNode.h"
#include "Engine.h"

MeshNode::MeshNode(string name)
	: mMeshName{ name }, SceneNode()
{

}

void MeshNode::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["planet"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["planet"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mPermutationDescriptorIdx, DescType::SRV));
	
	Engine::mMeshManager->Draw(mMeshName);

	if (mCollisionComponent != nullptr)
	{
		Engine::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["ColliderShape"].Get());
		Engine::mCommandList->SetPipelineState(Engine::mPSOs["ColliderShape"].Get());
		mCollisionComponent->Draw();
	}

	//drawing child nodes
	for (auto& childNode : mChildNodes)
	{
		childNode->Draw();
	}

}

void MeshNode::Update()
{
	/*
	* 여기에 추가로 필요한 것들을 작성한다.
	*/

	SceneNode::Update();
}

const string& MeshNode::GetMeshName() const
{
	return mMeshName;
}