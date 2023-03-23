#include "Engine.h"

vector<unique_ptr<Scene>> Engine::mScenes;
ComPtr<ID3D12GraphicsCommandList> Engine::mCommandList;
int Engine::mCurrentScene;

ComPtr<ID3D12Device> Engine::mDevice = nullptr;
PSOs Engine::mPSOs;
RootSigs Engine::mRootSignatures;

ComPtr<IDXGIFactory4> Engine::mFactory = nullptr;
ComPtr<ID3D12CommandQueue> Engine::mCommandQueue;

unique_ptr<DescriptorManager> Engine::mDescriptorManager;
unique_ptr<ResourceManager>	Engine::mResourceManager;

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
	//commandList가 필요하기 때문에 texture load를 여기에서 한다.
	//commandList가 필요하기 때문에 DirectX objects 생성 후에 model을 buffer에 복사한다.
	LoadScene();
	LoadCopyModelToBuffer();

	//texture가 로드된 후에 srv를 생성할 수 있기 때문에 다른 오브젝트들과 따로 생성한다.
	for (auto scene = mScenes.begin(); scene != mScenes.end(); scene++)
	{
		scene->get()->CreateModelSrv(MAX_OBJECT);
		scene->get()->CreateVolumeUav(MAX_OBJECT);
	}

	mParticleField = make_unique<ParticleField>();

	mRigidBodySystem = make_unique<RigidBodySystem>();

	mRigidBodySystem->Load();
	mRigidBodySystem->GenerateParticle();

	CreateNoiseMap();

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

void Engine::ChangeScene(int dstScene)
{
	mCurrentScene = dstScene;
}

void Engine::SelectObject(int x, int y)
{
	XMFLOAT3 newPos;
	float p00 = mScenes[mCurrentScene]->envFeature.projection._11;
	float p11 = mScenes[mCurrentScene]->envFeature.projection._22;
	
	//viewport에서 view coordinate으로 변환, z = 1
	newPos.x = (2.0f * x / (float)mWidth - 1.0f)/p00;
	newPos.y = (-2.0f * y / (float)mHeight + 1.0f)/p11;
	newPos.z = 1.0f;
	
	//newPos를 VC에서 WC로 변환한다.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&mScenes[mCurrentScene]->mCamera->view);
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det,inverseViewMatrix);

	XMVECTOR rayVector = XMLoadFloat3(&newPos);
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	//XMVector3TransformCoord는 일반적인 변환이고
	//XMVector3TransformNormal은 행렬의 0~3행 중 3행을 무시한다. 따라서 translation은 할 수 없다.
	rayVector = XMVector3Normalize(XMVector3TransformNormal(rayVector, inverseViewMatrix));
	rayOrigin = XMVector3TransformCoord(rayOrigin, inverseViewMatrix);

	//clickable에서 모델을 선택한다.
	float prevDist = 1000.0f;
	float dist = 1000.0f;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		BoundingOrientedBox boundingBox;

		model->second->mBound.Transform(boundingBox, XMLoadFloat4x4(&model->second->mObjFeature.world));

		if (boundingBox.Intersects(rayOrigin, rayVector, dist))
		{
			if (dist < prevDist)
			{
				mIsModelSelected = true;
				mSelectedModel = model->second;
				mSelectedModelName = model->first;
				prevDist = dist;
			}
		}
	}
}

void Engine::MoveObject(int x, int y)
{
	float p00 = mScenes[mCurrentScene]->envFeature.projection._11;
	float p11 = mScenes[mCurrentScene]->envFeature.projection._22;
	XMFLOAT3 newPos;

	//viewport에서 view coordinate으로 변환
	newPos.x = (2.0f * x / (float)mWidth - 1.0f) / p00*10.0f;
	newPos.y = (-2.0f * y / (float)mHeight + 1.0f) / p11*10.0f;
	newPos.z = 10.0f;

	//newPos를 VC에서 WC로 변환한다.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&mScenes[mCurrentScene]->mCamera->view);
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det, inverseViewMatrix);

	XMVECTOR rayVector = XMLoadFloat3(&newPos);

	rayVector = XMVector3TransformCoord(rayVector, inverseViewMatrix);

	XMStoreFloat3(&newPos, rayVector);

	mSelectedModel->SetPosition(newPos);
}

