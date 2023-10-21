#include "DiceNode.h"
#include "Engine.h"

DiceNode::DiceNode(string name)
	: MeshNode(name)
{
	mFace[1] = Vector3(1,0,0);
	mFace[2] = Vector3(-1, 0, 0);
	mFace[3] = Vector3(0, 1, 0);
	mFace[4] = Vector3(0, -1, 0);
	mFace[5] = Vector3(0, 0, 1);
	mFace[6] = Vector3(0, 0, -1);
}

void DiceNode::Draw()
{
	if (!mActivated)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Dice"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::Draw();
}

int DiceNode::UpperSide()
{
	Vector3 lUp(0, 1, 0);

	Matrix3x3 lInvRotation(GetAccumulatedQuaternion().Conjugate());

	lUp = lInvRotation * lUp;

	float maxDotValue = 0;
	float curDotValue;
	float maxDotValueFace = 0;
	for (int i = 1; i < 7; ++i)
	{
		if ((curDotValue = Vector3::Dot(lUp, mFace[i])) > 0)
		{
			if (curDotValue > maxDotValue)
			{
				maxDotValue = curDotValue;
				maxDotValueFace = i;
			}
		}
	}

	return maxDotValueFace;
}