#include "Game.h"

vector<unique_ptr<Scene>> Game::mScenes;
ComPtr<ID3D12GraphicsCommandList> Game::mCommandList;
int Game::mCurrentScene;

Game::Game(HINSTANCE hInstance)
	: mDirectX(mWidth,mHeight), mInstance(hInstance)
{
	mLatestWindow = this;
}

void Game::Initialize()
{
	//윈도우 초기화
	InitializeWindow();

	DebugEnable();

	//device, fence 등 생성
	mDirectX.Initialize();
	

	CreateFrames(MAX_OBJECT);

	CreateCommandObjects();

	//DirectX 객체들 생성 (swapchain, depth buffer, root signature, shader 등)
	mDirectX.CreateObjects(mWindowHandle);

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

	IfError::Throw(mCommandList->Close(),
		L"command list close error!");

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mDirectX.mCommandQueue->ExecuteCommandLists(1, lists);

	mTimer.Reset();
}

void Game::Run()
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

void Game::DebugEnable()
{
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
}

void Game::ChangeScene(int dstScene)
{
	mCurrentScene = dstScene;
}

void Game::SelectObject(int x, int y)
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

void Game::MoveObject(int x, int y)
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

void Game::SelectInventory(int x, int y)
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

void Game::WaitUntilPrevFrameComplete()
{
	auto waitFrame = mFrames[(mCurrentFrame + mNumberOfFrames -1)%mNumberOfFrames].get();
	if (waitFrame->mFenceValue != 0 && mDirectX.mFence->GetCompletedValue() < waitFrame->mFenceValue)
	{
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		IfError::Throw(mDirectX.mFence->SetEventOnCompletion(waitFrame->mFenceValue, event),
			L"set event on completion error!");
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}


LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Game::Get()->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

void Game::InitializeWindow()
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

Game* Game::mLatestWindow = nullptr;

Game* Game::Get()
{
	return mLatestWindow;
}


void Game::LoadScene()
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

void Game::CreateFrames(int numObjConstant)
{
	//효율성을 위해 cpu에서 미리 프레임을 계산해 놓기위해서 여러개의 프레임 자원을 생성.
	for (int i = 0; i < mNumberOfFrames; ++i)
	{
		mFrames.push_back(make_unique<Frame>(numObjConstant));
	}
}

void Game::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	IfError::Throw(Pipeline::mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mDirectX.mCommandQueue.GetAddressOf())),
		L"create command queue error!");

	IfError::Throw(Pipeline::mDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrames[mCurrentFrame]->Get(),
		nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())),
		L"create command list error!");
}


