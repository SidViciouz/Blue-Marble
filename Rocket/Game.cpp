#include "Game.h"

vector<unique_ptr<Scene>> Game::mScenes;

Game::Game(HINSTANCE hInstance)
	: mDirectX(mWidth,mHeight)
{
	mLatestWindow = this;
}

void Game::Initialize()
{
	//윈도우 초기화
	InitializeWindow();

	//device, fence 등 생성
	mDirectX.Initialize();
	
	//각 Scene들에 모델, 카메라, 조명 생성
	int numModels = LoadScene();

	//DirectX 객체들 생성 (Frame, swapchain, depth buffer, command objects, root signature, shader 등)
	mDirectX.CreateObjects(mWindowHandle, numModels);

	//commandList가 필요하기 때문에 texture load를 여기에서 한다.
	//commandList가 필요하기 때문에 DirectX objects 생성 후에 model을 buffer에 복사한다.
	LoadCopyModelToBuffer();

	//texture가 로드된 후에 srv를 생성할 수 있기 때문에 다른 오브젝트들과 따로 생성한다.
	mDirectX.CreateSrv(numModels + 2);

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
		mIsModelSelected = false;
		return 0;

	case WM_MOUSEMOVE:
		if (mIsModelSelected == true)
		{
			MoveObject(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_KEYDOWN :
		if (wParam == 0x51)
			ChangeScene(0);
		else if (wParam == 0x45)
			ChangeScene(1);
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

int Game::LoadScene()
{
	int numModels = 0;

	//scene 0
	mScenes.push_back(make_unique<Scene>());

	//모델 로드 (버텍스, 인덱스)
	mScenes[mCurrentScene]->mModels = CreateModel(0);

	mScenes[mCurrentScene]->mWorld = CreateWorld(0);

	numModels += mScenes[mCurrentScene]->mModels->size();

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();
	

	//scene 1
	mCurrentScene++;
	mScenes.push_back(make_unique<Scene>());
	
	//모델 로드 (버텍스, 인덱스)
	mScenes[mCurrentScene]->mModels = CreateModel(1);

	mScenes[mCurrentScene]->mWorld = CreateWorld(1);

	numModels += mScenes[mCurrentScene]->mModels->size();

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();

	return numModels;
}

/*
* model의 vertex,index offset이 scene 내에서 존재하기 buffer가 존재하기 때문에 어떤 scene내에 로드할 건지를 명시해준다.
*/
unique_ptr<Models> Game::CreateModel(int sceneIndex)
{
	unique_ptr<Models> model = make_unique<Models>();

	shared_ptr<Model> m;

	if (sceneIndex == 0)
	{
		m = make_shared<Model>(sceneIndex,"../Model/table.obj", L"../Model/textures/bricks1.dds");
		(*model)["table"] = move(m);

		m = make_shared<Model>(sceneIndex,"../Model/sword.obj", L"../Model/textures/bricks2.dds");
		m->mScale = { 0.1f,0.1f,0.1f };
		(*model)["sword"] = move(m);

		m = make_shared<Model>(sceneIndex,"../Model/my.obj", L"../Model/textures/checkboard.dds");
		(*model)["my"] = move(m);
	}
	else if (sceneIndex == 1)
	{
		m = make_shared<Model>(sceneIndex,"../Model/KSR-29 sniper rifle new_obj.obj", L"../Model/textures/bricks3.dds");
		m->SetPosition(0.0f, 0.2f, 0.0f);
		m->mScale = { 0.5f,0.5f,0.5f };
		(*model)["woodHouse"] = move(m);
	}

	return move(model);
}



void Game::LoadCopyModelToBuffer()
{
	//모델 데이터, 텍스처 로드, 버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
	for (int i = 0; i <= mCurrentScene; ++i)
	{
		mScenes[i]->LoadModels();
		mScenes[i]->CreateVertexIndexBuffer();
	}
}

unique_ptr<Models> Game::CreateWorld(int sceneIndex)
{
	unique_ptr<Models> model = make_unique<Models>();

	shared_ptr<Model> m;

	if (sceneIndex == 0)
	{
		m = make_shared<Model>(sceneIndex, "../Model/ball.obj", L"../Model/textures/sky.dds");
		m->mScale = { 100.0f,100.0f,100.0f };
		(*model)["sky"] = move(m);
	}
	else if (sceneIndex == 1)
	{
		m = make_shared<Model>(sceneIndex, "../Model/ball.obj", L"../Model/textures/sky.dds");
		m->mScale = { 100.0f,100.0f,100.0f };
		(*model)["sky"] = move(m);
	}

	return move(model);
}

trans Game::SetLight()
{
	trans env;
		
	env.lights[0].mPosition = { 10.0f,10.0f,-15.0f };
	env.lights[0].mDirection = { 1.0f,-1.0f,0.0f };
	env.lights[0].mColor = { 1.0f,1.0f,1.0f };
	env.lights[0].mType = Directional;
	env.lights[1].mPosition = { 10.0f,10.0f,-15.0f };
	env.lights[1].mDirection = { 1.0f,-1.0f,0.0f };
	env.lights[1].mColor = { 1.0f,1.0f,1.0f };
	env.lights[1].mType = Point;
	env.lights[2].mPosition = { 0.0f,0.0f,-1.0f };
	env.lights[2].mDirection = { 0.0f,0.0f,1.0f };
	env.lights[2].mType = Directional;

	return env;
}

void Game::Update()
{
	//현재 프레임이 gpu에서 전부 draw되지 않았을 시 기다리고, 완료된 경우에는 다음 frame으로 넘어가는 역할.
	mDirectX.Update();

	//실제 게임 데이터의 업데이트는 여기서부터 일어난다.
	Input();
	
	mScenes[mCurrentScene]->Update();
	mScenes[mCurrentScene]->envFeature.view = mScenes[mCurrentScene]->mCamera->view;
	mScenes[mCurrentScene]->envFeature.projection = mScenes[mCurrentScene]->mCamera->projection;
	mScenes[mCurrentScene]->envFeature.cameraPosition = mScenes[mCurrentScene]->mCamera->GetPosition();
	mScenes[mCurrentScene]->envFeature.cameraFront = mScenes[mCurrentScene]->mCamera->mFront;
	mDirectX.SetTransConstantBuffer(0, &mScenes[mCurrentScene]->envFeature, sizeof(trans));

	//각 모델별로 obj constant를 constant buffer의 해당위치에 로드함.
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		mDirectX.SetObjConstantBuffer(model->second->mObjIndex, &model->second->mObjFeature, sizeof(obj));
	}

	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		mDirectX.SetObjConstantBuffer(world->second->mObjIndex, &world->second->mObjFeature, sizeof(obj));
	}
	
}

void Game::Draw()
{
	mDirectX.Draw();

	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = mScenes[mCurrentScene]->mVertexBuffer->GetGpuAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(Vertex)* mScenes[mCurrentScene]->mAllVertices.size();

	D3D12_INDEX_BUFFER_VIEW ibv = {};
	ibv.BufferLocation = mScenes[mCurrentScene]->mIndexBuffer->GetGpuAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = sizeof(uint16_t)* mScenes[mCurrentScene]->mAllIndices.size();

	//vertex buffer, index buffer 바인딩.
	ID3D12GraphicsCommandList* cmdList = Pipeline::mCommandList.Get();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);

	ID3D12DescriptorHeap* heaps[] = { mDirectX.getSrvHeap() };
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);


	//선택된 물체에 노란색 테두리 렌더링
	if (mIsModelSelected == true)
	{
		mDirectX.SetPSO("Selected");
		mDirectX.SetObjConstantIndex(mSelectedModel->mObjIndex);
		cmdList->DrawIndexedInstanced(mSelectedModel->mIndexBufferSize, 1, mSelectedModel->mIndexBufferOffset, mSelectedModel->mVertexBufferOffset, 0);
		mDirectX.SetPSO("default");
	}

	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		mDirectX.SetObjConstantIndex(model->second->mObjIndex);
		mDirectX.SetSrvIndex(model->second->mObjIndex);
		cmdList->DrawIndexedInstanced(model->second->mIndexBufferSize, 1, model->second->mIndexBufferOffset, model->second->mVertexBufferOffset, 0);
	}

	for (auto world = mScenes[mCurrentScene]->mWorld->begin(); world != mScenes[mCurrentScene]->mWorld->end(); world++)
	{
		mDirectX.SetObjConstantIndex(world->second->mObjIndex);
		mDirectX.SetSrvIndex(world->second->mObjIndex);
		cmdList->DrawIndexedInstanced(world->second->mIndexBufferSize, 1, world->second->mIndexBufferOffset, world->second->mVertexBufferOffset, 0);
	}

	mDirectX.TransitionToPresent();
	mDirectX.CloseAndExecute();

	mDirectX.DrawFinish();
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
	
	if (dirty)
		mScenes[mCurrentScene]->mCamera->UpdateView();
}