#include "WorldNode.h"
#include "Engine.h"

WorldNode::WorldNode(string name)
	: MeshNode(name)
{
	mCharacter = make_shared<MeshNode>("box");
	mCharacter->SetRelativePosition(35.0f, 0.0f, 0.0f);
	mCharacter->SetTextureName("stone");
	AddChild(mCharacter);
}

void WorldNode::Draw()
{
	if (!mActivated)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["earth"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["earth"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));

	Engine::mCommandList->SetGraphicsRoot32BitConstant(4, 1, 0);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mAllScenes[Engine::mCurrentSceneName]->mShadowMap->GetTextureSrvIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(5, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("world"), DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::Draw();
}

void WorldNode::Update()
{
	/*
	* 여기에 추가로 필요한 것들을 작성한다.
	*/
	/*
	float delta = Engine::mTimer.GetDeltaTime();

	XMVECTOR pos = XMLoadFloat3(&mCharacter->GetRelativePosition().Get());
	XMVECTOR quat = XMVectorSet(0.0f, 0.0f,
		sinf(XMConvertToRadians(20.0f * delta)), cosf(XMConvertToRadians(20.0f * delta)));

	pos = XMVector3Rotate(pos, quat);
	XMFLOAT3 p;
	XMFLOAT4 q;
	XMStoreFloat3(&p, pos);
	XMStoreFloat4(&q, quat);
	mCharacter->SetRelativePosition(p);
	mCharacter->MulRelativeQuaternion(q);
	*/
	if (!mActivated)
		return;

	SceneNode::Update();
}