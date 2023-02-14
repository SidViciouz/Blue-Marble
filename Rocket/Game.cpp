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

	//mDirectX.CloseAndExecute();
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
			//Timer�� �߰��ؾ���.
			Update();
			Draw();
		}
	}
}

void Game::LoadModel()
{
	unique_ptr<Model> table = make_unique<Model>(mDirectX.GetDevice(), "../Model/Wood_Table.obj", mDirectX.GetCommandList());
	mModels["table"] = move(table);

	//unique_ptr<Model> house = make_unique<Model>(mDirectX.GetDevice(), "../Model/WoodHouse.obj", mDirectX.GetCommandList());
	//mModels["house"] = move(house);
}

void Game::CreateVertexIndexBuffer()
{
	//Vertex, Index ���� ����
	int vertexSize = 0;
	int indexSize = 0;
	for (auto it = mModels.begin(); it != mModels.end(); it++)
	{
		vertexSize += it->second->mVertices.size();
		indexSize += it->second->mIndices.size();
	}
	mVertexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(Vertex) * vertexSize);
	mIndexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(uint16_t) * indexSize);

	//Vertex, Index ���ۿ� Model ������ copy
	//vertex buffer�� index buffer�� ���� �������� ���� �� �ֵ��� �����ؾ���. �Ƹ� Copy�Լ��� �����ؾ��� ��.
	mVertexBuffer->Copy(mModels["table"]->mVertices.data(), sizeof(Vertex) * mModels["table"]->mVertices.size(), mDirectX.GetCommandList());
	mIndexBuffer->Copy(mModels["table"]->mIndices.data(), sizeof(uint16_t) * mModels["table"]->mIndices.size(), mDirectX.GetCommandList());
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
		XMMATRIX world = XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mWorld, world);
	}

	XMVECTOR eye = XMVectorSet(mCamera->mPosition.x, mCamera->mPosition.y, mCamera->mPosition.z,1.0f);
	XMVECTOR lookAt = XMVectorSet(mCamera->mLookAt.x, mCamera->mLookAt.y, mCamera->mLookAt.z,0.0f);
	XMVECTOR up = XMVectorSet(mCamera->mUp.x, mCamera->mUp.y, mCamera->mUp.z,0.0f);

	XMMATRIX view = XMMatrixLookAtLH(eye,lookAt,up);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(mCamera->mAngle, mCamera->mRatio, mCamera->mNear, mCamera->mFar);

	XMStoreFloat4x4(&mCamera->mViewProjection,XMMatrixMultiply(view, projection));

	//constant buffer update, �𵨺��� �ε��� �߰��ؼ� �װ��� ����ؾ���.
	mDirectX.SetObjConstantBuffer(0, &mModels["table"]->mWorld, sizeof(obj));

	mDirectX.SetTransConstantBuffer(0, &mCamera->mViewProjection, sizeof(obj));
}

void Game::Draw()
{
	mDirectX.Draw();

	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = mVertexBuffer->GetGpuAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(Vertex)*mModels["table"]->mVertices.size();

	D3D12_INDEX_BUFFER_VIEW ibv = {};
	ibv.BufferLocation = mIndexBuffer->GetGpuAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = sizeof(uint16_t)*mModels["table"]->mIndices.size();

	//vertex buffer, index buffer ���ε�.
	ID3D12GraphicsCommandList* cmdList = mDirectX.GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);

	cmdList->DrawIndexedInstanced(mModels["table"]->mIndices.size(), 1, 0, 0, 0);

	mDirectX.TransitionToPresent();
	mDirectX.CloseAndExecute();

	mDirectX.DrawFinish();
}