#include "Engine.h"

ComPtr<ID3D12GraphicsCommandList> Engine::mCommandList;

ComPtr<ID3D12Device> Engine::mDevice = nullptr;

PSOs Engine::mPSOs;

RootSigs Engine::mRootSignatures;

ComPtr<IDXGIFactory4> Engine::mFactory = nullptr;

ComPtr<ID3D12CommandQueue> Engine::mCommandQueue;

unique_ptr<DescriptorManager> Engine::mDescriptorManager;

unique_ptr<ResourceManager>	Engine::mResourceManager;

vector<unique_ptr<Frame>> Engine::mFrames;

int	Engine::mCurrentFrame = 0;

DXGI_FORMAT	Engine::mBackBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

unique_ptr<PerlinMap> Engine::mPerlinMap;

unique_ptr<MeshManager> Engine::mMeshManager;

shared_ptr<InputManager> Engine::mInputManager;

Timer Engine::mTimer;

unordered_map<string, shared_ptr<Scene>>	Engine::mAllScenes;

string Engine::mCurrentSceneName;

shared_ptr<TextManager>	Engine::mTextManager;

shared_ptr<TextureManager> Engine::mTextureManager;

int Engine::mWidth = 800;

int	Engine::mHeight = 600;

int	Engine::mCurrentBackBuffer = 0;

int Engine::mBackBufferOffset;

Engine::Engine(HINSTANCE hInstance)
	: mInstance(hInstance)
{
	mLatestWindow = this;
}

void Engine::Initialize()
{
	//윈도우 초기화
	InitializeWindow();

	DebugEnable();

	//device, fence 등 생성
	InitializePipeline();
	
	CreateCommandObjects();

	mResourceManager = make_unique<ResourceManager>();
	mDescriptorManager = make_unique<DescriptorManager>();

	CreateFrames();

	//DirectX 객체들 생성 (swapchain, depth buffer, root signature, shader 등)
	CreateObjects();

	//각 Scene들에 모델, 카메라, 조명 생성
	//LoadScene();

	mInputManager = make_shared<InputManager>();

	mMeshManager = make_unique<MeshManager>();
	mMeshManager->Load("ball", "../Model/ball.obj");
	mMeshManager->Load("box", "../Model/box.obj");
	mMeshManager->Load("my", "../Model/my.obj");
	mMeshManager->Load("inventory", "../Model/inventory.obj");
	mMeshManager->Load("menu", "../Model/menu.obj");

	mTextureManager = make_shared<TextureManager>();
	mTextureManager->Load("brick", L"../Texture/bricks3.dds");
	mTextureManager->Load("canvas", L"../Texture/canvas.dds");
	mTextureManager->Load("backPack", L"../Texture/backpack.dds");
	mTextureManager->Load("stone", L"../Texture/stone.dds");
	mTextureManager->Load("sun", L"../Texture/sun.dds");
	mTextureManager->Load("world", L"../Texture/world.dds");
	mTextureManager->Load("earth", L"../Texture/earth.dds");

	mTextManager = make_shared<TextManager>();

	mCurrentSceneName = "MenuScene";
	//mCurrentSceneName = "MainScene";
	mAllScenes["MenuScene"] = (make_shared<MenuScene>());
	mAllScenes["MainScene"] = (make_shared<MainScene>());


	mPerlinMap = make_unique<PerlinMap>();

	IfError::Throw(mCommandList->Close(),
		L"command list close error!");

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, lists);

	mTimer.Reset();
}

void Engine::Run()
{
	MSG msg = { 0 };


	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.Tick();
			Update();
			Draw();
		}
	}
}

void Engine::DebugEnable()
{
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
}


