#include "Pipeline.h"
#include "Constant.h"
#include "d3dx12.h"
#include "Game.h"
#include "DDSTextureLoader.h"

ComPtr<ID3D12Device> Pipeline::mDevice = nullptr;
PSOs Pipeline::mPSOs;
RootSigs Pipeline::mRootSignatures;

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

void Pipeline::CreateObjects(HWND windowHandle)
{
	CreateSwapChain(windowHandle);
	
	CreateDescriptorHeaps();

	CreateBackBuffersAndDepthBufferAndViews();

	CreateShaderAndRootSignature();
	
	CreatePso();

	SetViewportAndScissor();
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

	IfError::Throw(D3DCompileFromFile(L"VolumeSphereShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumeSphereVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"VolumeSphereShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumeSpherePS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"VolumeCubeShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumeCubeVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"VolumeCubeShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VolumeCubePS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ParticleShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ParticleVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"RigidParticleShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["RigidParticleVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"RigidParticleShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["RigidParticlePS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"DepthPeelingShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DepthPeelingVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"DepthPeelingShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DepthPeelingPS"] = move(blob);

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


	rsDesc.NumParameters = 3;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["Volume"] = move(rs);


	rsDesc.NumParameters = 2;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["RigidParticle"] = move(rs);

	range.BaseShaderRegister = 0;
	range.NumDescriptors = 1;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range.RegisterSpace = 0;

	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[0].DescriptorTable.pDescriptorRanges = &range;

	rsDesc.NumParameters = 1;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["Particle"] = move(rs);

	range.BaseShaderRegister = 0;
	range.NumDescriptors = 1;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.RegisterSpace = 0;

	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[0].DescriptorTable.pDescriptorRanges = &range;
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[1].Constants.Num32BitValues = 1;
	rootParameter[1].Constants.RegisterSpace = 0;
	rootParameter[1].Constants.ShaderRegister = 0;

	rsDesc.NumParameters = 2;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");

	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");

	mRootSignatures["DepthPeeling"] = move(rs);

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
	psoDesc.VS.pShaderBytecode = mShaders["VolumeSphereVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["VolumeSphereVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["VolumeSpherePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["VolumeSpherePS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_ALPHA;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["VolumeSphere"] = move(pso);

	psoDesc.VS.pShaderBytecode = mShaders["VolumeCubeVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["VolumeCubeVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["VolumeCubePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["VolumeCubePS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["VolumeCube"] = move(pso);

	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 2;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["Particle"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["ParticleVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["ParticleVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = nullptr;
	psoDesc.PS.BytecodeLength = 0;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Particle"] = move(pso);


	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 2;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["RigidParticle"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["RigidParticleVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["RigidParticleVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["RigidParticlePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["RigidParticlePS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.StencilEnable = true;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["RigidParticle"] = move(pso);

	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 1;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["DepthPeeling"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["DepthPeelingVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["DepthPeelingVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["DepthPeelingPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["DepthPeelingPS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["DepthPeeling"] = move(pso);
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