#include "Game.h"

vector<unique_ptr<Scene>> Game::mScenes;

Game::Game(HINSTANCE hInstance)
	: mDirectX(mWidth,mHeight)
{
	mLatestWindow = this;
}

void Game::Initialize()
{
	//������ �ʱ�ȭ
	InitializeWindow();

	//DirectX ��ü�� �ʱ�ȭ (Frame, swapchain, depth buffer, command objects, root signature, shader ��)
	mDirectX.Initialize(mWindowHandle);
	
	LoadScene();

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

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Game::Get()->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//������ ���� ī�޶� ������ ������Ʈ�� �������� �����.
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		ChangeScene(0);
		return 0;
	case WM_RBUTTONDOWN:
		ChangeScene(1);
		return 0;
		/*
	case WM_KEYDOWN :
		if(wParam == 0x57)
			mCamera->GoFront(10.0f * mTimer.GetDeltaTime());
		else if(wParam == 0x53)
			mCamera->GoFront(-10.0f * mTimer.GetDeltaTime());
		else if (wParam == 0x41)
			mCamera->GoRight(-10.0f*mTimer.GetDeltaTime());
		else if(wParam == 0x44)
			mCamera->GoRight(10.0f * mTimer.GetDeltaTime());
		*/
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
	
	//�� �ε� (���ؽ�, �ε���)
	mScenes[mCurrentScene]->mModels = LoadModel(0);
	//���� ����, �� ������ ī�� (commandlist�� ����)
	mScenes[mCurrentScene]->CreateVertexIndexBuffer(mDirectX);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();


	//scene 1
	mCurrentScene++;
	mScenes.push_back(make_unique<Scene>());

	//�� �ε� (���ؽ�, �ε���)
	mScenes[mCurrentScene]->mModels = LoadModel(1);
	//���� ����, �� ������ ī�� (commandlist�� ����)
	mScenes[mCurrentScene]->CreateVertexIndexBuffer(mDirectX);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();
}

/*
* model�� vertex,index offset�� scene ������ �����ϱ� buffer�� �����ϱ� ������ � scene���� �ε��� ������ ������ش�.
*/
 unique_ptr<Models> Game::LoadModel(int sceneIndex)
{
	unique_ptr<Models> model = make_unique<Models>();

	if (sceneIndex == 0)
	{
		//unique_ptr<Model> field = make_unique<Model>(mDirectX.GetDevice(), "../Model/field.obj", mDirectX.GetCommandList(), sceneIndex);
		//(*model)["field"] = move(field);

		unique_ptr<Model> table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		(*model)["table1"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { 0.0f,3.0f,0.0f };
		(*model)["table2"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { 0.0f,-3.0f,0.0f };
		(*model)["table3"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { 3.0f,0.0f,0.0f };
		(*model)["table4"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { -3.0f,0.0f,0.0f };
		(*model)["table5"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { 3.0f,3.0f,0.0f };
		(*model)["table6"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { -3.0f,-3.0f,0.0f };
		(*model)["table7"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { 3.0f,-3.0f,0.0f };
		(*model)["table8"] = move(table);

		table = make_unique<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		table->mPosition = { -3.0f,3.0f,0.0f };
		(*model)["table9"] = move(table);

	}
	else if (sceneIndex == 1)
	{
		unique_ptr<Model> woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/KSR-29 sniper rifle new_obj.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->mPosition = { 0.0f,0.2f,0.0f };
		(*model)["woodHouse"] = move(woodHouse);//frame���� obj constant buffer ũ�� �÷�����.
		/*
		woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/woodHouse.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->mPosition = { 10.0f,0.2f,0.0f };
		(*model)["woodHouse2"] = move(woodHouse);

		woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/woodHouse.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->mPosition = { -10.0f,0.2f,0.0f };
		(*model)["woodHouse3"] = move(woodHouse);

		woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/woodHouse.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->mPosition = { 0.0f,10.2f,0.0f };
		(*model)["woodHouse4"] = move(woodHouse);

		woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/woodHouse.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->mPosition = { 0.0f,-10.2f,0.0f };
		(*model)["woodHouse5"] = move(woodHouse);*/
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
	//���� �������� gpu���� ���� draw���� �ʾ��� �� ��ٸ���, �Ϸ�� ��쿡�� ���� frame���� �Ѿ�� ����.
	mDirectX.Update();

	//���� ���� �������� ������Ʈ�� ���⼭���� �Ͼ��.
	Input();

	//Model�� position���κ��� world, Camera�� �����ͷκ��� view�� projection matrix�� �����Ѵ�.
	//model�� world matrix�� ������Ʈ
	for (auto it = mScenes[mCurrentScene]->mModels->begin(); it != mScenes[mCurrentScene]->mModels->end(); it++)
	{
		XMFLOAT3 pos = it->second->mPosition;
		XMMATRIX world = XMMatrixTranslation(pos.x, pos.y, pos.z);
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

	XMMATRIX projection =XMMatrixPerspectiveFovLH(mScenes[mCurrentScene]->mCamera->mAngle, mScenes[mCurrentScene]->mCamera->mRatio,
		mScenes[mCurrentScene]->mCamera->mNear, mScenes[mCurrentScene]->mCamera->mFar);

	XMStoreFloat4x4(&mScenes[mCurrentScene]->envFeature.view,view);
	XMStoreFloat4x4(&mScenes[mCurrentScene]->envFeature.projection,projection);
	//�� �𵨺��� obj constant�� constant buffer�� �ش���ġ�� �ε���.

	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i) {
		mDirectX.SetObjConstantBuffer(model->second->mObjConstantIndex, &model->second->mObjFeature, sizeof(obj));
	}
	
	mScenes[mCurrentScene]->envFeature.cameraPosition = mScenes[mCurrentScene]->mCamera->mPosition;
	mScenes[mCurrentScene]->envFeature.cameraFront = mScenes[mCurrentScene]->mCamera->mFront;

	mDirectX.SetTransConstantBuffer(0, &mScenes[mCurrentScene]->envFeature, sizeof(trans));
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

	//vertex buffer, index buffer ���ε�.
	ID3D12GraphicsCommandList* cmdList = mDirectX.GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);

	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i)
	{
		//�𵨸��� obj constant index ��������� ��������.
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