void Engine::WaitUntilPrevFrameComplete()
{
	auto waitFrame = mFrames[(mCurrentFrame + mNumberOfFrames -1)%mNumberOfFrames].get();
	if (waitFrame->mFenceValue != 0 && mFence->GetCompletedValue() < waitFrame->mFenceValue)
	{
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		IfError::Throw(mFence->SetEventOnCompletion(waitFrame->mFenceValue, event),
			L"set event on completion error!");
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}


LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Engine::Get()->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Engine::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_LBUTTONDOWN:
		if (!mInputManager->GetMouseLeftDown())
		{
			mInputManager->SetMouseLeftDown(true);
			mInputManager->Push(msg, LOWORD(lParam), HIWORD(lParam));
			//SetCapture(hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		mInputManager->SetMouseLeftDown(false);
		mInputManager->Push(msg);
		//ReleaseCapture();

		return 0;

	case WM_MOUSEMOVE:
		if (mInputManager->GetMouseLeftDown())
		{
			mInputManager->Push(msg, LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_KEYDOWN :
		mInputManager->SetKeys(wParam, true);
		if (!mInputManager->GetKeyDown())
		{
			mInputManager->SetKeyDown(true);
			mInputManager->Push(msg, wParam);
		}
		return 0;

	case WM_KEYUP:
		mInputManager->SetKeys(wParam, false);
		mInputManager->SetKeyDown(false);
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Engine::InitializeWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWindow";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
	}
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mWidth, mHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mWindowHandle = CreateWindow(L"MainWindow", mWindowCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mInstance, 0);
	if (!mWindowHandle)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
	}

	ShowWindow(mWindowHandle, SW_SHOW);
	UpdateWindow(mWindowHandle);
}

Engine* Engine::mLatestWindow = nullptr;

Engine* Engine::Get()
{
	return mLatestWindow;
}

void Engine::CreateFrames()
{
	//효율성을 위해 cpu에서 미리 프레임을 계산해 놓기위해서 여러개의 프레임 자원을 생성.
	for (int i = 0; i < mNumberOfFrames; ++i)
	{
		mFrames.push_back(make_unique<Frame>());
	}
}

void Engine::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	IfError::Throw(Engine::mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");

	IfError::Throw(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mCommandQueue.GetAddressOf())),
		L"create command queue error!");

	IfError::Throw(mDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())),
		L"create command list error!");
}


void Engine::Update()
{
	//현재 프레임이 gpu에서 전부 draw되지 않았을 시 기다리고, 완료된 경우에는 다음 frame으로 넘어가는 역할.
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


	//실제 게임 데이터의 업데이트는 여기서부터 일어난다.
	mInputManager->Dispatch();

	mAllScenes[mCurrentSceneName]->UpdateScene(mTimer);
}

void Engine::Draw()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mResourceManager->GetResource(mBackBufferOffset + mCurrentBackBuffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	IfError::Throw(mFrames[mCurrentFrame]->Get()->Reset(),
		L"frame command allocator reset error!");

	mCommandList->Reset(mFrames[mCurrentFrame]->Get(), mPSOs["default"].Get());

	mCommandList->ResourceBarrier(1, &barrier);

	mAllScenes[mCurrentSceneName]->RenderShadowMap();

	mCommandList->RSSetScissorRects(1, &mScissor);
	mCommandList->RSSetViewports(1, &mViewport);


	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mDescriptorManager->GetCpuHandle(mCurrentBackBuffer, DescType::RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDescriptorManager->GetCpuHandle(0, DescType::DSV);
	float rgba[4] = { 0.0f,0.1f,0.0f,1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, rgba, 0, nullptr);
	mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	mCommandList->SetGraphicsRootSignature(mRootSignatures["Default"].Get());
	mCommandList->SetGraphicsRootConstantBufferView(1,
		mResourceManager->GetResource(mFrames[mCurrentFrame]->mEnvConstantBufferIdx)->GetGPUVirtualAddress());
	mCommandList->SetDescriptorHeaps(1, mDescriptorManager->GetHeapAddress(DescType::UAV));

	/*
	* scene의 object들을 draw한다.
	*/
	mAllScenes[mCurrentSceneName]->DrawScene();

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mResourceManager->GetResource(mBackBufferOffset + mCurrentBackBuffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCommandList->ResourceBarrier(1, &barrier);

	IfError::Throw(mCommandList->Close(),
		L"command list close error!");

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, lists);

	mResourceManager->SwapChainPresent();

	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % 2;
	mFrames[mCurrentFrame]->mFenceValue = ++mFenceValue;
	mCommandQueue->Signal(mFence.Get(), mFenceValue);
}


//-----------------------------pipeline에 있던 코드-----------------------------
void Engine::InitializePipeline()
{
	IfError::Throw(CreateDXGIFactory1(IID_PPV_ARGS(mFactory.GetAddressOf())),
		L"create factory error!");

	IfError::Throw(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&mDevice)),
		L"create device error!");

	IfError::Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf())),
		L"create fence error!");

	//DirectX12에서는 msaa swapchain을 생성하는 것이 지원되지 않는다. 따라서 Msaa render target을 따로 만들어야함. (추후 구현)
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {};
	qualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevels.Format = mBackBufferFormat;
	qualityLevels.NumQualityLevels = 0;
	qualityLevels.SampleCount = 4;

	IfError::Throw(mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(qualityLevels)),
		L"check feature support error!");

}

