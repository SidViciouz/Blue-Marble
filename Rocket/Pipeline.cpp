#include "Pipeline.h"
#include "Constant.h"
#include "d3dx12.h"
#include "Game.h"
#include "DDSTextureLoader.h"

ComPtr<ID3D12Device> Pipeline::mDevice = nullptr;
ComPtr<ID3D12GraphicsCommandList> Pipeline::mCommandList = nullptr;

Pipeline::Pipeline(const int& width, const int& height):
	mWidth(width), mHeight(height)
{

}


void Pipeline::Initialize()
{
	IfError::Throw(CreateDXGIFactory1(IID_PPV_ARGS(mFactory.GetAddressOf())),
		L"create factory error!");

	IfError::Throw(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&mDevice)),
		L"create device error!");

	IfError::Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(mFence.GetAddressOf())),
		L"create fence error!");

	//DirectX12에서는 msaa swapchain을 생성하는 것이 지원되지 않는다. 따라서 Msaa render target을 따로 만들어야함. (추후 구현)
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {};
	qualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevels.Format = mBackBufferFormat;
	qualityLevels.NumQualityLevels = 0;
	qualityLevels.SampleCount = 4;

	IfError::Throw(mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(qualityLevels)),
		L"check feature support error!");

	mMsaaQuality = qualityLevels.NumQualityLevels;

}

void Pipeline::CreateObjects(HWND windowHandle,int numModels)
{
	//효율성을 위해 cpu에서 미리 프레임을 계산해 놓기위해서 여러개의 프레임 자원을 생성.
	for (int i = 0; i < mNumberOfFrames; ++i)
	{
		mFrames.push_back(make_unique<Frame>(numModels));
	}

	CreateCommandObjects();

	CreateSwapChain(windowHandle);
	
	CreateDescriptorHeaps();

	CreateBackBuffersAndDepthBufferAndViews();

	CreateShaderAndRootSignature();
	
	CreatePso();

	SetViewportAndScissor();
}

void Pipeline::CloseAndExecute()
{
	IfError::Throw(mCommandList->Close(),
		L"command list close error!");

	ID3D12CommandList* lists[] = { mCommandList.Get() };

	mCommandQueue->ExecuteCommandLists(1, lists);
}

void Pipeline::Update()
{
	mCurrentFrame = (mCurrentFrame + 1) % mNumberOfFrames;

	auto currentFrame = mFrames[mCurrentFrame].get();

	if (currentFrame->mFenceValue != 0 && mFence->GetCompletedValue() < currentFrame->mFenceValue)
	{
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		IfError::Throw(mFence->SetEventOnCompletion(currentFrame->mFenceValue, event),
			L"set event on completion error!");
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}

void Pipeline::Draw()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mBackBuffers[mCurrentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	IfError::Throw(mFrames[mCurrentFrame]->Get()->Reset(),
		L"frame command allocator reset error!");

	mCommandList->Reset(mFrames[mCurrentFrame]->Get(),mPSOs["default"].Get());
	
	mCommandList->RSSetScissorRects(1, &mScissor);
	mCommandList->RSSetViewports(1, &mViewport);

	mCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * mCurrentBackBuffer;
	float rgba[4] = { 0.0f,0.1f,0.0f,1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle,rgba, 0, nullptr);
	mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);
	
	mCommandList->SetGraphicsRootSignature(mRootSignatures["Default"].Get());

	mCommandList->SetGraphicsRootConstantBufferView(1,mFrames[mCurrentFrame]->mTransConstantBuffer->GetGpuAddress());
}

void Pipeline::SetObjConstantBuffer(int index, const void* data, int byteSize)
{
	mFrames[mCurrentFrame]->mObjConstantBuffer->Copy(index, data, byteSize);
}

void Pipeline::SetTransConstantBuffer(int index, const void* data, int byteSize)
{
	mFrames[mCurrentFrame]->mTransConstantBuffer->Copy(index, data, byteSize);
}

void Pipeline::TransitionToPresent()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mBackBuffers[mCurrentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCommandList->ResourceBarrier(1, &barrier);
}

void Pipeline::DrawFinish()
{
	IfError::Throw(mSwapChain->Present(0, 0),
		L"swap chain present error!");

	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % 2;

	mFrames[mCurrentFrame]->mFenceValue = ++mFenceValue;

	mCommandQueue->Signal(mFence.Get(), mFenceValue);
}

void Pipeline::SetObjConstantIndex(int index)
{
	mCommandList->SetGraphicsRootConstantBufferView(0, mFrames[mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
		+ index * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
}

void Pipeline::CreateSrv(int size)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(mDevice->CreateDescriptorHeap(&heapDesc,IID_PPV_ARGS(mSrvHeap.GetAddressOf())),
		L"create srv heap error!");

	
	auto incrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (auto scene = Game::mScenes.begin(); scene != Game::mScenes.end(); scene++)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Texture2D.MipLevels = -1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		for (auto model = scene->get()->mModels->begin(); model != scene->get()->mModels->end(); model++)
		{
			viewDesc.Format = model->second->mTexture.mResource->GetDesc().Format;
			auto handle = mSrvHeap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += incrementSize * model->second->mObjIndex;
			mDevice->CreateShaderResourceView(model->second->mTexture.mResource.Get(), &viewDesc, handle);
		}

		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.TextureCube.MostDetailedMip = 0;
		viewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		for (auto world = scene->get()->mWorld->begin(); world != scene->get()->mWorld->end(); world++)
		{
			viewDesc.TextureCube.MipLevels = world->second->mTexture.mResource->GetDesc().MipLevels;
			viewDesc.Format = world->second->mTexture.mResource->GetDesc().Format;
			auto handle = mSrvHeap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += incrementSize * world->second->mObjIndex;
			mDevice->CreateShaderResourceView(world->second->mTexture.mResource.Get(), &viewDesc, handle);
		}
	}
}