void Engine::SelectInventory(int x, int y)
{
	XMFLOAT3 newPos;
	float p00 = mScenes[mCurrentScene]->envFeature.projection._11;
	float p11 = mScenes[mCurrentScene]->envFeature.projection._22;

	//viewport에서 view coordinate으로 변환, z = 1
	newPos.x = (2.0f * x / (float)mWidth - 1.0f) / p00;
	newPos.y = (-2.0f * y / (float)mHeight + 1.0f) / p11;
	newPos.z = 1.0f;

	//newPos를 VC에서 WC로 변환한다.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&mScenes[mCurrentScene]->mCamera->view);
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det, inverseViewMatrix);

	XMVECTOR rayVector = XMLoadFloat3(&newPos);
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	//XMVector3TransformCoord는 일반적인 변환이고
	//XMVector3TransformNormal은 행렬의 0~3행 중 3행을 무시한다. 따라서 translation은 할 수 없다.
	rayVector = XMVector3Normalize(XMVector3TransformNormal(rayVector, inverseViewMatrix));
	rayOrigin = XMVector3TransformCoord(rayOrigin, inverseViewMatrix);

	float dist = 1000.0f;
	BoundingOrientedBox boundingBox;

	if (mScenes[mCurrentScene]->mModels->count("inventory") != 0)
	{
		auto inventory = mScenes[mCurrentScene]->mModels->at("inventory");

		inventory->mBound.Transform(boundingBox, XMLoadFloat4x4(&inventory->mObjFeature.world));

		if (boundingBox.Intersects(rayOrigin, rayVector, dist))
		{
			mIsInventorySelected = true;
		}
		else
		{
			mIsInventorySelected = false;
		}
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
		if (mIsModelSelected == false)
		{
			SelectObject(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_LBUTTONUP:
		if (mIsInventorySelected == true)
		{
			mIsInventorySelected = false;

			Inventory* invtry = static_cast<Inventory*>(mScenes[mCurrentScene]->mModels->at("inventory").get());
			invtry->Store(mSelectedModelName, move(mScenes[mCurrentScene]->mModels->at(mSelectedModelName)));
			mScenes[mCurrentScene]->mModels->at(mSelectedModelName).reset();
			mScenes[mCurrentScene]->mModels->erase(mSelectedModelName);
			// mModels의 목록에서 완전히 제거해야하기 때문에 erase를 한다.
		}
		//임시
		if (mSelectedModelName.compare("button") == 0)
		{
			Button* b = static_cast<Button*>(mSelectedModel.get());
			b->Click();
		}
		mIsModelSelected = false;
		mSelectedModel = nullptr;

		return 0;

	case WM_MOUSEMOVE:
		if (mIsModelSelected == true)
		{
			MoveObject(LOWORD(lParam), HIWORD(lParam));
			if(mSelectedModelName.compare("inventory") != 0)
				SelectInventory(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_KEYDOWN :
		if (wParam == 0x54)
		{
			Inventory* invtry = static_cast<Inventory*>(mScenes[mCurrentScene]->mModels->at("inventory").get());
			mScenes[mCurrentScene]->mModels->insert({ "earth",invtry->Release("earth") });
			invtry->mInventory.erase("earth");

			mScenes[mCurrentScene]->mModels->insert({ "rifle",invtry->Release("rifle") });
			invtry->mInventory.erase("rifle");

			// mInventory의 목록에서 완전히 제거해야하기 때문에 erase를 한다.
		}
		else if (wParam == 0x45)
		{
			mScenes[mCurrentScene]->mSpawnSystem->SpawnPush({"Clickable","../Model/ball.obj",L"../Model/textures/bricks1.dds",{0.3f,3.0f,1.0f} });
		}
		else if (wParam == 0x52)
		{
			mScenes[mCurrentScene]->mSpawnSystem->DestroyPush({ "Clickable","0"});
		}

		return 0;
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


void Engine::LoadScene()
{
	//scene 0
	mScenes.push_back(make_unique<Scene>());

	//모델 로드 (버텍스, 인덱스)
	mScenes[mCurrentScene]->mModels = CreateModel(0);

	mScenes[mCurrentScene]->mWorld = CreateWorld(0);

	mScenes[mCurrentScene]->mVolume = CreateVolume(0);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();
	

	//scene 1
	mCurrentScene++;
	mScenes.push_back(make_unique<Scene>());
	
	//모델 로드 (버텍스, 인덱스)
	mScenes[mCurrentScene]->mModels = CreateModel(1);

	mScenes[mCurrentScene]->mWorld = CreateWorld(1);

	mScenes[mCurrentScene]->mVolume = CreateVolume(1);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();
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


/*
* model의 vertex,index offset이 scene 내에서 존재하기 buffer가 존재하기 때문에 어떤 scene내에 로드할 건지를 명시해준다.
*/
unique_ptr<Clickables> Engine::CreateModel(int sceneIndex)
{
	unique_ptr<Clickables> model = make_unique<Clickables>();

	shared_ptr<Clickable> m;

	if (sceneIndex == 0)
	{
		//m = make_shared<Clickable>("../Model/ball.obj", L"../Model/textures/bricks1.dds");
		//m->SetPosition(15.0f, 3.0f, 3.0f);
		//(*model)["table"] = move(m);

		//m = make_shared<Clickable>("../Model/sword.obj", L"../Model/textures/bricks2.dds");
		//m->SetPosition(2.0f, 0.0f, 0.0f);
		//m->mScale = { 0.1f,0.1f,0.1f };
		//(*model)["sword"] = move(m);

		//m = make_shared<Clickable>("../Model/my.obj", L"../Model/textures/checkboard.dds");
		//(*model)["my"] = move(m);

		//shared_ptr<Button> b = make_shared<Button>("../Model/inventory.obj", L"../Model/textures/earth.dds");
		//b->SetPosition(0.0f, -3.0f, 5.0f);
		//b->Set([&]() {
		//	ChangeScene(1);
		//});
		//(*model)["button"] = move(b);
	}
	else if (sceneIndex == 1)
	{
		m = make_shared<Clickable>("../Model/my.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(13.0f, 3.0f, 13.0f);
		(*model)["rifie0"] = move(m);

		m = make_shared<Clickable>("../Model/ball.obj", L"../Model/textures/earth.dds");
		m->SetPosition(10.0f,3.0f,10.0f);
		m->mScale = { 10.0f,10.0f,10.0f };
		m->mRigidBody->SetLinearMomentum(0.0f, 0.0f, 0.0f);
		m->mRigidBody->SetAngularMomentum(1.0f, 10.0f, 1.0f);
		(*model)["earth"] = move(m);

		m = make_shared<Clickable>("../Model/box.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(11.0f, 2.5f, 13.0f);
		m->mRigidBody->SetAngularMomentum(1000.0f, 10.0f, 10.0f);
		(*model)["lamp"] = move(m);

		m = make_shared<Clickable>("../Model/box.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(15.0f, 2.5f, 13.0f);
		(*model)["lamp0"] = move(m);

		m = make_shared<Clickable>("../Model/box.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(19.0f, 2.5f, 13.0f);
		(*model)["lamp1"] = move(m);

		m = make_shared<Inventory>("../Model/inventory.obj", L"../Model/textures/inventory.dds");
		m->SetPosition(0.0f, -1.5f, 5.0f);
		(*model)["inventory"] = move(m);
		
		shared_ptr<Button> b = make_shared<Button>("../Model/inventory.obj", L"../Model/textures/earth.dds");
		b->SetPosition(0.0f, -3.0f, 5.0f);
		b->Set([&](){
			ChangeScene(0);
		});
		(*model)["button"] = move(b);

		m = make_shared<Clickable>("../Model/my.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(5.0f, 3.0f, 3.0f);
		(*model)["rifle"] = move(m);
	}

	return move(model);
}



void Engine::LoadCopyModelToBuffer()
{
	//모델 데이터, 텍스처 로드, 버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
	for (int i = 0; i <= mCurrentScene; ++i)
	{
		mScenes[i]->Load();
	}
}

unique_ptr<Unclickables> Engine::CreateWorld(int sceneIndex)
{
	unique_ptr<Unclickables> model = make_unique<Unclickables>();

	shared_ptr<Unclickable> m;

	if (sceneIndex == 0)
	{
		m = make_shared<Unclickable>("../Model/space.obj", L"../Model/textures/stars.dds");
		(*model)["space"] = move(m);
	}
	else if (sceneIndex == 1)
	{
		m = make_shared<Unclickable>("../Model/space.obj", L"../Model/textures/stars.dds");
		(*model)["space"] = move(m);
	}

	return move(model);
}

unique_ptr<Volumes> Engine::CreateVolume(int sceneIndex)
{
	unique_ptr<Volumes> volumes = make_unique<Volumes>();

	shared_ptr<Volume> v;

	if (sceneIndex == 0)
	{
		v = make_shared<VolumeSphere>();
		(*volumes)["sphere"] = move(v);

		v = make_shared<VolumeCube>();
		v->SetPosition(9.0f, 0.0f, 0.0f);
		(*volumes)["cube"] = move(v);
	}
	else if (sceneIndex == 1)
	{
		v = make_shared<VolumeSphere>();
		(*volumes)["sphere"] = move(v);

		v = make_shared<VolumeCube>();
		v->SetPosition(9.0f, 0.0f, 0.0f);
		(*volumes)["cube"] = move(v);
	}

	return move(volumes);

}

env Engine::SetLight()
{
	env envs;
		
	envs.lights[0].mPosition = { 10.0f,0.0f,0.0f };
	envs.lights[0].mDirection = { -1.0f,0.0f,0.0f };
	envs.lights[0].mColor = {1.0f,1.0f,1.0f };
	envs.lights[0].mType = Point;
	envs.lights[1].mPosition = { 9.0f,3.0f,0.0f };
	envs.lights[1].mDirection = { 0.0f,-1.0f,0.0f };
	envs.lights[1].mColor = { 1.0f,1.0f,0.0f };
	envs.lights[1].mType = Spot;
	envs.lights[2].mPosition = { 0.0f, 3.0f,0.0f };
	envs.lights[2].mDirection = { 0.0f,-1.0f,0.0f };
	envs.lights[2].mColor = { 1.0f,1.0f,0.0f };
	envs.lights[2].mType = Spot;

	return envs;
}

/*
void Engine::SetObjConstantIndex(int index)
{
	mCommandList->SetGraphicsRootConstantBufferView(0, mFrames[mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
		+ index * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
}
*/

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
	Input();
	//mScenes[mCurrentScene]->mModels->at("earth")->MulQuaternion(0.0f, sinf(mTimer.GetDeltaTime()), 0.0f,cosf(mTimer.GetDeltaTime()));
	mScenes[mCurrentScene]->Update();
	mScenes[mCurrentScene]->envFeature.view = mScenes[mCurrentScene]->mCamera->view;
	mScenes[mCurrentScene]->envFeature.projection = mScenes[mCurrentScene]->mCamera->projection;
	mScenes[mCurrentScene]->envFeature.cameraPosition = mScenes[mCurrentScene]->mCamera->GetPosition();
	mScenes[mCurrentScene]->envFeature.cameraFront = mScenes[mCurrentScene]->mCamera->mFront;
	mScenes[mCurrentScene]->envFeature.invViewProjection = mScenes[mCurrentScene]->mCamera->invViewProjection;
	//mFrames[mCurrentFrame]->CopyTransConstantBuffer(0, &mScenes[mCurrentScene]->envFeature, sizeof(env));
	mResourceManager->Upload(mFrames[mCurrentFrame]->mEnvConstantBufferIdx, &mScenes[mCurrentScene]->envFeature, sizeof(env), 0);

	//각 모델별로 obj constant를 constant buffer의 해당위치에 로드함.
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		//mFrames[mCurrentFrame]->CopyObjConstantBuffer(model->second->mObjIndex, &model->second->mObjFeature, sizeof(obj));
		mResourceManager->Upload(mFrames[mCurrentFrame]->mObjConstantBufferIdx, &model->second->mObjFeature, sizeof(obj),
			model->second->mObjIndex * constantBufferAlignment(sizeof(obj)));
	}

	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		//mFrames[mCurrentFrame]->CopyObjConstantBuffer(world->second->mObjIndex, &world->second->mObjFeature, sizeof(obj));
		mResourceManager->Upload(mFrames[mCurrentFrame]->mObjConstantBufferIdx, &world->second->mObjFeature, sizeof(obj),
			world->second->mObjIndex * constantBufferAlignment(sizeof(obj)));
	}

	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		//mFrames[mCurrentFrame]->CopyObjConstantBuffer(volume->second->mObjIndex, &volume->second->mObjFeature, sizeof(obj));
		mResourceManager->Upload(mFrames[mCurrentFrame]->mObjConstantBufferIdx, &volume->second->mObjFeature, sizeof(obj),
			volume->second->mObjIndex * constantBufferAlignment(sizeof(obj)));
	}
	
	mParticleField->Update(mTimer);
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
	//
	
	//texture 초기화 해야함.
	//첫번째 draw인 경우에는 제외해야함.
	
	mRigidBodySystem->UploadRigidBody();
	mRigidBodySystem->CalculateRigidInertia(RigidBodySystem::mRigidBodies.size());
	mRigidBodySystem->CalculateParticlePosition(RigidBodySystem::mRigidBodies.size());
	mRigidBodySystem->CalculateParticleVelocity(RigidBodySystem::mRigidBodies.size());
	mRigidBodySystem->PutParticleOnGrid(RigidBodySystem::mRigidBodies.size());
	mRigidBodySystem->ParticleCollision(RigidBodySystem::mRigidBodies.size());
	mRigidBodySystem->NextRigidMomentum(mTimer.GetDeltaTime());
	mRigidBodySystem->NextRigidPosQuat(RigidBodySystem::mRigidBodies.size(), mTimer.GetDeltaTime());

	mRigidBodySystem->CopyRigidBody();

	if (mFenceValue > 0)
	{
		WaitUntilPrevFrameComplete();
		mRigidBodySystem->UpdateRigidBody();
	}
	
	//

	mScenes[mCurrentScene]->Spawn();

	if (!mScenes[mCurrentScene]->IsDestroyQueueEmpty())
	{
		WaitUntilPrevFrameComplete();
		mScenes[mCurrentScene]->Destroy();
	}

	mCommandList->RSSetScissorRects(1, &mScissor);
	mCommandList->RSSetViewports(1, &mViewport);

	mCommandList->ResourceBarrier(1, &barrier);


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

	//particle density update
	ID3D12DescriptorHeap* heaps[] = { mScenes[mCurrentScene]->mVolumeUavHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	mCommandList->SetPipelineState(mPSOs["Particle"].Get());
	mCommandList->SetGraphicsRootSignature(mRootSignatures["Particle"].Get());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	mCommandList->IASetVertexBuffers(0, 1, mParticleField->GetVertexBufferView());
	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += volume->second->mVolumeIndex * mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mScenes[mCurrentScene]->mVolumeUavHeapInvisible->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += volume->second->mVolumeIndex * mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		UINT color[4] = { 0,0,0,0 };
		mCommandList->ClearUnorderedAccessViewUint(gpuHandle,cpuHandle, volume->second->mTextureResource->mTexture.Get(), color, 0, nullptr);
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
		mCommandList->SetGraphicsRootDescriptorTable(0, handle);
		mCommandList->DrawInstanced(mParticleField->NumParticle(),1,0,0);
	}
	//

	mCommandList->SetGraphicsRootSignature(mRootSignatures["Default"].Get());

	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//heaps[0] = { mScenes[mCurrentScene]->mSrvHeap.Get() };
	//mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	mCommandList->SetDescriptorHeaps(1, mDescriptorManager->GetHeapAddress(DescType::SRV));

	mCommandList->SetPipelineState(mPSOs["World"].Get());
	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		//D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
		//handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * world->second->mObjIndex;
		mCommandList->SetGraphicsRootDescriptorTable(2,mDescriptorManager->GetGpuHandle(mScenes[mCurrentScene]->mSrvIndices[world->first],DescType::SRV));
		world->second->Draw();
	}

	//선택된 물체에 노란색 테두리 렌더링
	if (mIsModelSelected == true)
	{
		mCommandList->SetPipelineState(mPSOs["Selected"].Get());
		mCommandList->SetGraphicsRootConstantBufferView(0,
			mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
			+ mSelectedModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
		mSelectedModel->Draw();
	}
	
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	mCommandList->SetGraphicsRootSignature(mRootSignatures["planet"].Get());
	mCommandList->SetPipelineState(mPSOs["planet"].Get());
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		mCommandList->SetGraphicsRootConstantBufferView(0,
		mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
			+ model->second->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

		//D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
		//handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * model->second->mObjIndex;
		//handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 50;
		mCommandList->SetGraphicsRootDescriptorTable(2, mDescriptorManager->GetGpuHandle(mNoiseMapDescriptorIdx,DescType::SRV));
		model->second->Draw();
	}
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	if (mScenes[mCurrentScene]->mModels->count("inventory") != 0)
	{
		Inventory* invtry = static_cast<Inventory*>(mScenes[mCurrentScene]->mModels->at("inventory").get());
		for (auto inventory = invtry->mInventory.begin(); inventory != invtry->mInventory.end(); inventory++)
		{
			mCommandList->SetGraphicsRootConstantBufferView(0,
				mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
				+ mScenes[mCurrentScene]->mModels->at("inventory")->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
			//D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
			//handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * inventory->second->mObjIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, mDescriptorManager->GetGpuHandle(mScenes[mCurrentScene]->mSrvIndices[inventory->first], DescType::SRV));
			inventory->second->Draw();
		}
	}

	heaps[0] = { mScenes[mCurrentScene]->mVolumeUavHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	mCommandList->SetGraphicsRootSignature(mRootSignatures["Volume"].Get());
	mCommandList->IASetVertexBuffers(0,0,nullptr);
	mCommandList->IASetIndexBuffer(nullptr);
	int i = 0;
	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		if (i == 0)
		{
			mCommandList->SetPipelineState(mPSOs["VolumeCube"].Get());
			D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, handle);
			mCommandList->SetGraphicsRootConstantBufferView(0,
				mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
				+ volume->second->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

			volume->second->Draw();
		}
		else
		{
			mCommandList->SetPipelineState(mPSOs["VolumeSphere"].Get());
			D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, handle);
			mCommandList->SetGraphicsRootConstantBufferView(0,
				mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
				+ volume->second->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

			volume->second->Draw();
		}
		++i;
	}

	for (auto rigid = RigidBodySystem::mRigidBodies.begin(); rigid != RigidBodySystem::mRigidBodies.end(); rigid++)
	{
		mCommandList->SetGraphicsRootConstantBufferView(0,
			mResourceManager->GetResource(mFrames[mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
			+ (*rigid)->mModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

		mCommandList->SetGraphicsRootConstantBufferView(1,
			mResourceManager->GetResource(mFrames[mCurrentFrame]->mEnvConstantBufferIdx)->GetGPUVirtualAddress());
	}

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

void Engine::Input()
{
	float deltaTime = mTimer.GetDeltaTime();
	bool dirty = false;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		dirty = true;
		mScenes[mCurrentScene]->mCamera->GoFront(10.0f * deltaTime);
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		dirty = true;
		mScenes[mCurrentScene]->mCamera->GoFront(-10.0f * deltaTime);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		dirty = true;
		mScenes[mCurrentScene]->mCamera->GoRight(10.0f * deltaTime);
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		dirty = true;
		mScenes[mCurrentScene]->mCamera->GoRight(-10.0f * deltaTime);
	}
	
	if (dirty) {
		mScenes[mCurrentScene]->mCamera->UpdateView();
		mScenes[mCurrentScene]->mCamera->UpdateInvViewProjection();
	}
}

void Engine::CreateNoiseMap()
{
	mNoiseMap = make_unique<TextureResource>();
	
	const double pi = 3.14159265358979;

	float perlinArray[128][512];
	unsigned int tableMask = 127;

	mt19937 generator(2020);
	uniform_real_distribution<float> dist;
	
	for (int i = 0; i < 128; ++i)
	{
		for (int j = 0; j < 128; ++j)
		{
			//float theta = acos(2.0f * dist(generator) - 1.0f);
			float phi = 2.0f * dist(generator) * pi;

			float x = cos(phi);
			float y = sin(phi);
			//float z = cos(theta);

			perlinArray[j][4 * i]= x;
			perlinArray[j][4 * i + 1] = y;
			perlinArray[j][4 * i + 2] = 0;
			perlinArray[j][4 * i + 3] = 0;
		}
	}

	mNoiseMap->CopyCreate(perlinArray, 128, 128, 16, DXGI_FORMAT_R32G32B32A32_FLOAT);


	mNoiseMapDescriptorIdx =  mDescriptorManager->CreateSrv(mNoiseMap->mTexture.Get(),
		DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_SRV_DIMENSION_TEXTURE2D);
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

	mMsaaQuality = qualityLevels.NumQualityLevels;

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

	IfError::Throw(D3DCompileFromFile(L"CreateParticles.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["CreateParticlesCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"particlePosition.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["particlePositionCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"particleVelocity.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["particleVelocityCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"RigidInertia.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["RigidInertiaCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"GridShader.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["GridShaderCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"Collision.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["CollisionCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"RigidMomentum.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["RigidMomentumCS"] = move(blob);

	IfError::Throw(D3DCompileFromFile(L"RigidPosQuat.hlsl", nullptr, nullptr, "CS", "cs_5_1", 0, 0, &blob, nullptr),
		L"compile shader error!");
	mShaders["RigidPosQuatCS"] = move(blob);

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
		L"compile shader error4!");
	mShaders["planetPS"] = move(blob);

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
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[2].Descriptor.RegisterSpace = 0;
	rootParameter[2].Descriptor.ShaderRegister = 1;

	rsDesc.NumParameters = 3;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["DepthPeeling"] = move(rs);


	D3D12_DESCRIPTOR_RANGE rangeCompute[3];
	rangeCompute[0].BaseShaderRegister = 0;
	rangeCompute[0].NumDescriptors = 1;
	rangeCompute[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeCompute[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeCompute[0].RegisterSpace = 0;
	rangeCompute[1].BaseShaderRegister = 0;
	rangeCompute[1].NumDescriptors = 15;
	rangeCompute[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangeCompute[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	rangeCompute[1].RegisterSpace = 0;

	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[0].DescriptorTable.NumDescriptorRanges = 2;
	rootParameter[0].DescriptorTable.pDescriptorRanges = rangeCompute;
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].Constants.Num32BitValues = 2;
	rootParameter[1].Constants.RegisterSpace = 0;
	rootParameter[1].Constants.ShaderRegister = 0;

	rsDesc.NumParameters = 2;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["CreateParticles"] = move(rs);


	D3D12_DESCRIPTOR_RANGE rangePlanet[1];
	rangePlanet[0].BaseShaderRegister = 0;
	rangePlanet[0].NumDescriptors = 1;
	rangePlanet[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rangePlanet[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangePlanet[0].RegisterSpace = 0;

	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[0].Descriptor.RegisterSpace = 0;
	rootParameter[0].Descriptor.ShaderRegister = 0;
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].Descriptor.RegisterSpace = 0;
	rootParameter[1].Descriptor.ShaderRegister = 1;
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //구체적으로 지정해서 최적화할 여지있음.
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[2].DescriptorTable.pDescriptorRanges = rangePlanet;

	rsDesc.NumParameters = 3;
	rsDesc.pParameters = rootParameter;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	IfError::Throw(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), nullptr),
		L"serialize root signature error!");
	IfError::Throw(mDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(rs.GetAddressOf())),
		L"create root signature error!");
	mRootSignatures["planet"] = move(rs);

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


	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	computePsoDesc.CachedPSO.pCachedBlob = nullptr;
	computePsoDesc.CS.BytecodeLength = mShaders["CreateParticlesCS"]->GetBufferSize();
	computePsoDesc.CS.pShaderBytecode = mShaders["CreateParticlesCS"]->GetBufferPointer();
	computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	computePsoDesc.NodeMask = 0;
	computePsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["CreateParticles"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC particlePosComputePsoDesc = {};
	particlePosComputePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	particlePosComputePsoDesc.CachedPSO.pCachedBlob = nullptr;
	particlePosComputePsoDesc.CS.BytecodeLength = mShaders["particlePositionCS"]->GetBufferSize();
	particlePosComputePsoDesc.CS.pShaderBytecode = mShaders["particlePositionCS"]->GetBufferPointer();
	particlePosComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	particlePosComputePsoDesc.NodeMask = 0;
	particlePosComputePsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&particlePosComputePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["particlePosition"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC particleVelComputePsoDesc = {};
	particleVelComputePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	particleVelComputePsoDesc.CachedPSO.pCachedBlob = nullptr;
	particleVelComputePsoDesc.CS.BytecodeLength = mShaders["particleVelocityCS"]->GetBufferSize();
	particleVelComputePsoDesc.CS.pShaderBytecode = mShaders["particleVelocityCS"]->GetBufferPointer();
	particleVelComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	particleVelComputePsoDesc.NodeMask = 0;
	particleVelComputePsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&particleVelComputePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["particleVelocity"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC rigidInertiaComputePsoDesc = {};
	rigidInertiaComputePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	rigidInertiaComputePsoDesc.CachedPSO.pCachedBlob = nullptr;
	rigidInertiaComputePsoDesc.CS.BytecodeLength = mShaders["RigidInertiaCS"]->GetBufferSize();
	rigidInertiaComputePsoDesc.CS.pShaderBytecode = mShaders["RigidInertiaCS"]->GetBufferPointer();
	rigidInertiaComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	rigidInertiaComputePsoDesc.NodeMask = 0;
	rigidInertiaComputePsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&rigidInertiaComputePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["RigidInertia"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC gridComputePsoDesc = {};
	gridComputePsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	gridComputePsoDesc.CachedPSO.pCachedBlob = nullptr;
	gridComputePsoDesc.CS.BytecodeLength = mShaders["GridShaderCS"]->GetBufferSize();
	gridComputePsoDesc.CS.pShaderBytecode = mShaders["GridShaderCS"]->GetBufferPointer();
	gridComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gridComputePsoDesc.NodeMask = 0;
	gridComputePsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&gridComputePsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["GridShader"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC collisionPsoDesc = {};
	collisionPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	collisionPsoDesc.CachedPSO.pCachedBlob = nullptr;
	collisionPsoDesc.CS.BytecodeLength = mShaders["CollisionCS"]->GetBufferSize();
	collisionPsoDesc.CS.pShaderBytecode = mShaders["CollisionCS"]->GetBufferPointer();
	collisionPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	collisionPsoDesc.NodeMask = 0;
	collisionPsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&collisionPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["Collision"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC rigidMomentumPsoDesc = {};
	rigidMomentumPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	rigidMomentumPsoDesc.CachedPSO.pCachedBlob = nullptr;
	rigidMomentumPsoDesc.CS.BytecodeLength = mShaders["RigidMomentumCS"]->GetBufferSize();
	rigidMomentumPsoDesc.CS.pShaderBytecode = mShaders["RigidMomentumCS"]->GetBufferPointer();
	rigidMomentumPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	rigidMomentumPsoDesc.NodeMask = 0;
	rigidMomentumPsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&rigidMomentumPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["RigidMomentum"] = move(pso);

	D3D12_COMPUTE_PIPELINE_STATE_DESC rigidPosQuatPsoDesc = {};
	rigidPosQuatPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	rigidPosQuatPsoDesc.CachedPSO.pCachedBlob = nullptr;
	rigidPosQuatPsoDesc.CS.BytecodeLength = mShaders["RigidPosQuatCS"]->GetBufferSize();
	rigidPosQuatPsoDesc.CS.pShaderBytecode = mShaders["RigidPosQuatCS"]->GetBufferPointer();
	rigidPosQuatPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	rigidPosQuatPsoDesc.NodeMask = 0;
	rigidPosQuatPsoDesc.pRootSignature = mRootSignatures["CreateParticles"].Get();
	IfError::Throw(mDevice->CreateComputePipelineState(&rigidPosQuatPsoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["RigidPosQuat"] = move(pso);


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
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	IfError::Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())),
		L"create graphics pso error!");
	mPSOs["planet"] = move(pso);

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