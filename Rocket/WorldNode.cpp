#include "WorldNode.h"
#include "Engine.h"
#include "json/json.h"
#include <iostream>

WorldNode::WorldNode(string name)
	: MeshNode(name)
{
	mCharacter = make_shared<MeshNode>("box");
	mCharacter->SetRelativePosition(35.0f, 0.0f, 0.0f);
	mCharacter->SetTextureName("stone");
	AddChild(mCharacter);

	string str;
	Json::Value root;
	root["name"] = "KKK";
	root["age"] = 12;
	root["address"] = "kor";
	root["gfriend"] = true;

	Json::Value family;
	family.append("mother");
	family.append("father");
	family.append("brother");
	root["family"] = family;

	Json::StyledWriter writer;
	str = writer.write(root);
	cout << str << endl;

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
	UpdateCharacter();

	if (!mActivated)
		return;

	SceneNode::Update();
}

void WorldNode::MoveCharacter(const XMFLOAT3& pos)
{
	isMoving = true;

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().Get();
	
	mMoveInfo.totalFrame = 60;
	mMoveInfo.curFrame = 0;
	mMoveInfo.radius = GetScale().x;

	XMFLOAT3 center = mAccumulatedPosition.Get();
	Vector3 v1(curPos.x - center.x, curPos.y - center.y, curPos.z - center.z);
	Vector3 v2(pos.x - center.x, pos.y - center.y, pos.z - center.z);

	v1 = v1.normalize();
	v2 = v2.normalize();

	mMoveInfo.axis = (v1 ^ v2).normalize().v;
	mMoveInfo.angle = acos(v1 * v2)/ mMoveInfo.totalFrame;
}

void WorldNode::UpdateCharacter()
{
	if (!isMoving)
		return;

	if (mMoveInfo.totalFrame < mMoveInfo.curFrame)
	{
		isMoving = false;
		return;
	}

	++mMoveInfo.curFrame;

	XMVECTOR quat = XMQuaternionRotationAxis(XMLoadFloat3(&mMoveInfo.axis), mMoveInfo.angle);

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().Get();
	XMFLOAT3 center = mAccumulatedPosition.Get();

	XMVECTOR xmCurPos = XMLoadFloat3(&curPos);
	XMVECTOR xmCenter = XMLoadFloat3(&center);

	XMVECTOR cur = xmCurPos - xmCenter;

	cur = XMVector3Normalize(XMVector3Rotate(cur, quat))*mMoveInfo.radius;

	xmCurPos = xmCenter + cur;

	XMStoreFloat3(&curPos, xmCurPos);

	mCharacter->SetAccumulatedPosition(curPos);
}