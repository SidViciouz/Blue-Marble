#include "PerlinMap.h"
#include "Engine.h"

PerlinMap::PerlinMap()
{
	mGradientsIdx = Engine::mResourceManager->CreateTexture1D(128, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPermutationIdx = Engine::mResourceManager->CreateTexture1D(256, DXGI_FORMAT_R32_SINT);

	mGradientsUploadIdx = Engine::mResourceManager->CreateUploadBuffer(constantBufferAlignment(16 * 128));
	mPermutationUploadIdx = Engine::mResourceManager->CreateUploadBuffer(constantBufferAlignment(4 * 256));

	float gradients[4 * 128] = { 0, };
	int permutation[256] = { 0, };

	const double pi = 3.14159265358979;

	unsigned int tableMask = 127;

	mt19937 generator(2020);
	uniform_real_distribution<float> dist;

	for (int i = 0; i < 128; ++i)
	{
		float theta = acos(2.0f * dist(generator) - 1.0f);
		float phi = 2.0f * dist(generator) * pi;

		float x = cos(phi) * sin(theta);
		float y = sin(phi) * sin(theta);
		float z = cos(theta);

		gradients[4 * i] = x;
		gradients[4 * i + 1] = y;
		gradients[4 * i + 2] = z;

		permutation[i] = i;
	}

	uniform_int_distribution<unsigned int> intDist;
	for (int i = 0; i < 128; ++i)
	{
		swap(permutation[i], permutation[intDist(generator) & tableMask]);
	}
	for (int i = 0; i < 128; ++i)
	{
		permutation[128 + i] = permutation[i];
	}

	Engine::mResourceManager->Upload(mGradientsUploadIdx, gradients, 16 * 128, 0);
	Engine::mResourceManager->Upload(mPermutationUploadIdx, permutation, 4 * 256, 0);

	Engine::mResourceManager->CopyUploadToTexture(mGradientsUploadIdx, mGradientsIdx, 128, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 16);
	Engine::mResourceManager->CopyUploadToTexture(mPermutationUploadIdx, mPermutationIdx, 256, 1, 1, DXGI_FORMAT_R32_SINT, 4);

	D3D12_RESOURCE_BARRIER b[2];
	b[0] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mGradientsIdx), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ);
	b[1] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mPermutationIdx), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ);
	Engine::mCommandList->ResourceBarrier(2, b);

	mGradientsDescriptorIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mGradientsIdx), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_SRV_DIMENSION_TEXTURE1D);
	mPermutationDescriptorIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mPermutationIdx), DXGI_FORMAT_R32_SINT, D3D12_SRV_DIMENSION_TEXTURE1D);
}