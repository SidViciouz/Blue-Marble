#include "RigidBodySystem.h"
#include "Engine.h"
#include "d3dx12.h"
#include "IfError.h"
#include "Engine.h"

vector<RigidBody*> RigidBodySystem::mRigidBodies;

RigidBodySystem::RigidBodySystem()
{
	/*
	* rigid body system에 필요한 자원을 생성한다.
	*/
	mViewport = { 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 1.0f };
	mScissor = { 0, 0, 10, 10 };

	mDepthTexture = make_unique<TextureResource>();
	mParticleCOMTexture = make_unique<TextureResource>();
	mParticlePosTexture = make_unique<TextureResource>();
	mParticleVelTexture = make_unique<TextureResource>();
	mRigidBodyPosTexture = make_unique<TextureResource>();
	mRigidBodyQuatTexture = make_unique<TextureResource>();
	mRigidBodyLMTexture = make_unique<TextureResource>();
	mRigidBodyAMTexture = make_unique<TextureResource>();
	mRigidInfos = make_unique<TextureResource>();
	mGrid = make_unique<TextureResource>();
	mRigidInertia = make_unique<TextureResource>();
	mParticleForce = make_unique<TextureResource>();
	mRigidBodyPosTexture2 = make_unique<TextureResource>();
	mRigidBodyQuatTexture2 = make_unique<TextureResource>();
	mRigidBodyLMTexture2 = make_unique<TextureResource>();
	mRigidBodyAMTexture2 = make_unique<TextureResource>();
	

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 4;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
		L"create dsv descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 16;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mSrvUavHeap.GetAddressOf())),
		L"create srv descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mNonVisibleSrvUavHeap.GetAddressOf())),
		L"create srv descriptor heap in rigid body system error!");


	mDsvIncrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mSrvUavIncrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void RigidBodySystem::Load()
{
	//여기에서 존재 mRigidBodies에 저장된 rigid body를 모두 texture로 업로드한다. 
	//새롭게 생성되는 rigidbody에는 따로 처리가 필요하다.

	//TextureResource에서 float을 지원하도록 수정해야함.
	float pos[1000] = { 0, };
	float quat[1000] = { 0, };
	float lm[1000] = { 0, };
	float am[1000] = { 0, };

	for (int i = 0; i < mRigidBodies.size(); ++i)
	{
		int offset = i * 4;

		int x = offset;
		int y = offset + 1;
		int z = offset + 2;
		int w = offset + 3;

		XMFLOAT3 position = mRigidBodies[i]->mModel->GetPosition();
		XMFLOAT4 quaternion = mRigidBodies[i]->mModel->GetQuaternion();
		XMFLOAT3 linearMomentum = mRigidBodies[i]->GetLinearMomentum();
		XMFLOAT3 angularMomentum = mRigidBodies[i]->GetAngularMomentum();

		pos[x] = position.x;
		pos[y] = position.y;
		pos[z] = position.z;
		quat[x] = quaternion.x;
		quat[y] = quaternion.y;
		quat[z] = quaternion.z;
		quat[w] = quaternion.w;
		lm[x] = linearMomentum.x;
		lm[y] = linearMomentum.y;
		lm[z] = linearMomentum.z;
		am[x] = angularMomentum.x;
		am[y] = angularMomentum.y;
		am[z] = angularMomentum.z;
	}

	mRigidBodyPosTexture->CopyCreate(pos, 128,128,2,16,false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyQuatTexture->CopyCreate(quat, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyLMTexture->CopyCreate(lm, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyAMTexture->CopyCreate(am, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mParticleCOMTexture->Create(128, 128, 2, 8, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
	mParticlePosTexture->Create(128, 128, 2, 8, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
	mParticleVelTexture->Create(128, 128, 2, 8, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
	mDepthTexture->CreateDepth(10, 10, 4, 4);
	mRigidInfos->Create(128, 3, 1, 4, true, DXGI_FORMAT_R32_UINT); //2d이기 때문에 isarray를 true로 설정한다.
	/*
	* x*y*z = 32*32*32에 하나당 64비트이므로 16비트씩 총 4개까지의 particle을 저장할 수 있다.
	*/
	mGrid->Create(32, 32, 32, 8, false, DXGI_FORMAT_R16G16B16A16_SINT);
	mRigidInertia->Create(128, 32, 1, 8, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
	mParticleForce->Create(128, 32, 1, 8, true, DXGI_FORMAT_R16G16B16A16_FLOAT);
	
	mRigidBodyPosTexture2->CopyCreate(pos, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyQuatTexture2->CopyCreate(quat, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyLMTexture2->CopyCreate(lm, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyAMTexture2->CopyCreate(am, 128, 128, 2, 16, false, DXGI_FORMAT_R32G32B32A32_FLOAT);
	
}

void RigidBodySystem::GenerateParticle()
{
	int offset = 0;


	for (int i = 0; i < 4; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += mDsvIncrementSize *i;
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Texture2DArray.ArraySize = 4-i;
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		dsvDesc.Texture2DArray.MipSlice = 0;
		Engine::mDevice->CreateDepthStencilView(mDepthTexture->mTexture.Get(), &dsvDesc,handle);
	}

	/*
	* depth buffer에 대한 srv를 생성할때 DXGI_FORMAT_R24_UNORM_X8_TYPELESS format을 사용한다.
	* depth buffer의 format은 DXGI_FORMAT_R24G8_TYPELESS를 사용했다.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.ArraySize = 4;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	Engine::mDevice->CreateShaderResourceView(mDepthTexture->mTexture.Get(), &srvDesc, srvHandle);


	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavComDesc = {};
	uavComDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavComDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavComDesc.Texture2DArray.ArraySize = 2;
	uavComDesc.Texture2DArray.FirstArraySlice = 0;
	uavComDesc.Texture2DArray.MipSlice = 0;
	uavComDesc.Texture2DArray.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mParticleCOMTexture->mTexture.Get(), nullptr, &uavComDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavPosDesc = {};
	uavPosDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavPosDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavPosDesc.Texture2DArray.ArraySize = 2;
	uavPosDesc.Texture2DArray.FirstArraySlice = 0;
	uavPosDesc.Texture2DArray.MipSlice = 0;
	uavPosDesc.Texture2DArray.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mParticlePosTexture->mTexture.Get(), nullptr, &uavPosDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavVelDesc = {};
	uavVelDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavVelDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavVelDesc.Texture2DArray.ArraySize = 2;
	uavVelDesc.Texture2DArray.FirstArraySlice = 0;
	uavVelDesc.Texture2DArray.MipSlice = 0;
	uavVelDesc.Texture2DArray.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mParticleVelTexture->mTexture.Get(), nullptr, &uavVelDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyPosUavDesc = {};
	rigidBodyPosUavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyPosUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyPosUavDesc.Texture3D.FirstWSlice = 0;
	rigidBodyPosUavDesc.Texture3D.MipSlice = 0;
	rigidBodyPosUavDesc.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyPosTexture->mTexture.Get(), nullptr, &rigidBodyPosUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyQuatUavDesc = {};
	rigidBodyQuatUavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyQuatUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyQuatUavDesc.Texture3D.FirstWSlice = 0;
	rigidBodyQuatUavDesc.Texture3D.MipSlice = 0;
	rigidBodyQuatUavDesc.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyQuatTexture->mTexture.Get(), nullptr, &rigidBodyQuatUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyLMUavDesc = {};
	rigidBodyLMUavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyLMUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyLMUavDesc.Texture3D.FirstWSlice = 0;
	rigidBodyLMUavDesc.Texture3D.MipSlice = 0;
	rigidBodyLMUavDesc.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyLMTexture->mTexture.Get(), nullptr, &rigidBodyLMUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyAMUavDesc = {};
	rigidBodyAMUavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyAMUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyAMUavDesc.Texture3D.FirstWSlice = 0;
	rigidBodyAMUavDesc.Texture3D.MipSlice = 0;
	rigidBodyAMUavDesc.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyAMTexture->mTexture.Get(), nullptr, &rigidBodyAMUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidInfosUavDesc = {};
	rigidInfosUavDesc.Format = DXGI_FORMAT_R32_UINT;
	rigidInfosUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	rigidInfosUavDesc.Texture2D.MipSlice = 0;
	rigidInfosUavDesc.Texture2D.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mRigidInfos->mTexture.Get(), nullptr, &rigidInfosUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC gridUavDesc = {};
	gridUavDesc.Format = DXGI_FORMAT_R16G16B16A16_SINT;
	gridUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	gridUavDesc.Texture3D.FirstWSlice = 0;
	gridUavDesc.Texture3D.MipSlice = 0;
	gridUavDesc.Texture3D.WSize = 32;
	Engine::mDevice->CreateUnorderedAccessView(mGrid->mTexture.Get(), nullptr, &gridUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidInertiaUavDesc = {};
	rigidInertiaUavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rigidInertiaUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	rigidInertiaUavDesc.Texture2D.MipSlice = 0;
	rigidInertiaUavDesc.Texture2D.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mRigidInertia->mTexture.Get(), nullptr, &rigidInertiaUavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC particleForceUavDesc = {};
	particleForceUavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	particleForceUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	particleForceUavDesc.Texture2D.MipSlice = 0;
	particleForceUavDesc.Texture2D.PlaneSlice = 0;
	Engine::mDevice->CreateUnorderedAccessView(mParticleForce->mTexture.Get(), nullptr, &particleForceUavDesc, srvHandle);
	
	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyPosUavDesc2 = {};
	rigidBodyPosUavDesc2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyPosUavDesc2.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyPosUavDesc2.Texture3D.FirstWSlice = 0;
	rigidBodyPosUavDesc2.Texture3D.MipSlice = 0;
	rigidBodyPosUavDesc2.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyPosTexture2->mTexture.Get(), nullptr, &rigidBodyPosUavDesc2, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyQuatUavDesc2 = {};
	rigidBodyQuatUavDesc2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyQuatUavDesc2.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyQuatUavDesc2.Texture3D.FirstWSlice = 0;
	rigidBodyQuatUavDesc2.Texture3D.MipSlice = 0;
	rigidBodyQuatUavDesc2.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyQuatTexture2->mTexture.Get(), nullptr, &rigidBodyQuatUavDesc2, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyLMUavDesc2 = {};
	rigidBodyLMUavDesc2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyLMUavDesc2.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyLMUavDesc2.Texture3D.FirstWSlice = 0;
	rigidBodyLMUavDesc2.Texture3D.MipSlice = 0;
	rigidBodyLMUavDesc2.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyLMTexture2->mTexture.Get(), nullptr, &rigidBodyLMUavDesc2, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyAMUavDesc2 = {};
	rigidBodyAMUavDesc2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rigidBodyAMUavDesc2.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyAMUavDesc2.Texture3D.FirstWSlice = 0;
	rigidBodyAMUavDesc2.Texture3D.MipSlice = 0;
	rigidBodyAMUavDesc2.Texture3D.WSize = 2;
	Engine::mDevice->CreateUnorderedAccessView(mRigidBodyAMTexture2->mTexture.Get(), nullptr, &rigidBodyAMUavDesc2, srvHandle);

	//non-shader visible desciptor for mGrid
	D3D12_UNORDERED_ACCESS_VIEW_DESC nonGridDesc = {};
	nonGridDesc.Format = DXGI_FORMAT_R16G16B16A16_SINT;
	nonGridDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	nonGridDesc.Texture3D.FirstWSlice = 0;
	nonGridDesc.Texture3D.MipSlice = 0;
	nonGridDesc.Texture3D.WSize = 32;
	Engine::mDevice->CreateUnorderedAccessView(mGrid->mTexture.Get(), nullptr, &nonGridDesc, mNonVisibleSrvUavHeap->GetCPUDescriptorHandleForHeapStart());

	int i = -1;
	for (auto rigidBody : mRigidBodies)
	{
		++i;
		DepthPass(rigidBody);
		UploadParticleFromDepth(i);
	}
	

	//DepthPass(mRigidBodies[0]);
	//UploadParticleFromDepth(0);
	/*
	CalculateRigidInertia(mRigidBodies.size());
	CalculateParticlePosition(mRigidBodies.size());
	CalculateParticleVelocity(mRigidBodies.size());
	PutParticleOnGrid(mRigidBodies.size());
	ParticleCollision(mRigidBodies.size());
	NextRigidMomentum(0.1f);
	NextRigidPosQuat(mRigidBodies.size(), 0.1f);
	*/
}

void RigidBodySystem::DepthPass(RigidBody* rigidBody)
{

	Engine::mCommandList->RSSetScissorRects(1, &mScissor);
	Engine::mCommandList->RSSetViewports(1, &mViewport);
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["DepthPeeling"].Get());
	Engine::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["DepthPeeling"].Get());

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	ID3D12DescriptorHeap* heaps[] = { mSrvUavHeap.Get() };
	Engine::mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	Engine::mCommandList->SetGraphicsRootDescriptorTable(0, handle);

	Engine::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Engine::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Engine::mCommandList->SetGraphicsRoot32BitConstant(1, 0, 0);
	rigidBody->mModel->Draw();
	
	dsvHandle.ptr += mDsvIncrementSize;
	Engine::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Engine::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Engine::mCommandList->SetGraphicsRoot32BitConstant(1, 1, 0);
	rigidBody->mModel->Draw();

	dsvHandle.ptr += mDsvIncrementSize;
	Engine::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Engine::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Engine::mCommandList->SetGraphicsRoot32BitConstant(1, 2, 0);
	rigidBody->mModel->Draw();

	dsvHandle.ptr += mDsvIncrementSize;
	Engine::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Engine::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Engine::mCommandList->SetGraphicsRoot32BitConstant(1, 3, 0);
	rigidBody->mModel->Draw();
}

void RigidBodySystem::UploadParticleFromDepth(int index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[2];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInertia->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["CreateParticles"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1,mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, index, 0);
	/*
	* 이전 오브젝트가 offset과 particle 개수를 모두 계산해야
	* 다음 오브젝트가 그 결과를 이용해서 자신의 offset을 계산한다.
	* 따라서 barrier가 필요하다.
	*/
	Engine::mCommandList->ResourceBarrier(2,barrier);
	Engine::mCommandList->Dispatch(1,1,1);
}

void RigidBodySystem::UploadRigidBody()
{

	float pos[1000] = { 0, };
	float quat[1000] = { 0, };
	float lm[1000] = { 0, };
	float am[1000] = { 0, };

	for (int i = 0; i < mRigidBodies.size(); ++i)
	{
		int offset = i * 4;

		int x = offset;
		int y = offset + 1;
		int z = offset + 2;
		int w = offset + 3;

		XMFLOAT3 position = mRigidBodies[i]->mModel->GetPosition();
		XMFLOAT4 quaternion = mRigidBodies[i]->mModel->GetQuaternion();
		XMFLOAT3 linearMomentum = mRigidBodies[i]->GetLinearMomentum();
		XMFLOAT3 angularMomentum = mRigidBodies[i]->GetAngularMomentum();

		pos[x] = position.x;
		pos[y] = position.y;
		pos[z] = position.z;
		quat[x] = quaternion.x;
		quat[y] = quaternion.y;
		quat[z] = quaternion.z;
		quat[w] = quaternion.w;
		lm[x] = linearMomentum.x;
		lm[y] = linearMomentum.y;
		lm[z] = linearMomentum.z;
		am[x] = angularMomentum.x;
		am[y] = angularMomentum.y;
		am[z] = angularMomentum.z;
	}

	mRigidBodyPosTexture->Copy(pos, 128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyQuatTexture->Copy(quat, 128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyLMTexture->Copy(lm, 128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyAMTexture->Copy(am, 128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void RigidBodySystem::CalculateRigidInertia(int objNum)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[3];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleCOMTexture->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInertia->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["RigidInertia"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	Engine::mCommandList->ResourceBarrier(3, barrier);
	Engine::mCommandList->Dispatch(1, 1, 1);
}

void RigidBodySystem::CalculateParticlePosition(int objNum)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[2];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleCOMTexture->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["particlePosition"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	/*
	* particle COM과, offset 계산을 모두 마친 후에 그 결과를 이용해야한다.
	*/
	Engine::mCommandList->ResourceBarrier(2, barrier);
	Engine::mCommandList->Dispatch(1, 1, 1);
}

void RigidBodySystem::CalculateParticleVelocity(int objNum)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[3];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleCOMTexture->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInertia->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["particleVelocity"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	/*
	* particle COM과, offset 계산을 모두 마친 후에 그 결과를 이용해야한다.
	*/
	Engine::mCommandList->ResourceBarrier(3, barrier);
	Engine::mCommandList->Dispatch(1, 1, 1);
}

void RigidBodySystem::PutParticleOnGrid(int objNum)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuViewHandle = handle;
	gpuViewHandle.ptr += mSrvUavIncrementSize*9;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuViewHandle = mNonVisibleSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	UINT clearColor[4] = {-1,-1,-1,-1 };

	D3D12_RESOURCE_BARRIER barrier[4];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleCOMTexture->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mParticlePosTexture->mTexture.Get());
	barrier[3] = CD3DX12_RESOURCE_BARRIER::UAV(mGrid->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["GridShader"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->ClearUnorderedAccessViewUint(gpuViewHandle, cpuViewHandle, mGrid->mTexture.Get(), clearColor, 0, nullptr);
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	Engine::mCommandList->ResourceBarrier(4, barrier);
	Engine::mCommandList->Dispatch(1, 1, 1);
}

void RigidBodySystem::ParticleCollision(int objNum)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[4];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleVelTexture->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mParticlePosTexture->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mGrid->mTexture.Get());
	barrier[3] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Collision"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	Engine::mCommandList->ResourceBarrier(4, barrier);
	Engine::mCommandList->Dispatch(32, 1, 1);
}

void RigidBodySystem::NextRigidMomentum(float deltaTime)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[7];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mParticlePosTexture->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyLMTexture->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyAMTexture->mTexture.Get());
	barrier[3] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[4] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleForce->mTexture.Get());
	barrier[5] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyLMTexture2->mTexture.Get());
	barrier[6] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyAMTexture2->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["RigidMomentum"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1,reinterpret_cast<UINT&>(deltaTime), 0);
	Engine::mCommandList->ResourceBarrier(7, barrier);
	Engine::mCommandList->Dispatch(mRigidBodies.size(), 1, 1);
}

void RigidBodySystem::NextRigidPosQuat(int objNum, float deltaTime)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_RESOURCE_BARRIER barrier[10];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::UAV(mParticlePosTexture->mTexture.Get());
	barrier[1] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyLMTexture2->mTexture.Get());
	barrier[2] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyAMTexture2->mTexture.Get());
	barrier[3] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInfos->mTexture.Get());
	barrier[4] = CD3DX12_RESOURCE_BARRIER::UAV(mParticleForce->mTexture.Get());
	barrier[5] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidInertia->mTexture.Get());
	barrier[6] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyPosTexture->mTexture.Get());
	barrier[7] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyQuatTexture->mTexture.Get());
	barrier[8] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyPosTexture2->mTexture.Get());
	barrier[9] = CD3DX12_RESOURCE_BARRIER::UAV(mRigidBodyQuatTexture2->mTexture.Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["RigidPosQuat"].Get());
	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["CreateParticles"].Get());
	Engine::mCommandList->SetDescriptorHeaps(1, mSrvUavHeap.GetAddressOf());
	Engine::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, objNum, 0);
	Engine::mCommandList->SetComputeRoot32BitConstant(1, reinterpret_cast<UINT&>(deltaTime), 1);
	Engine::mCommandList->ResourceBarrier(10, barrier);
	Engine::mCommandList->Dispatch(1, 1, 1);
}

