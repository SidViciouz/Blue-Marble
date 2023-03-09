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
	mViewport = { 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 1.0f };
	mScissor = { 0, 0, 20, 20 };

	mRigidBodyTexture = make_unique<TextureResource>();
	mParticleTexture = make_unique<TextureResource>();
	mDepthTexture = make_unique<TextureResource>();
	mPrevDepthTexture = make_unique<TextureResource>();


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mSrvHeap.GetAddressOf())),
		L"create uav descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mInvisibleSrvHeap.GetAddressOf())),
		L"create uav shader-invisible descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
		L"create dsv descriptor heap in rigid body system error!");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mCbvHeap.GetAddressOf())),
		L"create dsv descriptor heap in rigid body system error!");


}

void RigidBodySystem::Load()
{
	//여기에서 존재 mRigidBodies에 저장된 rigid body를 모두 texture로 업로드한다. 
	//새롭게 생성되는 rigidbody에는 따로 처리가 필요하다.

	//TextureResource에서 float을 지원하도록 수정해야함.
	float data[256*5*2] = { 0, };

	for (int i = 0; i < mRigidBodies.size(); ++i)
	{
		int x = 3 * i;
		int y = 3 * i + 1;
		int z = 3 * i + 2;

		XMFLOAT3 position = mRigidBodies[i]->mModel->GetPosition();
		data[x] = position.x;
		data[y] = position.y;
		data[z] = position.z;
	}

	for (int i = 0; i < 3 * mRigidBodies.size(); ++i)
		printf("%8X\n", reinterpret_cast<UINT32&>(data[i]));

	mRigidBodyTexture->Copy(data, 256, 256, 2, 4);
}

void RigidBodySystem::GenerateParticle()
{
	int offset = 0;

	mDepthTexture->CreateDepth(20, 20, 4, 4);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2DArray.ArraySize = 4;
	dsvDesc.Texture2DArray.FirstArraySlice = 0;
	dsvDesc.Texture2DArray.MipSlice = 0;
	Pipeline::mDevice->CreateDepthStencilView(mDepthTexture->mTexture.Get(),&dsvDesc,mDsvHeap->GetCPUDescriptorHandleForHeapStart());

	/*
	* depth buffer에 대한 srv를 생성할때 다음과 같은 format을 사용한다.
	* depth buffer의 format은 DXGI_FORMAT_R24G8_TYPELESS를 사용했다.
	*/

	mPrevDepthTexture->Create(20, 20, 4, 4,true);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 4;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.PlaneSlice = 0;
	Pipeline::mDevice->CreateUnorderedAccessView(mPrevDepthTexture->mTexture.Get(), nullptr,&uavDesc, mSrvHeap->GetCPUDescriptorHandleForHeapStart());
	Pipeline::mDevice->CreateUnorderedAccessView(mPrevDepthTexture->mTexture.Get(), nullptr, &uavDesc, mInvisibleSrvHeap->GetCPUDescriptorHandleForHeapStart());

	mCbvBuffer = make_unique<UploadBuffer>(Pipeline::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(XMFLOAT4X4)));
	XMMATRIX proj;
	XMMATRIX cam;
	XMMATRIX m;
	XMFLOAT4X4 mat;

	cam =  XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -5.0f,1.0f), XMVectorSet(0.0f, 0.0f, 0.0f,1.0f), XMVectorSet(0.0f, 1.0f, 0.0f,1.0f));
	proj = XMMatrixOrthographicLH(800.0f, 600.0f, 1.0f, 1000.0f);
	m = XMMatrixMultiply(cam,proj);

	XMStoreFloat4x4(&mat, m);
	
	mCbvBuffer->Copy(0, &mat, BufferInterface::ConstantBufferByteSize(sizeof(XMFLOAT4X4)));
	/*
	for (auto rigidBody : mRigidBodies)
	{
		if (rigidBody->mModel->mVertexBufferSize == 0)
			return;
		DepthPass(rigidBody);
	}
	*/
	DepthPass(mRigidBodies[8]);
}

void RigidBodySystem::DepthPass(RigidBody* rigidBody)
{

	Game::mCommandList->RSSetScissorRects(1, &mScissor);
	Game::mCommandList->RSSetViewports(1, &mViewport);

	Game::mCommandList->SetPipelineState(Pipeline::mPSOs["DepthPeeling"].Get());

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	Game::mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	Game::mCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);

	Game::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Game::mCommandList->SetGraphicsRootSignature(Pipeline::mRootSignatures["DepthPeeling"].Get());


	ID3D12DescriptorHeap* heaps[] = { mSrvHeap.Get() };
	Game::mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvHeap->GetGPUDescriptorHandleForHeapStart();
	Game::mCommandList->SetGraphicsRootDescriptorTable(0, handle);

	D3D12_CPU_DESCRIPTOR_HANDLE Invisiblehandle = mInvisibleSrvHeap->GetCPUDescriptorHandleForHeapStart();

	FLOAT color[4] = { 1.0f,1.0f,1.0f,1.0f };
	Game::mCommandList->ClearUnorderedAccessViewFloat(handle, Invisiblehandle, mPrevDepthTexture->mTexture.Get(), color, 0, nullptr);

	Game::mCommandList->SetGraphicsRootConstantBufferView(2, mCbvBuffer->GetGpuAddress());

	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 0, 0);
	rigidBody->mModel->Draw();

	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 1, 0);
	rigidBody->mModel->Draw();

	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 2, 0);
	rigidBody->mModel->Draw();

	Game::mCommandList->SetGraphicsRoot32BitConstant(1, 3, 0);
	rigidBody->mModel->Draw();
}