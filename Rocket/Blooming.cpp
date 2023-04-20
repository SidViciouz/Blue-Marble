#include "Blooming.h"
#include "Engine.h"

Blooming::Blooming(int width, int height)
	: mWidth{ width }, mHeight{height}
{
	for (int i = 0; i < 2; ++i)
	{
		mRenderTargetSrvIdx[i] = Engine::mDescriptorManager->CreateSrv(
			Engine::mResourceManager->GetResource(Engine::mBackBufferOffset + i),
			Engine::mBackBufferFormat,
			D3D12_SRV_DIMENSION_TEXTURE2D);
	}

	mDownScaledTextureIdx = Engine::mResourceManager->CreateTexture2D(mWidth/2, mHeight/2,
		Engine::mBackBufferFormat,D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mDownScaledTextureUavIdx = Engine::mDescriptorManager->CreateUav(Engine::mResourceManager->GetResource(mDownScaledTextureIdx),
		Engine::mBackBufferFormat, D3D12_UAV_DIMENSION_TEXTURE2D);

	mBrightTextureIdx = Engine::mResourceManager->CreateTexture2D(mWidth / 2, mHeight / 2,
		Engine::mBackBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mBrightTextureUavIdx = Engine::mDescriptorManager->CreateUav(Engine::mResourceManager->GetResource(mBrightTextureIdx),
		Engine::mBackBufferFormat, D3D12_UAV_DIMENSION_TEXTURE2D);

	mHorizontalBlurredTextureIdx = Engine::mResourceManager->CreateTexture2D(mWidth / 2, mHeight / 2,
		Engine::mBackBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mHorizontalBlurredTextureUavIdx = Engine::mDescriptorManager->CreateUav(Engine::mResourceManager->GetResource(mHorizontalBlurredTextureIdx),
		Engine::mBackBufferFormat, D3D12_UAV_DIMENSION_TEXTURE2D);

	mBlurredTextureIdx = Engine::mResourceManager->CreateTexture2D(mWidth / 2, mHeight / 2,
		Engine::mBackBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mBlurredTextureUavIdx =Engine::mDescriptorManager->CreateUav(Engine::mResourceManager->GetResource(mBlurredTextureIdx),
		Engine::mBackBufferFormat, D3D12_UAV_DIMENSION_TEXTURE2D);

	mConvolvedTextureIdx = Engine::mResourceManager->CreateTexture2D(mWidth, mHeight,
		Engine::mBackBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mConvolvedTextureUavIdx = Engine::mDescriptorManager->CreateUav(Engine::mResourceManager->GetResource(mConvolvedTextureIdx),
		Engine::mBackBufferFormat, D3D12_UAV_DIMENSION_TEXTURE2D);
}

void Blooming::Resize(int width, int height)
{

}

void Blooming::DownScalePass()
{
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["DownSampling"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["DownSampling"].Get());
	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mRenderTargetSrvIdx[Engine::mCurrentBackBuffer], DescType::SRV));
	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mDownScaledTextureUavIdx,DescType::UAV));
	Engine::mCommandList->Dispatch(4, 30, 1);

}

void Blooming::BrightPass()
{
	D3D12_RESOURCE_BARRIER b = {};
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	b.UAV.pResource = Engine::mResourceManager->GetResource(mDownScaledTextureIdx);

	Engine::mCommandList->ResourceBarrier(1, &b);
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Bright"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["DownSampling"].Get());
	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mDownScaledTextureUavIdx, DescType::UAV));
	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mBrightTextureUavIdx, DescType::UAV));
	Engine::mCommandList->Dispatch(4, 30, 1);
}

void Blooming::BlurPass()
{
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["HorizontalBlur"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["DownSampling"].Get());
	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mBrightTextureUavIdx, DescType::UAV));
	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mHorizontalBlurredTextureUavIdx, DescType::UAV));
	Engine::mCommandList->Dispatch(4, 30, 1);

	// barrier ÇÊ¿ä
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["VerticalBlur"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["DownSampling"].Get());
	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mHorizontalBlurredTextureUavIdx, DescType::UAV));
	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mBlurredTextureUavIdx, DescType::UAV));
	Engine::mCommandList->Dispatch(4, 30, 1);

	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Convolve"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["Convolve"].Get());
	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mRenderTargetSrvIdx[Engine::mCurrentBackBuffer], DescType::SRV));
	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mBlurredTextureUavIdx, DescType::UAV));
	Engine::mCommandList->SetComputeRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(mConvolvedTextureUavIdx, DescType::UAV));
	Engine::mCommandList->Dispatch(8, 60, 1);

	
	D3D12_RESOURCE_BARRIER b[2];
	b[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	b[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	b[0].UAV.pResource = Engine::mResourceManager->GetResource(mConvolvedTextureIdx);
	b[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(Engine::mBackBufferOffset + Engine::mCurrentBackBuffer),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COPY_DEST
	);

	Engine::mCommandList->ResourceBarrier(2, b);

	Engine::mCommandList->CopyResource(
		Engine::mResourceManager->GetResource(Engine::mBackBufferOffset + Engine::mCurrentBackBuffer),
		Engine::mResourceManager->GetResource(mConvolvedTextureIdx));

	b[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(Engine::mBackBufferOffset + Engine::mCurrentBackBuffer),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	Engine::mCommandList->ResourceBarrier(1, &b[1]);

}