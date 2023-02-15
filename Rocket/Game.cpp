#include "Game.h"

Game::Game(HINSTANCE hInstance)
	: mWindow(hInstance,mWidth,mHeight), mDirectX(mWidth,mHeight), mCamera(make_unique<Camera>(mWidth,mHeight))
{

}

void Game::Initialize()
{
	//������ �ʱ�ȭ
	mWindow.Initialize();

	//DirectX ��ü�� �ʱ�ȭ (Frame, swapchain, depth buffer, command objects, root signature, shader ��)
	mDirectX.Initialize(mWindow.GetWindowHandle());
	
	//�� �ε� (���ؽ�, �ε���)
	LoadModel();

	//���� ����, �� ������ ī�� (commandlist�� ����)
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

void Game::LoadModel()
{
	unique_ptr<Model> table = make_unique<Model>(mDirectX.GetDevice(), "../Model/Wood_Table.obj", mDirectX.GetCommandList());
	mModels["table"] = move(table);

	unique_ptr<Model> house = make_unique<Model>(mDirectX.GetDevice(), "../Model/triangle.obj", mDirectX.GetCommandList());
	mModels["triangle"] = move(house);

	unique_ptr<Model> woodHouse = make_unique<Model>(mDirectX.GetDevice(), "../Model/WoodHouse.obj", mDirectX.GetCommandList());
	mModels["woodHouse"] = move(woodHouse);//frame���� obj constant buffer ũ�� �÷�����.
}

void Game::CreateVertexIndexBuffer()
{
	//make_move_iterator�� �ٲ㼭 move�ؾ� ������ �ö�.
	for (auto model = mModels.begin(); model != mModels.end(); model++)
	{
		allVertices.insert(allVertices.end(), model->second->mVertices.begin(), model->second->mVertices.end());
		allIndices.insert(allIndices.end(), model->second->mIndices.begin(), model->second->mIndices.end());
	}

	//Vertex, Index ���� ����
	mVertexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(Vertex) * allVertices.size());
	mIndexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(uint16_t) * allIndices.size());

	//Vertex, Index ���ۿ� Model ������ copy
	mVertexBuffer->Copy(allVertices.data(), sizeof(Vertex) * allVertices.size(), mDirectX.GetCommandList());
	mIndexBuffer->Copy(allIndices.data(), sizeof(uint16_t) * allIndices.size(), mDirectX.GetCommandList());
}

void Game::Update()
{
	//���� �������� gpu���� ���� draw���� �ʾ��� �� ��ٸ���, �Ϸ�� ��쿡�� ���� frame���� �Ѿ�� ����.
	mDirectX.Update();

	//Model�� position���κ��� world, Camera�� �����ͷκ��� view�� projection matrix�� �����Ѵ�.
	//model�� world matrix�� ������Ʈ
	for (auto it = mModels.begin(); it != mModels.end(); it++)
	{
		XMFLOAT3 pos = it->second->mPosition;
		XMMATRIX world = XMMatrixScaling(0.5f,0.5f,0.5f)*XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mWorld, world);
	}

	XMVECTOR eye = XMVectorSet(mCamera->mPosition.x, mCamera->mPosition.y, mCamera->mPosition.z,1.0f);
	XMVECTOR lookAt = XMVectorSet(mCamera->mLookAt.x, mCamera->mLookAt.y, mCamera->mLookAt.z,0.0f);
	XMVECTOR up = XMVectorSet(mCamera->mUp.x, mCamera->mUp.y, mCamera->mUp.z,0.0f);

	XMMATRIX view = XMMatrixLookAtLH(eye,lookAt,up);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(mCamera->mAngle, mCamera->mRatio, mCamera->mNear, mCamera->mFar);

	XMStoreFloat4x4(&mCamera->mViewProjection,XMMatrixMultiply(view, projection));
	
	//�� �𵨺��� obj constant�� constant buffer�� �ش���ġ�� �ε���.

	int i = 0;
	for (auto model = mModels.begin(); model != mModels.end(); model++,++i)
		mDirectX.SetObjConstantBuffer(i, &model->second->mWorld, sizeof(obj));
	
	trans env = {};
	env.viewProjection = mCamera->mViewProjection;
	env.lights[0].mPosition = {1.0f,0.0f,0.0f};
	env.lights[0].mDirection = { -1.0f,0.0f,0.0f };
	env.lights[1].mPosition = { 0.0f,1.0f,0.0f };
	env.lights[1].mDirection = { 0.0f,-1.0f,0.0f };
	env.lights[2].mPosition = { 0.0f,0.0f,-1.0f };
	env.lights[2].mDirection = { 0.0f,0.0f,1.0f };
	env.lights[2].mColor = { 0.3f,0.5f,0.7f };
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

	//vertex buffer, index buffer ���ε�.
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