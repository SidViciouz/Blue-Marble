#include "Game.h"

Game::Game(HINSTANCE hInstance)
	: mWindow(hInstance,mWidth,mHeight), mDirectX(mWidth,mHeight)
{

}

void Game::Initialize()
{
	mWindow.Initialize();
	mDirectX.Initialize(mWindow.GetWindowHandle());
	
	LoadModel();

	//버퍼 생성, 모델 데이터 카피
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
	//Vertex, Index 버퍼 생성
	int vertexSize = 0;
	int indexSize = 0;
	for (auto it = mModels.begin(); it != mModels.end(); it++)
	{
		vertexSize += it->second->mVertices.size();
		indexSize += it->second->mIndices.size();
	}
	mVertexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(Vertex) * vertexSize);
	mIndexBuffer = make_unique<Buffer>(mDirectX.GetDevice(), sizeof(uint16_t) * indexSize);

	//Vertex, Index 버퍼에 Model 데이터 copy
	//vertex buffer와 index buffer에 값을 차곡차곡 쌓을 수 있도록 수정해야함. 아마 Copy함수도 수정해야할 듯.
	mVertexBuffer->Copy(mModels["car"]->mVertices.data(), sizeof(Vertex) * vertexSize, mDirectX.GetCommandList());
	mIndexBuffer->Copy(mModels["car"]->mIndices.data(), sizeof(uint16_t) * indexSize, mDirectX.GetCommandList());
}
