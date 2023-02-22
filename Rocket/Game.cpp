#include "Game.h"
#include "DDSTextureLoader.h"

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
	
	int numModels = LoadScene();

	mDirectX.CreateSrv(numModels);

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
	mIsModelSelected = true;
	
	mSelectedModel = mScenes[mCurrentScene]->mModels->begin()->second;

	XMFLOAT3 pos = mSelectedModel->GetPosition();
	XMFLOAT3 newPos;
	float p00 = mScenes[mCurrentScene]->envFeature.projection._11;
	float p11 = mScenes[mCurrentScene]->envFeature.projection._22;
	
	//viewport���� view coordinate���� ��ȯ, z = 10
	newPos.x = (2.0f * x / (float)mWidth - 1.0f)/p00 * 10.0f;
	newPos.y = (-2.0f * y / (float)mHeight + 1.0f)/p11 *10.0f;
	newPos.z = 10.0f;
	
	//newPos�� VC���� WC�� ��ȯ�Ѵ�.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&mScenes[mCurrentScene]->mCamera->view);
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det,inverseViewMatrix);

	XMVECTOR newPosVector = XMLoadFloat3(&newPos);

	newPosVector = XMVector3TransformCoord(newPosVector, inverseViewMatrix);

	XMStoreFloat3(&newPos, newPosVector);

	mSelectedModel->SetPosition(newPos);
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Game::Get()->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool selected = false;

	switch (msg)
	{
	case WM_MOUSEMOVE:
		if(wParam == MK_LBUTTON)
			SelectObject(LOWORD(lParam),HIWORD(lParam));
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

	//�� �ε� (���ؽ�, �ε���)
	mScenes[mCurrentScene]->mModels = LoadModel(0);

	numModels += mScenes[mCurrentScene]->mModels->size();

	//���� ����, �� ������ ī�� (commandlist�� ����)
	mScenes[mCurrentScene]->CreateVertexIndexBuffer(mDirectX);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();
	

	//scene 1
	mCurrentScene++;
	mScenes.push_back(make_unique<Scene>());
	
	//�� �ε� (���ؽ�, �ε���)
	mScenes[mCurrentScene]->mModels = LoadModel(1);

	numModels += mScenes[mCurrentScene]->mModels->size();

	//���� ����, �� ������ ī�� (commandlist�� ����)
	mScenes[mCurrentScene]->CreateVertexIndexBuffer(mDirectX);

	mScenes[mCurrentScene]->mCamera = make_unique<Camera>(mWidth, mHeight);

	mScenes[mCurrentScene]->envFeature = SetLight();

	return numModels;
}

/*
* model�� vertex,index offset�� scene ������ �����ϱ� buffer�� �����ϱ� ������ � scene���� �ε��� ������ ������ش�.
*/
unique_ptr<Models> Game::LoadModel(int sceneIndex)
{
	unique_ptr<Models> model = make_unique<Models>();

	if (sceneIndex == 0)
	{
		shared_ptr<Model> table = make_shared<Model>(mDirectX.GetDevice(), "../Model/table.obj", mDirectX.GetCommandList(), sceneIndex);
		IfError::Throw(CreateDDSTextureFromFile12(mDirectX.GetDevice(), mDirectX.GetCommandList(), L"../Model/textures/bricks3.dds",
			table->mTexture.mResource, table->mTexture.mUpload),
			L"load dds texture error!");
		(*model)["table1"] = move(table);
	}
	else if (sceneIndex == 1)
	{
		shared_ptr<Model> woodHouse = make_shared<Model>(mDirectX.GetDevice(), "../Model/KSR-29 sniper rifle new_obj.obj", mDirectX.GetCommandList(), sceneIndex);
		woodHouse->SetPosition(0.0f, 0.2f, 0.0f);
		IfError::Throw(CreateDDSTextureFromFile12(mDirectX.GetDevice(), mDirectX.GetCommandList(), L"../Model/textures/bricks3.dds",
			woodHouse->mTexture.mResource, woodHouse->mTexture.mUpload),
			L"load dds texture error!");
		(*model)["woodHouse"] = move(woodHouse);//frame���� obj constant buffer ũ�� �÷�����;
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
		XMFLOAT3 pos = it->second->GetPosition();
		XMMATRIX world = XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mObjFeature.world, world);
	}

	mScenes[mCurrentScene]->envFeature.view = mScenes[mCurrentScene]->mCamera->view;
	mScenes[mCurrentScene]->envFeature.projection = mScenes[mCurrentScene]->mCamera->projection;

	//�� �𵨺��� obj constant�� constant buffer�� �ش���ġ�� �ε���.
	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i) {
		mDirectX.SetObjConstantBuffer(model->second->mObjIndex, &model->second->mObjFeature, sizeof(obj));
	}
	
	mScenes[mCurrentScene]->envFeature.cameraPosition = mScenes[mCurrentScene]->mCamera->GetPosition();
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

	ID3D12DescriptorHeap* heaps[] = { mDirectX.getSrvHeap() };
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	int i = 0;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++, ++i)
	{
		//�𵨸��� obj constant index ��������� ��������.
		mDirectX.SetObjConstantIndex(model->second->mObjIndex);
		mDirectX.SetSrvIndex(model->second->mObjIndex);
		cmdList->DrawIndexedInstanced(model->second->mIndexBufferSize, 1, model->second->mIndexBufferOffset, model->second->mVertexBufferOffset, 0);
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