void Engine::CreateObjects()
{
	mBackBufferOffset = mResourceManager->CreateSwapChain(mWidth, mHeight, mBackBufferFormat, mWindowHandle);

	mCurrentBackBuffer = 0;

	for (int i = 0; i < 2; ++i)
		mDescriptorManager->CreateRtv(mResourceManager->GetResource(mBackBufferOffset + i));

	//create depth stencil buffer and view
	mDepthBufferIndex =  mResourceManager->CreateTexture2D(mWidth, mHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	mDescriptorManager->CreateDsv(mResourceManager->GetResource(mDepthBufferIndex), D3D12_DSV_DIMENSION_TEXTURE2D);

	CreateShaderAndRootSignature();

	CreatePso();

	SetViewportAndScissor();
}

void Engine::CreateShaderAndRootSignature()
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

	IfError::Throw(D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["planetVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, "HS", "hs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["planetHS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, "DS", "ds_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["planetDS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, "GS", "gs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["planetGS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["planetPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ColliderShape.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ColliderShapeVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ColliderShape.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ColliderShapePS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"TextShader.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["TextShaderVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"TextShader.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["TextShaderPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ShadowMap.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ShadowMapVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ShadowMap.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ShadowMapPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"DownSampling.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["DownSamplingCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Bright.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["BrightCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Blur.hlsl", nullptr, nullptr, "HorizontalBlurCS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["HorizontalBlurCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Blur.hlsl", nullptr, nullptr, "VerticalBlurCS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["VerticalBlurCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Convolve.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ConvolveCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"earth.hlsl", nullptr, nullptr, "VS", "vs_5_1", 0, 0, &blob, nullptr),
		L"compile VSshader error!");
	mShaders["earthVS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"earth.hlsl", nullptr, nullptr, "HS", "hs_5_1", 0, 0, &blob, nullptr),
		L"compile HSshader error!");
	mShaders["earthHS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"earth.hlsl", nullptr, nullptr, "DS", "ds_5_1", 0, 0, &blob, nullptr),
		L"compile DSshader error!");
	mShaders["earthDS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"earth.hlsl", nullptr, nullptr, "GS", "gs_5_1", 0, 0, &blob, nullptr),
		L"compile GSshader error!");
	mShaders["earthGS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"earth.hlsl", nullptr, nullptr, "PS", "ps_5_1", 0, 0, &blob, nullptr),
		L"compile PSshader error!");
	mShaders["earthPS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"ColorCountry.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["ColorCountryCS"] = move(blob);

	//shader에 대응되는 root signature 생성.
	ComPtr<ID3D12RootSignature> rs = nullptr;

	ComPtr<ID3DBlob> serialized;

	D3D12_DESCRIPTOR_RANGE range = {};
	range.BaseShaderRegister = 0;
	range.NumDescriptors = 1;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameter[5];
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


	D3D12_DESCRIPTOR_RANGE range2 = {};
	range2.BaseShaderRegister = 1;
	range2.NumDescriptors = 1;
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range2.RegisterSpace = 0;
	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[3].DescriptorTable.pDescriptorRanges = &range2;
	rsDesc.NumParameters = 4;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["VolumeCube"] = move(rs);

	D3D12_DESCRIPTOR_RANGE sphereRange = {};
	sphereRange.BaseShaderRegister = 1;
	sphereRange.NumDescriptors = 1;
	sphereRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	sphereRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	sphereRange.RegisterSpace = 0;
	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[3].DescriptorTable.pDescriptorRanges = &sphereRange;
	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[4].Constants.Num32BitValues = 2;
	rootParameter[4].Constants.RegisterSpace = 0;
	rootParameter[4].Constants.ShaderRegister = 2;
	rsDesc.NumParameters = 5;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["VolumeSphere"] = move(rs);

	D3D12_DESCRIPTOR_RANGE rangePlanet[4];
	rangePlanet[0].BaseShaderRegister = 0;
	rangePlanet[0].NumDescriptors = 1;
	rangePlanet[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangePlanet[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangePlanet[0].RegisterSpace = 0;
	rangePlanet[1].BaseShaderRegister = 1;
	rangePlanet[1].NumDescriptors = 1;
	rangePlanet[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangePlanet[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangePlanet[1].RegisterSpace = 0;
	rangePlanet[2].BaseShaderRegister = 2;
	rangePlanet[2].NumDescriptors = 1;
	rangePlanet[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangePlanet[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangePlanet[2].RegisterSpace = 0;
	rangePlanet[3].BaseShaderRegister = 3;
	rangePlanet[3].NumDescriptors = 1;
	rangePlanet[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangePlanet[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangePlanet[3].RegisterSpace = 0;


	D3D12_ROOT_PARAMETER rootParameterPlanet[7];
	rootParameterPlanet[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterPlanet[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[0].Descriptor.RegisterSpace = 0;
	rootParameterPlanet[0].Descriptor.ShaderRegister = 0;
	rootParameterPlanet[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterPlanet[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[1].Descriptor.RegisterSpace = 0;
	rootParameterPlanet[1].Descriptor.ShaderRegister = 1;
	rootParameterPlanet[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterPlanet[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterPlanet[2].DescriptorTable.pDescriptorRanges = &rangePlanet[0];
	rootParameterPlanet[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterPlanet[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterPlanet[3].DescriptorTable.pDescriptorRanges = &rangePlanet[1];
	rootParameterPlanet[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterPlanet[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[4].Constants.Num32BitValues = 1;
	rootParameterPlanet[4].Constants.RegisterSpace = 0;
	rootParameterPlanet[4].Constants.ShaderRegister = 2;
	rootParameterPlanet[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterPlanet[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterPlanet[5].DescriptorTable.pDescriptorRanges = &rangePlanet[2];
	rootParameterPlanet[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterPlanet[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterPlanet[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterPlanet[6].DescriptorTable.pDescriptorRanges = &rangePlanet[3];



	rsDesc.NumParameters = 7;
	rsDesc.pParameters = rootParameterPlanet;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["planet"] = move(rs);


	D3D12_ROOT_PARAMETER rootParameterCollider[3];
	rootParameterCollider[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterCollider[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterCollider[0].Descriptor.RegisterSpace = 0;
	rootParameterCollider[0].Descriptor.ShaderRegister = 0;
	rootParameterCollider[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterCollider[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterCollider[1].Descriptor.RegisterSpace = 0;
	rootParameterCollider[1].Descriptor.ShaderRegister = 1;
	rootParameterCollider[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterCollider[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterCollider[2].Constants.Num32BitValues = 4;
	rootParameterCollider[2].Constants.RegisterSpace = 0;
	rootParameterCollider[2].Constants.ShaderRegister = 2;

	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = 3;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.pParameters = rootParameterCollider;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["ColliderShape"] = move(rs);


	D3D12_DESCRIPTOR_RANGE textRange[3];
	textRange[0].BaseShaderRegister = 0;
	textRange[0].NumDescriptors = 1;
	textRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	textRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textRange[0].RegisterSpace = 0;
	textRange[1].BaseShaderRegister = 1;
	textRange[1].NumDescriptors = 1;
	textRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	textRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textRange[1].RegisterSpace = 0;
	textRange[2].BaseShaderRegister = 2;
	textRange[2].NumDescriptors = 1;
	textRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	textRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textRange[2].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameterText[6];
	rootParameterText[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterText[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterText[0].Descriptor.RegisterSpace = 0;
	rootParameterText[0].Descriptor.ShaderRegister = 0;
	rootParameterText[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterText[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterText[1].Descriptor.RegisterSpace = 0;
	rootParameterText[1].Descriptor.ShaderRegister = 1;
	rootParameterText[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterText[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameterText[2].Constants.Num32BitValues = 1;
	rootParameterText[2].Constants.RegisterSpace = 0;
	rootParameterText[2].Constants.ShaderRegister = 2;
	rootParameterText[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterText[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterText[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterText[3].DescriptorTable.pDescriptorRanges = &textRange[0];
	rootParameterText[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterText[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterText[4].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterText[4].DescriptorTable.pDescriptorRanges = &textRange[1];
	rootParameterText[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterText[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterText[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterText[5].DescriptorTable.pDescriptorRanges = &textRange[2];

	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rsDesc.NumParameters = 6;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.pParameters = rootParameterText;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["Text"] = move(rs);


	D3D12_ROOT_PARAMETER rootParameterShadowMap[3];
	rootParameterShadowMap[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterShadowMap[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterShadowMap[0].Descriptor.RegisterSpace = 0;
	rootParameterShadowMap[0].Descriptor.ShaderRegister = 0;
	rootParameterShadowMap[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterShadowMap[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; 
	rootParameterShadowMap[1].Descriptor.RegisterSpace = 0;
	rootParameterShadowMap[1].Descriptor.ShaderRegister = 1;
	rootParameterShadowMap[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterShadowMap[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; 
	rootParameterShadowMap[2].Constants.Num32BitValues = 1;
	rootParameterShadowMap[2].Constants.RegisterSpace = 0;
	rootParameterShadowMap[2].Constants.ShaderRegister = 2;


	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = 3;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.pParameters = rootParameterShadowMap;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["ShadowMap"] = move(rs);

	D3D12_DESCRIPTOR_RANGE downSamplingRange[2];
	downSamplingRange[0].BaseShaderRegister = 0;
	downSamplingRange[0].NumDescriptors = 1;
	downSamplingRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	downSamplingRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	downSamplingRange[0].RegisterSpace = 0;
	downSamplingRange[1].BaseShaderRegister = 0;
	downSamplingRange[1].NumDescriptors = 1;
	downSamplingRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	downSamplingRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	downSamplingRange[1].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameterDownSampling[2];
	rootParameterDownSampling[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterDownSampling[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterDownSampling[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterDownSampling[0].DescriptorTable.pDescriptorRanges = &downSamplingRange[0];
	rootParameterDownSampling[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterDownSampling[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterDownSampling[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterDownSampling[1].DescriptorTable.pDescriptorRanges = &downSamplingRange[1];

	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rsDesc.NumParameters = 2;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pParameters = rootParameterDownSampling;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["DownSampling"] = move(rs);

	D3D12_DESCRIPTOR_RANGE convolveRange[3];
	convolveRange[0].BaseShaderRegister = 0;
	convolveRange[0].NumDescriptors = 1;
	convolveRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	convolveRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	convolveRange[0].RegisterSpace = 0;
	convolveRange[1].BaseShaderRegister = 1;
	convolveRange[1].NumDescriptors = 1;
	convolveRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	convolveRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	convolveRange[1].RegisterSpace = 0;
	convolveRange[2].BaseShaderRegister = 0;
	convolveRange[2].NumDescriptors = 1;
	convolveRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	convolveRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	convolveRange[2].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameterConvolve[3];
	rootParameterConvolve[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterConvolve[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterConvolve[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterConvolve[0].DescriptorTable.pDescriptorRanges = &convolveRange[0];
	rootParameterConvolve[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterConvolve[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterConvolve[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterConvolve[1].DescriptorTable.pDescriptorRanges = &convolveRange[1];
	rootParameterConvolve[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterConvolve[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterConvolve[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterConvolve[2].DescriptorTable.pDescriptorRanges = &convolveRange[2];

	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rsDesc.NumParameters = 3;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pParameters = rootParameterConvolve;

	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["Convolve"] = move(rs);


	D3D12_DESCRIPTOR_RANGE rangeEarth[5];
	rangeEarth[0].BaseShaderRegister = 0;
	rangeEarth[0].NumDescriptors = 1;
	rangeEarth[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeEarth[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeEarth[0].RegisterSpace = 0;
	rangeEarth[1].BaseShaderRegister = 1;
	rangeEarth[1].NumDescriptors = 1;
	rangeEarth[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeEarth[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeEarth[1].RegisterSpace = 0;
	rangeEarth[2].BaseShaderRegister = 2;
	rangeEarth[2].NumDescriptors = 1;
	rangeEarth[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeEarth[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeEarth[2].RegisterSpace = 0;
	rangeEarth[3].BaseShaderRegister = 3;
	rangeEarth[3].NumDescriptors = 1;
	rangeEarth[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeEarth[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeEarth[3].RegisterSpace = 0;
	rangeEarth[4].BaseShaderRegister = 4;
	rangeEarth[4].NumDescriptors = 1;
	rangeEarth[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeEarth[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeEarth[4].RegisterSpace = 0;


	D3D12_ROOT_PARAMETER rootParameterEarth[8];
	rootParameterEarth[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterEarth[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[0].Descriptor.RegisterSpace = 0;
	rootParameterEarth[0].Descriptor.ShaderRegister = 0;
	rootParameterEarth[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameterEarth[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[1].Descriptor.RegisterSpace = 0;
	rootParameterEarth[1].Descriptor.ShaderRegister = 1;
	rootParameterEarth[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterEarth[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterEarth[2].DescriptorTable.pDescriptorRanges = &rangeEarth[0];
	rootParameterEarth[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterEarth[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterEarth[3].DescriptorTable.pDescriptorRanges = &rangeEarth[1];
	rootParameterEarth[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterEarth[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[4].Constants.Num32BitValues = 2;
	rootParameterEarth[4].Constants.RegisterSpace = 0;
	rootParameterEarth[4].Constants.ShaderRegister = 2;
	rootParameterEarth[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterEarth[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterEarth[5].DescriptorTable.pDescriptorRanges = &rangeEarth[2];
	rootParameterEarth[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterEarth[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterEarth[6].DescriptorTable.pDescriptorRanges = &rangeEarth[3];
	rootParameterEarth[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterEarth[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterEarth[7].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterEarth[7].DescriptorTable.pDescriptorRanges = &rangeEarth[4];

	rsDesc.NumParameters = 8;
	rsDesc.pParameters = rootParameterEarth;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["earth"] = move(rs);


	D3D12_DESCRIPTOR_RANGE rangeColorCountry[2];
	rangeColorCountry[0].BaseShaderRegister = 0;
	rangeColorCountry[0].NumDescriptors = 1;
	rangeColorCountry[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeColorCountry[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeColorCountry[0].RegisterSpace = 0;
	rangeColorCountry[1].BaseShaderRegister = 0;
	rangeColorCountry[1].NumDescriptors = 1;
	rangeColorCountry[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeColorCountry[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	rangeColorCountry[1].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameterColorCountry[3];
	rootParameterColorCountry[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterColorCountry[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterColorCountry[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterColorCountry[0].DescriptorTable.pDescriptorRanges = &rangeColorCountry[0];
	rootParameterColorCountry[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameterColorCountry[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterColorCountry[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameterColorCountry[1].DescriptorTable.pDescriptorRanges = &rangeColorCountry[1];
	rootParameterColorCountry[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameterColorCountry[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameterColorCountry[2].Constants.Num32BitValues = 1;
	rootParameterColorCountry[2].Constants.RegisterSpace = 0;
	rootParameterColorCountry[2].Constants.ShaderRegister = 0;

	rsDesc.NumParameters = 3;
	rsDesc.pParameters = rootParameterColorCountry;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &samplerDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["ColorCountry"] = move(rs);
}

void Engine::CreatePso()
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
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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

	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

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

	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Default"] = move(pso);

	psoDesc.InputLayout.NumElements = 0;
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.pRootSignature = mRootSignatures["VolumeSphere"].Get();
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

	psoDesc.pRootSignature = mRootSignatures["VolumeCube"].Get();
	psoDesc.VS.pShaderBytecode = mShaders["VolumeCubeVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["VolumeCubeVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["VolumeCubePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["VolumeCubePS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["VolumeCube"] = move(pso);


	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 1;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["ColliderShape"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["ColliderShapeVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["ColliderShapeVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["ColliderShapePS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["ColliderShapePS"]->GetBufferSize();
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["ColliderShape"] = move(pso);


	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "TEXTURE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[2] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 3;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["planet"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["planetVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["planetVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["planetPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["planetPS"]->GetBufferSize();
	psoDesc.HS.pShaderBytecode = mShaders["planetHS"]->GetBufferPointer();
	psoDesc.HS.BytecodeLength = mShaders["planetHS"]->GetBufferSize();
	psoDesc.DS.pShaderBytecode = mShaders["planetDS"]->GetBufferPointer();
	psoDesc.DS.BytecodeLength = mShaders["planetDS"]->GetBufferSize();
	psoDesc.GS.pShaderBytecode = mShaders["planetGS"]->GetBufferPointer();
	psoDesc.GS.BytecodeLength = mShaders["planetGS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.RasterizerState.DepthBias = 100;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["planet"] = move(pso);

	psoDesc.InputLayout.NumElements = 0;
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.pRootSignature = mRootSignatures["Text"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["TextShaderVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["TextShaderVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["TextShaderPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["TextShaderPS"]->GetBufferSize();
	psoDesc.HS.pShaderBytecode = nullptr;
	psoDesc.HS.BytecodeLength = 0;
	psoDesc.DS.pShaderBytecode = nullptr;
	psoDesc.DS.BytecodeLength = 0;
	psoDesc.GS.pShaderBytecode = nullptr;
	psoDesc.GS.BytecodeLength = 0;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Text"] = move(pso);


	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "TEXTURE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[2] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 3;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["ShadowMap"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["ShadowMapVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["ShadowMapVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["ShadowMapPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["ShadowMapPS"]->GetBufferSize();
	psoDesc.HS.pShaderBytecode = nullptr;
	psoDesc.HS.BytecodeLength = 0;
	psoDesc.DS.pShaderBytecode = nullptr;
	psoDesc.DS.BytecodeLength = 0;
	psoDesc.GS.pShaderBytecode = nullptr;
	psoDesc.GS.BytecodeLength = 0;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["ShadowMap"] = move(pso);


	D3D12_COMPUTE_PIPELINE_STATE_DESC downSamplingPsoDesc = {};
	downSamplingPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	downSamplingPsoDesc.CachedPSO.pCachedBlob = nullptr;
	downSamplingPsoDesc.CS.BytecodeLength = mShaders["DownSamplingCS"]->GetBufferSize();
	downSamplingPsoDesc.CS.pShaderBytecode = mShaders["DownSamplingCS"]->GetBufferPointer();
	downSamplingPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	downSamplingPsoDesc.NodeMask = 0;
	downSamplingPsoDesc.pRootSignature = mRootSignatures["DownSampling"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&downSamplingPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["DownSampling"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC brightPsoDesc = {};
	brightPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	brightPsoDesc.CachedPSO.pCachedBlob = nullptr;
	brightPsoDesc.CS.BytecodeLength = mShaders["BrightCS"]->GetBufferSize();
	brightPsoDesc.CS.pShaderBytecode = mShaders["BrightCS"]->GetBufferPointer();
	brightPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	brightPsoDesc.NodeMask = 0;
	brightPsoDesc.pRootSignature = mRootSignatures["DownSampling"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&brightPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["Bright"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC HorizontalBlurPsoDesc = {};
	HorizontalBlurPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	HorizontalBlurPsoDesc.CachedPSO.pCachedBlob = nullptr;
	HorizontalBlurPsoDesc.CS.BytecodeLength = mShaders["HorizontalBlurCS"]->GetBufferSize();
	HorizontalBlurPsoDesc.CS.pShaderBytecode = mShaders["HorizontalBlurCS"]->GetBufferPointer();
	HorizontalBlurPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HorizontalBlurPsoDesc.NodeMask = 0;
	HorizontalBlurPsoDesc.pRootSignature = mRootSignatures["DownSampling"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&HorizontalBlurPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["HorizontalBlur"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC VerticalBlurPsoDesc = {};
	VerticalBlurPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	VerticalBlurPsoDesc.CachedPSO.pCachedBlob = nullptr;
	VerticalBlurPsoDesc.CS.BytecodeLength = mShaders["VerticalBlurCS"]->GetBufferSize();
	VerticalBlurPsoDesc.CS.pShaderBytecode = mShaders["VerticalBlurCS"]->GetBufferPointer();
	VerticalBlurPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	VerticalBlurPsoDesc.NodeMask = 0;
	VerticalBlurPsoDesc.pRootSignature = mRootSignatures["DownSampling"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&VerticalBlurPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["VerticalBlur"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC ConvolvePsoDesc = {};
	ConvolvePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	ConvolvePsoDesc.CachedPSO.pCachedBlob = nullptr;
	ConvolvePsoDesc.CS.BytecodeLength = mShaders["ConvolveCS"]->GetBufferSize();
	ConvolvePsoDesc.CS.pShaderBytecode = mShaders["ConvolveCS"]->GetBufferPointer();
	ConvolvePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ConvolvePsoDesc.NodeMask = 0;
	ConvolvePsoDesc.pRootSignature = mRootSignatures["Convolve"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&ConvolvePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["Convolve"] = move(pso);


	inputElements[0] = { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[1] = { "TEXTURE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	inputElements[2] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0 };
	psoDesc.InputLayout.NumElements = 3;
	psoDesc.InputLayout.pInputElementDescs = inputElements;
	psoDesc.pRootSignature = mRootSignatures["earth"].Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.VS.pShaderBytecode = mShaders["earthVS"]->GetBufferPointer();
	psoDesc.VS.BytecodeLength = mShaders["earthVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = mShaders["earthPS"]->GetBufferPointer();
	psoDesc.PS.BytecodeLength = mShaders["earthPS"]->GetBufferSize();
	psoDesc.HS.pShaderBytecode = mShaders["earthHS"]->GetBufferPointer();
	psoDesc.HS.BytecodeLength = mShaders["earthHS"]->GetBufferSize();
	psoDesc.DS.pShaderBytecode = mShaders["earthDS"]->GetBufferPointer();
	psoDesc.DS.BytecodeLength = mShaders["earthDS"]->GetBufferSize();
	psoDesc.GS.pShaderBytecode = mShaders["earthGS"]->GetBufferPointer();
	psoDesc.GS.BytecodeLength = mShaders["earthGS"]->GetBufferSize();
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.RasterizerState.DepthBias = 100;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["earth"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC ColorCountryPsoDesc = {};
	ColorCountryPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	ColorCountryPsoDesc.CachedPSO.pCachedBlob = nullptr;
	ColorCountryPsoDesc.CS.BytecodeLength = mShaders["ColorCountryCS"]->GetBufferSize();
	ColorCountryPsoDesc.CS.pShaderBytecode = mShaders["ColorCountryCS"]->GetBufferPointer();
	ColorCountryPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ColorCountryPsoDesc.NodeMask = 0;
	ColorCountryPsoDesc.pRootSignature = mRootSignatures["ColorCountry"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&ColorCountryPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create compute pso error!");
	mPSOs["ColorCountry"] = move(pso);
}

void Engine::SetViewportAndScissor()
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

void Engine::ChangeScene(const string& sceneName)
{
	mCurrentSceneName = sceneName;
}