void RigidBodySystem::UpdateRigidBody()
{
	float pos[1000] = { 0, };
	float quat[1000] = { 0, };
	float lm[1000] = { 0, };
	float am[1000] = { 0, };

	mRigidBodyPosTexture2->ReadbackBufferCopy(pos);
	mRigidBodyQuatTexture2->ReadbackBufferCopy(quat);
	mRigidBodyLMTexture2->ReadbackBufferCopy(lm);
	mRigidBodyAMTexture2->ReadbackBufferCopy(am);

	for (int i = 0; i < mRigidBodies.size(); ++i)
	{
		int offset = i * 4;

		int x = offset;
		int y = offset + 1;
		int z = offset + 2;
		int w = offset + 3;

		
		mRigidBodies[i]->mModel->SetPosition(pos[x], pos[y], pos[z]);
		mRigidBodies[i]->mModel->SetQuaternion(quat[x], quat[y], quat[z], quat[w]);
		mRigidBodies[i]->SetLinearMomentum(lm[x],lm[y],lm[z]);
		mRigidBodies[i]->SetAngularMomentum(am[x], am[y], am[z]);
	}
}

void RigidBodySystem::CopyRigidBody()
{

	mRigidBodyPosTexture2->Readback(128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyQuatTexture2->Readback(128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyLMTexture2->Readback(128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRigidBodyAMTexture2->Readback(128, 128, 2, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);
}