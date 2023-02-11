#include "Pipeline.h"
#include "Constant.h"

Pipeline::Pipeline(const int& width, const int& height):
	mWidth(width), mHeight(height)
{

}


void Pipeline::Initialize(HWND windowHandle)
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

	//효율성을 위해 cpu에서 미리 프레임을 계산해 놓기위해서 여러개의 프레임 자원을 생성.
	for (int i = 0; i < mNumberOfFrames; ++i)
	{
		mFrames.push_back(make_unique<Frame>(mDevice.Get()));
	}

	CreateCommandObjects();

	CreateSwapChain(windowHandle);
	
	CreateDescriptorHeaps();

	CreateBackBuffersAndDepthBufferAndViews();

	CreateShaderAndRootSignature();

	mCommandList->Close();

	ID3D12CommandList* lists[] = { mCommandList.Get() };

	mCommandQueue->ExecuteCommandLists(1,lists);
}

ID3D12Device* Pipeline::GetDevice()
{
	return mDevice.Get();
}

ID3D12GraphicsCommandList* Pipeline::GetCommandList()
{
	return mCommandList.Get();
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
	
	IfError::Throw(D3DCompileFromFile(L"Shader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &blob, nullptr),
		L"compile shader error!");

	mShaders["default"] = move(blob);
	
	//shader에 대응되는 root signature 생성.
	ComPtr<ID3D12RootSignature> rs = nullptr;
	
	ComPtr<ID3DBlob> serialized;

	D3D12_ROOT_PARAMETER rootParameter[2];
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[0].Descriptor.RegisterSpace = 0;
	rootParameter[0].Descriptor.ShaderRegister = 0;
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[1].Descriptor.RegisterSpace = 0;
	rootParameter[1].Descriptor.ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = 2;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;
	rsDesc.pParameters = rootParameter;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["default"] = move(rs);
}