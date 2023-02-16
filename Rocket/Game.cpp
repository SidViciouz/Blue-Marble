#include "Game.h"

Game::Game(HINSTANCE hInstance)
	: mDirectX(mWidth,mHeight), mCamera(make_unique<Camera>(mWidth,mHeight))
{
	mLatestWindow = this;
}

void Game::Initialize()
{
	//윈도우 초기화
	InitializeWindow();

	//DirectX 객체들 초기화 (Frame, swapchain, depth buffer, command objects, root signature, shader 등)
	mDirectX.Initialize(mWindowHandle);
	
	//모델 로드 (버텍스, 인덱스)
	LoadModel();

	//버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
	CreateVertexIndexBuffer();

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

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Game::Get()->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//다음과 같은 카메라 움직임 업데이트는 버벅임이 생긴다.
	/*
	switch (msg)
	{
	case WM_KEYDOWN :
		if(wParam == 0x57)
			mCamera->GoFront(10.0f * mTimer.GetDeltaTime());
		else if(wParam == 0x53)
			mCamera->GoFront(-10.0f * mTimer.GetDeltaTime());
		else if (wParam == 0x41)
			mCamera->GoRight(-10.0f*mTimer.GetDeltaTime());
		else if(wParam == 0x44)
			mCamera->GoRight(10.0f * mTimer.GetDeltaTime());
	}
	*/
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

void Game::LoadModel()
{
	unique_ptr<Model> table = make_unique<Model>(mDirectX.GetDevice(), "../Model/Wood_Table.obj", mDirectX.GetCommandList());
	mModels["table"] = move(table);

	unique_ptr<Model> house = make_unique<Model>(mDirectX.GetDevice(), "../Model/triangle.obj", mDirectX.GetCommandList());
	mModels["triangle"] = move(house);

	unique_ptr<Model> woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/WoodHouse.obj", mDirectX.GetCommandList());
	mModels["woodHouse"] = move(woodHouse);//frame에서 obj constant buffer 크기 늘려야함.
}

void Game::CreateVertexIndexBuffer()
{
	//make_move_iterator로 바꿔서 move해야 성능이 올라감.
	for (auto model = mModels.begin(); model != mModels.end(); model++)
	{
		allVertices.insert(allVertices.end(), model->second->mVertices.begin(), model->second->mVertices.end());
		allIndices.insert(allIndices.end(), model->second->mIndices.begin(), model->second->mIndices.end());
	}

	//Vertex, Index 버퍼 생성
	mVertexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(Vertex) * allVertices.size());
	mIndexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(uint16_t) * allIndices.size());

	//Vertex, Index 버퍼에 Model 데이터 copy
	mVertexBuffer->Copy(allVertices.data(), sizeof(Vertex) * allVertices.size(), mDirectX.GetCommandList());
	mIndexBuffer->Copy(allIndices.data(), sizeof(uint16_t) * allIndices.size(), mDirectX.GetCommandList());
}

