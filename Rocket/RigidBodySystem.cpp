#include "RigidBodySystem.h"
#include "Pipeline.h"
#include "d3dx12.h"
#include "IfError.h"

vector<RigidBody*> RigidBodySystem::mRigidBodies;

RigidBodySystem::RigidBodySystem()
{
	/*
	* rigid body system에 필요한 자원을 생성한다.
	*/
	mRigidBodyTexture = make_unique<TextureResource>();
	mParticleTexture = make_unique<TextureResource>();
	mDepthTexture = make_unique<TextureResource>();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mSrvHeap.GetAddressOf())),
		L"create uav descriptor heap in rigid body system error!");


	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
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

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	/*
	* depth buffer에 대한 srv를 생성할때 다음과 같은 format을 사용한다.
	* depth buffer의 format은 DXGI_FORMAT_R24G8_TYPELESS를 사용했다.
	*/
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.ArraySize = 4;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

	Pipeline::mDevice->CreateShaderResourceView(mDepthTexture->mTexture.Get(), &srvDesc, mSrvHeap->GetCPUDescriptorHandleForHeapStart());

	for (auto rigidBody : mRigidBodies)
	{
		
	}
}