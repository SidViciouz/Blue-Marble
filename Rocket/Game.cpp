#include "Game.h"

Game::Game(HINSTANCE hInstance)
	: mDirectX(mWidth,mHeight)
{
	mLatestWindow = this;
}

void Game::Initialize()
{
	//윈도우 초기화
	InitializeWindow();

	//DirectX 객체들 초기화 (Frame, swapchain, depth buffer, command objects, root signature, shader 등)
	mDirectX.Initialize(mWindowHandle);
	
	LoadScene();

	//모델 로드 (버텍스, 인덱스)
	LoadModel();

	//버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
	CreateVertexIndexBuffer();

	SetLight();

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

void Game::LoadScene()
{
	unique_ptr<Scene> scene = make_unique<Scene>();

	scene->mModels = LoadModel();

	scene->mCamera = make_unique<Camera>(mWidth, mHeight);

	scene->envFeature = SetLight();

	mScenes.push_back(move(scene));
}

 unique_ptr<Models> Game::LoadModel()
{
	unique_ptr<Models> model = make_unique<Models>();

	unique_ptr<Model> table = make_unique<Model>(mDirectX.GetDevice(), "../Model/Wood_Table.obj", mDirectX.GetCommandList());
	(*model)["table"] = move(table);

	unique_ptr<Model> house = make_unique<Model>(mDirectX.GetDevice(), "../Model/triangle.obj", mDirectX.GetCommandList());
	(*model)["triangle"] = move(house);

	unique_ptr<Model> woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/WoodHouse.obj", mDirectX.GetCommandList());
	(*model)["woodHouse"] = move(woodHouse);//frame에서 obj constant buffer 크기 늘려야함.

	return move(model);
}

void Game::CreateVertexIndexBuffer()
{

	//Vertex, Index 버퍼 생성
	Model::mVertexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(Vertex) * Model::mAllVertices.size());
	Model::mIndexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(uint16_t) * Model::mAllIndices.size());

	//Vertex, Index 버퍼에 Model 데이터 copy
	Model::mVertexBuffer->Copy(Model::mAllVertices.data(), sizeof(Vertex) * Model::mAllVertices.size(), mDirectX.GetCommandList());
	Model::mIndexBuffer->Copy(Model::mAllIndices.data(), sizeof(uint16_t) * Model::mAllIndices.size(), mDirectX.GetCommandList());
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

	//Model의 position으로부터 world, Camera의 데이터로부터 view와 projection matrix를 설정한다.
	//model의 world matrix를 업데이트
	for (auto it = mScenes[mCurrentScene]->mModels->begin(); it != mScenes[mCurrentScene]->mModels->end(); it++)
	{
		XMFLOAT3 pos = it->second->mPosition;
		XMMATRIX world = XMMatrixScaling(0.5f, 0.5f, 0.5f)*XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mObjFeature.world, world);
	}

	XMVECTOR right = XMLoadFloat3(&mScenes[mCurrentScene]->mCamera->mRight);
	XMVECTOR up = XMLoadFloat3(&mScenes[mCurrentScene]->mCamera->mUp);
	XMVECTOR front = XMVector3Normalize(XMLoadFloat3(&mScenes[mCurrentScene]->mCamera->mFront));
	XMVECTOR position = XMLoadFloat3(&mScenes[mCurrentScene]->mCamera->mPosition);

	up = XMVector3Normalize(XMVector3Cross(front, right));
	right = XMVector3Cross(up, front);

	XMStoreFloat3(&mScenes[mCurrentScene]->mCamera->mRight, right);
	XMStoreFloat3(&mScenes[mCurrentScene]->mCamera->mUp, up);
	XMStoreFloat3(&mScenes[mCurrentScene]->mCamera->mFront,front);

	float x = -XMVectorGetX(XMVector3Dot(position, right));
	float y = -XMVectorGetX(XMVector3Dot(position, up));
	float z = -XMVectorGetX(XMVector3Dot(position, front));

	XMFLOAT4X4 viewMatrix = {
		mScenes[mCurrentScene]->mCamera->mRight.x, mScenes[mCurrentScene]->mCamera->mUp.x, mScenes[mCurrentScene]->mCamera->mFront.x, 0.0f,
		mScenes[mCurrentScene]->mCamera->mRight.y, mScenes[mCurrentScene]->mCamera->mUp.y, mScenes[mCurrentScene]->mCamera->mFront.y, 0.0f,
		mScenes[mCurrentScene]->mCamera->mRight.z, mScenes[mCurrentScene]->mCamera->mUp.z, mScenes[mCurrentScene]->mCamera->mFront.z, 0.0f,
		x,y,z,1.0f
	};

	XMMATRIX view = XMLoadFloat4x4(&viewMatrix);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(mScenes[mCurrentScene]->mCamera->mAngle, mScenes[mCurrentScene]->mCamera->mRatio,
		mScenes[mCurrentScene]->mCamera->mNear, mScenes[mCurrentScene]->mCamera->mFar);

	XMStoreFloat4x4(&mScenes[mCurrentScene]->mCamera->mViewProjection,XMMatrixMultiply(view, projection));
	
	//각 모델별로 obj constant를 constant buffer의 해당위치에 로드함.

	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i) {
		mDirectX.SetObjConstantBuffer(model->second->mObjConstantIndex, &model->second->mObjFeature, sizeof(obj));
	}
	
	mScenes[mCurrentScene]->envFeature.viewProjection = mScenes[mCurrentScene]->mCamera->mViewProjection;
	mScenes[mCurrentScene]->envFeature.cameraPosition = mScenes[mCurrentScene]->mCamera->mPosition;

	mDirectX.SetTransConstantBuffer(0, &mScenes[mCurrentScene]->envFeature, sizeof(trans));
}

void Game::Draw()
{
	mDirectX.Draw();
 
	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = Model::mVertexBuffer->GetGpuAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(Vertex)*Model::mAllVertices.size();

	D3D12_INDEX_BUFFER_VIEW ibv = {};
	ibv.BufferLocation = Model::mIndexBuffer->GetGpuAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = sizeof(uint16_t)* Model::mAllIndices.size();

	//vertex buffer, index buffer 바인딩.
	ID3D12GraphicsCommandList* cmdList = mDirectX.GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);


	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i)
	{
		//모델마다 obj constant index 멤버변수를 만들어야함.
		mDirectX.SetObjConstantIndex(model->second->mObjConstantIndex);
		cmdList->DrawIndexedInstanced(model->second->mIndexBufferSize, 1, model->second->mIndexBufferOffset, model->second->mVertexBufferOffset, 0);
	}

	mDirectX.TransitionToPresent();
	mDirectX.CloseAndExecute();

	mDirectX.DrawFinish();
}

void Game::Input()
{
	float deltaTime = mTimer.GetDeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		mScenes[mCurrentScene]->mCamera->GoFront(10.0f * deltaTime);

	if (GetAsyncKeyState('S') & 0x8000)
		mScenes[mCurrentScene]->mCamera->GoFront(-10.0f * deltaTime);

	if (GetAsyncKeyState('D') & 0x8000)
		mScenes[mCurrentScene]->mCamera->GoRight(10.0f * deltaTime);

	if (GetAsyncKeyState('A') & 0x8000)
		mScenes[mCurrentScene]->mCamera->GoRight(-10.0f * deltaTime);
}