void Game::Update()
{
	//현재 프레임이 gpu에서 전부 draw되지 않았을 시 기다리고, 완료된 경우에는 다음 frame으로 넘어가는 역할.
	mDirectX.Update();

	//실제 게임 데이터의 업데이트는 여기서부터 일어난다.
	Input();

	//Model의 position으로부터 world, Camera의 데이터로부터 view와 projection matrix를 설정한다.
	//model의 world matrix를 업데이트
	for (auto it = mModels.begin(); it != mModels.end(); it++)
	{
		XMFLOAT3 pos = it->second->mPosition;
		XMMATRIX world = XMMatrixScaling(0.5f, 0.5f, 0.5f)*XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mWorld, world);
	}

	XMVECTOR right = XMLoadFloat3(&mCamera->mRight);
	XMVECTOR up = XMLoadFloat3(&mCamera->mUp);
	XMVECTOR front = XMVector3Normalize(XMLoadFloat3(&mCamera->mFront));
	XMVECTOR position = XMLoadFloat3(&mCamera->mPosition);

	up = XMVector3Normalize(XMVector3Cross(front, right));
	right = XMVector3Cross(up, front);

	XMStoreFloat3(&mCamera->mRight, right);
	XMStoreFloat3(&mCamera->mUp, up);
	XMStoreFloat3(&mCamera->mFront,front);

	float x = -XMVectorGetX(XMVector3Dot(position, right));
	float y = -XMVectorGetX(XMVector3Dot(position, up));
	float z = -XMVectorGetX(XMVector3Dot(position, front));

	XMFLOAT4X4 viewMatrix = {
		mCamera->mRight.x, mCamera->mUp.x, mCamera->mFront.x, 0.0f,
		mCamera->mRight.y, mCamera->mUp.y, mCamera->mFront.y, 0.0f,
		mCamera->mRight.z, mCamera->mUp.z, mCamera->mFront.z, 0.0f,
		x,y,z,1.0f
	};

	XMMATRIX view = XMLoadFloat4x4(&viewMatrix);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(mCamera->mAngle, mCamera->mRatio, mCamera->mNear, mCamera->mFar);

	XMStoreFloat4x4(&mCamera->mViewProjection,XMMatrixMultiply(view, projection));
	
	//각 모델별로 obj constant를 constant buffer의 해당위치에 로드함.

	int i = 0;
	for (auto model = mModels.begin(); model != mModels.end(); model++, ++i) {
		obj objFeature = {};
		objFeature.world = model->second->mWorld;
		objFeature.diffuseAlbedo = { 0.7f,0.9f,0.75f };
		objFeature.roughness = 0.3f;
		objFeature.fresnel = { 0.1f,0.1f,0.1f };
		mDirectX.SetObjConstantBuffer(i, &objFeature, sizeof(obj));
	}
	
	trans env = {};
	env.viewProjection = mCamera->mViewProjection;
	env.lights[0].mPosition = {10.0f,10.0f,-15.0f};
	env.lights[0].mDirection = { 1.0f,-1.0f,0.0f };
	env.lights[0].mColor = { 1.0f,1.0f,1.0f };
	env.lights[0].mType = Directional;
	env.lights[1].mPosition = { 10.0f,10.0f,-15.0f };
	env.lights[1].mDirection = { 1.0f,-1.0f,0.0f };
	env.lights[1].mColor = { 1.0f,1.0f,1.0f };
	env.lights[1].mType = Point;
	env.lights[2].mPosition = { 0.0f,0.0f,-1.0f };
	env.lights[2].mDirection = { 0.0f,0.0f,1.0f };
	env.cameraPosition = mCamera->mPosition;
	//mDirectX.SetTransConstantBuffer(0, &mCamera->mViewProjection, sizeof(trans));
	mDirectX.SetTransConstantBuffer(0, &env, sizeof(trans));
}

void Game::Draw()
{
	mDirectX.Draw();
 
	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = mVertexBuffer->GetGpuAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(Vertex)*allVertices.size();

	D3D12_INDEX_BUFFER_VIEW ibv = {};
	ibv.BufferLocation = mIndexBuffer->GetGpuAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = sizeof(uint16_t)*allIndices.size();

	//vertex buffer, index buffer 바인딩.
	ID3D12GraphicsCommandList* cmdList = mDirectX.GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);

	int baseIndex = 0;
	int baseVertex = 0;
	int i = 0;
	for (auto model = mModels.begin(); model != mModels.end(); model++, ++i)
	{
		mDirectX.SetObjConstantIndex(i);
		cmdList->DrawIndexedInstanced(model->second->mIndices.size(), 1, baseIndex, baseVertex, 0);
		baseIndex += model->second->mIndices.size();
		baseVertex += model->second->mVertices.size();
	}

	mDirectX.TransitionToPresent();
	mDirectX.CloseAndExecute();

	mDirectX.DrawFinish();
}

void Game::Input()
{
	float deltaTime = mTimer.GetDeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		mCamera->GoFront(10.0f * deltaTime);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera->GoFront(-10.0f * deltaTime);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera->GoRight(10.0f * deltaTime);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera->GoRight(-10.0f * deltaTime);
}