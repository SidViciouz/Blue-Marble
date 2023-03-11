#include "RigidBodySystem.h"
#include "Pipeline.h"
#include "d3dx12.h"
#include "IfError.h"
#include "Game.h"

vector<RigidBody*> RigidBodySystem::mRigidBodies;

RigidBodySystem::RigidBodySystem()
{
	/*
	* rigid body system에 필요한 자원을 생성한다.
	*/
	mViewport = { 0.0f, 0.0f, 5.0f, 5.0f, 0.0f, 1.0f };
	mScissor = { 0, 0, 5, 5 };

	mRigidBodyTexture = make_unique<TextureResource>();
	mParticleTexture = make_unique<TextureResource>();
	mDepthTexture = make_unique<TextureResource>();


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 4;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
		L"create dsv descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 3;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mSrvUavHeap.GetAddressOf())),
		L"create srv descriptor heap in rigid body system error!");


	mDsvIncrementSize = Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mSrvUavIncrementSize = Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void RigidBodySystem::Load()
{
	//여기에서 존재 mRigidBodies에 저장된 rigid body를 모두 texture로 업로드한다. 
	//새롭게 생성되는 rigidbody에는 따로 처리가 필요하다.

	//TextureResource에서 float을 지원하도록 수정해야함.
	float data[256*5*2] = { 0, };

	for (int i = 0; i < mRigidBodies.size(); ++i)
	{
		int x = 5 * i;
		int y = 5 * i + 1;
		int z = 5 * i + 2;
		int particleNumber = 5 * i + 3;
		int offset = 5 * i + 4;

		XMFLOAT3 position = mRigidBodies[i]->mModel->GetPosition();
		data[x] = position.x;
		data[y] = position.y;
		data[z] = position.z;
		data[particleNumber] = 0;
		data[offset] = 0;
	}

	for (int i = 0; i < 3 * mRigidBodies.size(); ++i)
		printf("%8X\n", reinterpret_cast<UINT32&>(data[i]));

	mRigidBodyTexture->Copy(data, 256, 256, 2, 4);
	mParticleTexture->Create(100, 100, 2, 4, true);
}

void RigidBodySystem::GenerateParticle()
{
	int offset = 0;

	mDepthTexture->CreateDepth(5, 5, 4, 4);

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
		Pipeline::mDevice->CreateDepthStencilView(mDepthTexture->mTexture.Get(), &dsvDesc,handle);
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
	Pipeline::mDevice->CreateShaderResourceView(mDepthTexture->mTexture.Get(), &srvDesc, srvHandle);


	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 2;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.PlaneSlice = 0;
	Pipeline::mDevice->CreateUnorderedAccessView(mParticleTexture->mTexture.Get(), nullptr, &uavDesc, srvHandle);

	srvHandle.ptr += mSrvUavIncrementSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC rigidBodyUavDesc = {};
	rigidBodyUavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rigidBodyUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	rigidBodyUavDesc.Texture3D.FirstWSlice = 0;
	rigidBodyUavDesc.Texture3D.MipSlice = 0;
	rigidBodyUavDesc.Texture3D.WSize = 2;
	Pipeline::mDevice->CreateUnorderedAccessView(mRigidBodyTexture->mTexture.Get(), nullptr, &rigidBodyUavDesc, srvHandle);

	
	int i = -1;
	for (auto rigidBody : mRigidBodies)
	{
		++i;
		if (rigidBody->mModel->mVertexBufferSize == 0)
			continue;
		DepthPass(rigidBody);
		UploadParticleFromDepth(i);
	}
	
	/*
	DepthPass(mRigidBodies[8]);
	UploadParticleFromDepth(8);
	*/
}

void RigidBodySystem::DepthPass(RigidBody* rigidBody)
{

	Game::mCommandList->RSSetScissorRects(1, &mScissor);
	Game::mCommandList->RSSetViewports(1, &mViewport);
	Game::mCommandList->SetPipelineState(Pipeline::mPSOs["DepthPeeling"].Get());
	Game::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Game::mCommandList->SetGraphicsRootSignature(Pipeline::mRootSignatures["DepthPeeling"].Get());

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	ID3D12DescriptorHeap* heaps[] = { mSrvUavHeap.Get() };
	Game::mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	Game::mCommandList->SetGraphicsRootDescriptorTable(0, handle);


	Game::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Game::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 0, 0);
	rigidBody->mModel->Draw();
	
	dsvHandle.ptr += mDsvIncrementSize;
	Game::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Game::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 1, 0);
	rigidBody->mModel->Draw();

	dsvHandle.ptr += mDsvIncrementSize;
	Game::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Game::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 2, 0);
	rigidBody->mModel->Draw();

	dsvHandle.ptr += mDsvIncrementSize;
	Game::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Game::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 3, 0);
	rigidBody->mModel->Draw();
}

void RigidBodySystem::UploadParticleFromDepth(int index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	Game::mCommandList->SetPipelineState(Pipeline::mPSOs["ParticleUpload"].Get());
	Game::mCommandList->SetComputeRootSignature(Pipeline::mRootSignatures["ParticleUpload"].Get());
	Game::mCommandList->SetDescriptorHeaps(1,mSrvUavHeap.GetAddressOf());
	Game::mCommandList->SetComputeRootDescriptorTable(0, handle);
	Game::mCommandList->SetComputeRoot32BitConstant(1, index, 0);
	Game::mCommandList->Dispatch(1,1,1);
}