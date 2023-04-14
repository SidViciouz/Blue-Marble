
#include "TextNode.h"
#include "Engine.h"

TextNode::TextNode() 
	: SceneNode()
{
	
}

void TextNode::Draw()
{
	
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Text"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Text"].Get());

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRoot32BitConstant(2, mText.size(), 0);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mTextManager->GetTextInfoSrvIdx(), DescType::SRV)
	);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(4,
		Engine::mDescriptorManager->GetGpuHandle(mUploadBufferSrvIdx, DescType::SRV)
	);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(5,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mTextManager->GetTextureSrvIdx(), DescType::SRV)
	);

	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(6, 1, 0, 0);

	SceneNode::Draw();
}

void TextNode::Update()
{
	SceneNode::Update();
}

void TextNode::SetText(const string& text)
{
	mText = text;

	for (int i = 0; i < mText.size(); ++i)
	{
		mText32bits[i] = mText[i];
	}

	mUploadBufferIdx =  Engine::mResourceManager->CreateUploadBuffer(mText.size() * sizeof(int));

	Engine::mResourceManager->Upload(mUploadBufferIdx, mText32bits, mText.size() * sizeof(int), 0);

	mUploadBufferSrvIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mUploadBufferIdx),
		DXGI_FORMAT_UNKNOWN,
		D3D12_SRV_DIMENSION_BUFFER,
		1,
		mText.size(),
		sizeof(int));
}

const string& TextNode::GetText() const
{
	return mText;
}