void Pipeline::SetPSO(string name)
{
	mCommandList->SetPipelineState(mPSOs[name].Get());
}

void Pipeline::SetSrvIndex(int index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = mSrvHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	mCommandList->SetGraphicsRootDescriptorTable(2, handle);
}

ID3D12DescriptorHeap* Pipeline::getSrvHeap()
{
	return mSrvHeap.Get();
}

void Pipeline::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	IfError::Throw(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mCommandQueue.GetAddressOf())),
		L"create command queue error!");

	IfError::Throw(mDevice->CreateCommandList(
		0,D3D12_COMMAND_LIST_TYPE_DIRECT,mFrames[mCurrentFrame]->Get(),
		nullptr,IID_PPV_ARGS(mCommandList.GetAddressOf())),
		L"create command list error!");
}

void Pipeline::CreateBackBuffersAndDepthBufferAndViews()
{
	//create back buffers and views
	for (int i = 0; i < 2; ++i)
		mBackBuffers[i].Reset();

	mSwapChain->ResizeBuffers(2, mWidth, mHeight, mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	mCurrentBackBuffer = 0;
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapCPUHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < 2; ++i)
	{
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(mBackBuffers[i].GetAddressOf()));
		mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHeapCPUHandle);
		rtvHeapCPUHandle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}


	//create depth stencil buffer and view
	mDepthBuffer.Reset();

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES defaultHeapProperty = {};
	defaultHeapProperty.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperty.CreationNodeMask = 0;
	defaultHeapProperty.VisibleNodeMask = 0;


	IfError::Throw(mDevice->CreateCommittedResource(&defaultHeapProperty, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,&clearValue,IID_PPV_ARGS(mDepthBuffer.GetAddressOf())),
		L"create Depth buffer error!");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapCPUHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, dsvHeapCPUHandle);
}

void Pipeline::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	IfError::Throw(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())),
	L"create renter target descriptor heap error!");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	IfError::Throw(mDevice->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
	L"create depth stencil descriptor heap error!");
}
void Pipeline::CreateSwapChain(HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = mWidth;
	swapChainDesc.BufferDesc.Height = mHeight;
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0; // dx12에서는 swapchain생성시에 msaa를 지원하지 않는다.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Windowed = true;
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IfError::Throw(mFactory->CreateSwapChain(mCommandQueue.Get(), &swapChainDesc, mSwapChain.GetAddressOf()),
		L"create swap chain error!");
}

void Pipeline::CreateShaderAndRootSignature()
{
	//shader compile.
	ComPtr<ID3DBlob> blob = nullptr;

	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DefaultVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DefaultPS"] = move(blob);
	
	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "DistortionVS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DistortionVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "SelectedVS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["SelectedVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "SelectedPS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["SelectedPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"WorldShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["WorldVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"WorldShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["WorldPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"VolumeShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumeVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"VolumeShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumePS"] = move(blob);

	//shader에 대응되는 root signature 생성.
	ComPtr<ID3D12RootSignature> rs = nullptr;
	
	ComPtr<ID3DBlob> serialized;

	D3D12_DESCRIPTOR_RANGE range = {};
	range.BaseShaderRegister = 0;
	range.NumDescriptors = 1;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameter[3];
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[0].Descriptor.RegisterSpace = 0;
	rootParameter[0].Descriptor.ShaderRegister = 0;
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[1].Descriptor.RegisterSpace = 0;
	rootParameter[1].Descriptor.ShaderRegister = 1;
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[2].DescriptorTable.pDescriptorRanges = &range;

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc(0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, 
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = 3;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.pParameters = rootParameter;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["Default"] = move(rs);


	rsDesc.NumParameters = 2;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["Volume"] = move(rs);

}

void Pipeline::CreatePso()
{
	ComPtr<ID3D12PipelineState> pso;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	D3D12_INPUT_ELEMENT_DESC inputElements[3];
	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[2] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };

	psoDesc.InputLayout.NumElements = 3;
	psoDesc.InputLayout.pInputElementDescs = inputElements;

	psoDesc.pRootSignature = mRootSignatures["Default"].Get();

	psoDesc.VS.pShaderBytecode = mShaders["DefaultVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["DefaultVS"]->GetBufferSize();

	psoDesc.PS.pShaderBytecode = mShaders["DefaultPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["DefaultPS"]->GetBufferSize();
	
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
	psoDesc.DepthStencilState.BackFace = defaultStencilOp;

	psoDesc.SampleMask = UINT_MAX;
	
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.NumRenderTargets = 1;

	psoDesc.RTVFormats[0] = mBackBufferFormat;

	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	psoDesc.DSVFormat = mDepthStencilBufferFormat;

	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc,IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Default"] = move(pso);

	psoDesc.VS.pShaderBytecode = mShaders["SelectedVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["SelectedVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["SelectedPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["SelectedPS"]->GetBufferSize();
	psoDesc.DepthStencilState.DepthEnable = false;

	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Selected"] = move(pso);

	psoDesc.VS.pShaderBytecode = mShaders["WorldVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["WorldVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["WorldPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["WorldPS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["World"] = move(pso);

	psoDesc.InputLayout.NumElements = 0;
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.pRootSignature = mRootSignatures["Volume"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["VolumeVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["VolumeVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["VolumePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["VolumePS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_ALPHA;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Volume"] = move(pso);
}

void Pipeline::SetViewportAndScissor()
{
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

void Pipeline::SetRootSignature(string name)
{
	mCommandList->SetGraphicsRootSignature(mRootSignatures[name].Get());
}