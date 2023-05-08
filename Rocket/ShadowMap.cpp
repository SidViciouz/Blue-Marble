#include "ShadowMap.h"
#include "Engine.h"

ShadowMap::ShadowMap(int width,int height)
	: mWidth{ width }, mHeight{ height }
{
	mTextureIdx =  Engine::mResourceManager->CreateTexture2D(
		mWidth,
		mHeight,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	mTextureSrvIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mTextureIdx),
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D12_SRV_DIMENSION_TEXTURE2D
	);

	mTextureDsvIdx = Engine::mDescriptorManager->CreateDsv(
		Engine::mResourceManager->GetResource(mTextureIdx),
		D3D12_DSV_DIMENSION_TEXTURE2D
	);
	
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = static_cast<float>(mWidth);
	mViewport.Height = static_cast<float>(mHeight);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mScissor.left = 0;
	mScissor.top = 0;
	mScissor.right = mWidth;
	mScissor.bottom = mHeight;
}

void ShadowMap::PipelineSetting()
{
	Engine::mCommandList->RSSetScissorRects(1, &mScissor);
	Engine::mCommandList->RSSetViewports(1, &mViewport);
	Engine::mCommandList->SetDescriptorHeaps(1, Engine::mDescriptorManager->GetHeapAddress(DescType::UAV));

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = Engine::mDescriptorManager->GetCpuHandle(mTextureDsvIdx, DescType::DSV);
	Engine::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	Engine::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["ShadowMap"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["ShadowMap"].Get());

	Engine::mCommandList->SetGraphicsRootConstantBufferView(1,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mEnvConstantBufferIdx)->GetGPUVirtualAddress());

	Engine::mCommandList->SetGraphicsRoot32BitConstant(2, 1, 0);
}

int ShadowMap::GetTextureSrvIdx() const
{
	return mTextureSrvIdx;
}