/*
* model의 vertex,index offset이 scene 내에서 존재하기 buffer가 존재하기 때문에 어떤 scene내에 로드할 건지를 명시해준다.
*/
unique_ptr<Clickables> Game::CreateModel(int sceneIndex)
{
	unique_ptr<Clickables> model = make_unique<Clickables>();

	shared_ptr<Clickable> m;

	if (sceneIndex == 0)
	{
		m = make_shared<Clickable>("../Model/ball.obj", L"../Model/textures/bricks1.dds");
		m->SetPosition(5.0f, 3.0f, 3.0f);
		(*model)["table"] = move(m);

		m = make_shared<Clickable>("../Model/sword.obj", L"../Model/textures/bricks2.dds");
		m->SetPosition(2.0f, 0.0f, 0.0f);
		m->mScale = { 0.1f,0.1f,0.1f };
		(*model)["sword"] = move(m);

		m = make_shared<Clickable>("../Model/my.obj", L"../Model/textures/checkboard.dds");
		(*model)["my"] = move(m);

		shared_ptr<Button> b = make_shared<Button>("../Model/inventory.obj", L"../Model/textures/earth.dds");
		b->SetPosition(0.0f, -3.0f, 5.0f);
		b->Set([&]() {
			ChangeScene(1);
		});
		(*model)["button"] = move(b);
	}
	else if (sceneIndex == 1)
	{
		m = make_shared<Clickable>("../Model/KSR-29 sniper rifle new_obj.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(0.0f, 0.2f, 0.0f);
		m->mScale = { 0.5f,0.5f,0.5f };
		(*model)["rifle"] = move(m);

		m = make_shared<Clickable>("../Model/ball.obj", L"../Model/textures/earth.dds");
		m->mScale = { 1.0f,1.0f,1.0f };
		(*model)["earth"] = move(m);

		m = make_shared<Clickable>("../Model/box.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(0.0f, -5.0f, 0.0f);
		m->mScale = { 15.0f,0.5f,15.0f };
		(*model)["lamp"] = move(m);

		m = make_shared<Inventory>("../Model/inventory.obj", L"../Model/textures/inventory.dds");
		m->SetPosition(0.0f, -1.5f, 5.0f);
		(*model)["inventory"] = move(m);
		
		shared_ptr<Button> b = make_shared<Button>("../Model/inventory.obj", L"../Model/textures/earth.dds");
		b->SetPosition(0.0f, -3.0f, 5.0f);
		b->Set([&](){
			ChangeScene(0);
		});
		(*model)["button"] = move(b);

	}

	return move(model);
}



void Game::LoadCopyModelToBuffer()
{
	//모델 데이터, 텍스처 로드, 버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
	for (int i = 0; i <= mCurrentScene; ++i)
	{
		mScenes[i]->Load();
	}
}

unique_ptr<Unclickables> Game::CreateWorld(int sceneIndex)
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

unique_ptr<Volumes> Game::CreateVolume(int sceneIndex)
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

trans Game::SetLight()
{
	trans env;
		
	env.lights[0].mPosition = { 10.0f,0.0f,0.0f };
	env.lights[0].mDirection = { -1.0f,0.0f,0.0f };
	env.lights[0].mColor = {1.0f,1.0f,1.0f };
	env.lights[0].mType = Point;
	env.lights[1].mPosition = { 9.0f,3.0f,0.0f };
	env.lights[1].mDirection = { 0.0f,-1.0f,0.0f };
	env.lights[1].mColor = { 1.0f,1.0f,0.0f };
	env.lights[1].mType = Spot;
	env.lights[2].mPosition = { 0.0f, 3.0f,0.0f };
	env.lights[2].mDirection = { 0.0f,-1.0f,0.0f };
	env.lights[2].mColor = { 1.0f,1.0f,0.0f };
	env.lights[2].mType = Spot;

	return env;
}

void Game::SetObjConstantIndex(int index)
{
	mCommandList->SetGraphicsRootConstantBufferView(0, mFrames[mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
		+ index * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
}

void Game::Update()
{
	//현재 프레임이 gpu에서 전부 draw되지 않았을 시 기다리고, 완료된 경우에는 다음 frame으로 넘어가는 역할.
	mCurrentFrame = (mCurrentFrame + 1) % mNumberOfFrames;

	auto currentFrame = mFrames[mCurrentFrame].get();

	if (currentFrame->mFenceValue != 0 && mDirectX.mFence->GetCompletedValue() < currentFrame->mFenceValue)
	{
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		IfError::Throw(mDirectX.mFence->SetEventOnCompletion(currentFrame->mFenceValue, event),
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
	mFrames[mCurrentFrame]->CopyTransConstantBuffer(0, &mScenes[mCurrentScene]->envFeature, sizeof(trans));

	//각 모델별로 obj constant를 constant buffer의 해당위치에 로드함.
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		mFrames[mCurrentFrame]->CopyObjConstantBuffer(model->second->mObjIndex, &model->second->mObjFeature, sizeof(obj));
	}

	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		mFrames[mCurrentFrame]->CopyObjConstantBuffer(world->second->mObjIndex, &world->second->mObjFeature, sizeof(obj));
	}

	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		mFrames[mCurrentFrame]->CopyObjConstantBuffer(volume->second->mObjIndex, &volume->second->mObjFeature, sizeof(obj));
	}
	
	mParticleField->Update(mTimer);
}

void Game::Draw()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mDirectX.mBackBuffers[mDirectX.mCurrentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	IfError::Throw(mFrames[mCurrentFrame]->Get()->Reset(),
		L"frame command allocator reset error!");

	mCommandList->Reset(mFrames[mCurrentFrame]->Get(), mDirectX.mPSOs["default"].Get());

	mScenes[mCurrentScene]->Spawn();

	if (!mScenes[mCurrentScene]->IsDestroyQueueEmpty())
	{
		WaitUntilPrevFrameComplete();
		mScenes[mCurrentScene]->Destroy();
	}

	mCommandList->RSSetScissorRects(1, &mDirectX.mScissor);
	mCommandList->RSSetViewports(1, &mDirectX.mViewport);

	mCommandList->ResourceBarrier(1, &barrier);

	//mRigidBodySystem->DepthPass(RigidBodySystem::mRigidBodies[8]);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mDirectX.mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDirectX.mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * mDirectX.mCurrentBackBuffer;
	float rgba[4] = { 0.0f,0.1f,0.0f,1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, rgba, 0, nullptr);
	mCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	mCommandList->SetGraphicsRootSignature(mDirectX.mRootSignatures["Default"].Get());

	mCommandList->SetGraphicsRootConstantBufferView(1, mFrames[mCurrentFrame]->mTransConstantBuffer->GetGpuAddress());


	//particle density update
	ID3D12DescriptorHeap* heaps[] = { mScenes[mCurrentScene]->mVolumeUavHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	mCommandList->SetPipelineState(mDirectX.mPSOs["Particle"].Get());
	mCommandList->SetGraphicsRootSignature(mDirectX.mRootSignatures["Particle"].Get());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	mCommandList->IASetVertexBuffers(0, 1, mParticleField->GetVertexBufferView());
	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += volume->second->mVolumeIndex * Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mScenes[mCurrentScene]->mVolumeUavHeapInvisible->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += volume->second->mVolumeIndex * Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		UINT color[4] = { 0,0,0,0 };
		mCommandList->ClearUnorderedAccessViewUint(gpuHandle,cpuHandle, volume->second->mTextureResource->mTexture.Get(), color, 0, nullptr);
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
		mCommandList->SetGraphicsRootDescriptorTable(0, handle);
		mCommandList->DrawInstanced(mParticleField->NumParticle(),1,0,0);
	}
	//

	mCommandList->SetGraphicsRootSignature(mDirectX.mRootSignatures["Default"].Get());

	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	heaps[0] = { mScenes[mCurrentScene]->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	mCommandList->SetPipelineState(mDirectX.mPSOs["World"].Get());
	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * world->second->mObjIndex;
		mCommandList->SetGraphicsRootDescriptorTable(2, handle);
		world->second->Draw();
	}

	//선택된 물체에 노란색 테두리 렌더링
	if (mIsModelSelected == true)
	{
		mCommandList->SetPipelineState(mDirectX.mPSOs["Selected"].Get());
		Game::mCommandList->SetGraphicsRootConstantBufferView(0, Game::mFrames[Game::mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
			+ mSelectedModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
		mSelectedModel->Draw();
	}
	
	mCommandList->SetPipelineState(mDirectX.mPSOs["Default"].Get());
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		Game::mCommandList->SetGraphicsRootConstantBufferView(0, Game::mFrames[Game::mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
			+ model->second->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

		D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * model->second->mObjIndex;
		mCommandList->SetGraphicsRootDescriptorTable(2, handle);
		model->second->Draw();
	}

	if (mScenes[mCurrentScene]->mModels->count("inventory") != 0)
	{
		Inventory* invtry = static_cast<Inventory*>(mScenes[mCurrentScene]->mModels->at("inventory").get());
		for (auto inventory = invtry->mInventory.begin(); inventory != invtry->mInventory.end(); inventory++)
		{
			SetObjConstantIndex(mScenes[mCurrentScene]->mModels->at("inventory")->mObjIndex);
			D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mSrvHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * inventory->second->mObjIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, handle);
			inventory->second->Draw();
		}
	}

	heaps[0] = { mScenes[mCurrentScene]->mVolumeUavHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	mCommandList->SetGraphicsRootSignature(mDirectX.mRootSignatures["Volume"].Get());
	mCommandList->IASetVertexBuffers(0,0,nullptr);
	mCommandList->IASetIndexBuffer(nullptr);
	int i = 0;
	for (auto volume = mScenes[mCurrentScene]->mVolume->begin(); volume != mScenes[mCurrentScene]->mVolume->end(); volume++)
	{
		if (i == 0)
		{
			mCommandList->SetPipelineState(mDirectX.mPSOs["VolumeCube"].Get());
			D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, handle);
			SetObjConstantIndex(volume->second->mObjIndex);
			volume->second->Draw();
		}
		else
		{
			mCommandList->SetPipelineState(mDirectX.mPSOs["VolumeSphere"].Get());
			D3D12_GPU_DESCRIPTOR_HANDLE handle = mScenes[mCurrentScene]->mVolumeUavHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * volume->second->mVolumeIndex;
			mCommandList->SetGraphicsRootDescriptorTable(2, handle);
			SetObjConstantIndex(volume->second->mObjIndex);
			volume->second->Draw();
		}
		++i;
	}

	for (auto rigid = RigidBodySystem::mRigidBodies.begin(); rigid != RigidBodySystem::mRigidBodies.end(); rigid++)
	{
		Game::mCommandList->SetGraphicsRootConstantBufferView(0, mFrames[mCurrentFrame]->mObjConstantBuffer->GetGpuAddress()
			+ (*rigid)->mModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
		Game::mCommandList->SetGraphicsRootConstantBufferView(1, mFrames[mCurrentFrame]->mTransConstantBuffer->GetGpuAddress());
		(*rigid)->DrawParticles();
	}

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = mDirectX.mBackBuffers[mDirectX.mCurrentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCommandList->ResourceBarrier(1, &barrier);

	IfError::Throw(mCommandList->Close(),
		L"command list close error!");

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mDirectX.mCommandQueue->ExecuteCommandLists(1, lists);

	IfError::Throw(mDirectX.mSwapChain->Present(0, 0),
		L"swap chain present error!");

	mDirectX.mCurrentBackBuffer = (mDirectX.mCurrentBackBuffer + 1) % 2;
	mFrames[mCurrentFrame]->mFenceValue = ++mDirectX.mFenceValue;
	mDirectX.mCommandQueue->Signal(mDirectX.mFence.Get(), mDirectX.mFenceValue);
}

void Game::Input()
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