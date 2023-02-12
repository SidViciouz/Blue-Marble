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

	//���� ����, �� ������ ī��
	CreateVertexIndexBuffer();
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
	unique_ptr<Model> m = make_unique<Model>(mDirectX.GetDevice(), "../Model/car.obj", mDirectX.GetCommandList());
	mModels["car"] = move(m);
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
	mVertexBuffer->Copy(mModels["car"]->mVertices.data(), sizeof(Vertex) * vertexSize, mDirectX.GetCommandList());
	mIndexBuffer->Copy(mModels["car"]->mIndices.data(), sizeof(uint16_t) * indexSize, mDirectX.GetCommandList());
}

void Game::Update()
{
	
}

void Game::Draw()
{

}