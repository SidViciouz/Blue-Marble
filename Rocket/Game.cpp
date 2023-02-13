#include "Game.h"

Game::Game(HINSTANCE hInstance)
	: mWindow(hInstance,mWidth,mHeight), mDirectX(mWidth,mHeight), mCamera(make_unique<Camera>(mWidth,mHeight))
{

}

void Game::Initialize()
{
	//윈도우 초기화
	mWindow.Initialize();

	//DirectX 객체들 초기화 (Frame, swapchain, depth buffer, command objects, root signature, shader 등)
	mDirectX.Initialize(mWindow.GetWindowHandle());
	
	//모델 로드 (버텍스, 인덱스)
	LoadModel();

	//버퍼 생성, 모델 데이터 카피 (commandlist에 제출)
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
			//Timer를 추가해야함.
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

void Game::Update()
{
	//현재 프레임이 gpu에서 전부 draw되지 않았을 시 기다리고, 완료된 경우에는 다음 frame으로 넘어가는 역할.
	mDirectX.Update();

	//Model의 position으로부터 world, Camera의 데이터로부터 view와 projection matrix를 설정한다.

	//model의 world matrix를 업데이트
	for (auto it = mModels.begin(); it != mModels.end(); it++)
	{
		XMFLOAT3 pos = it->second->mPosition;
		XMMATRIX world = XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&it->second->mWorld, world);
	}

	XMVECTOR eye = XMLoadFloat3(&mCamera->mPosition);
	XMVECTOR lookAt = XMLoadFloat3(&mCamera->mLookAt);
	XMVECTOR up = XMLoadFloat3(&mCamera->mUp);

	XMMATRIX view = XMMatrixLookAtLH(eye,lookAt,up);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(mCamera->mAngle, mCamera->mRatio, mCamera->mNear, mCamera->mFar);

	XMStoreFloat4x4(&mCamera->mViewProjection,XMMatrixMultiply(view, projection));

	//constant buffer update, 모델별로 인덱스 추가해서 그것을 사용해야함.
	mDirectX.SetObjConstantBuffer(0, &mModels["car"]->mWorld, sizeof(obj));

	mDirectX.SetTransConstantBuffer(0, &mCamera->mViewProjection, sizeof(obj));
}

void Game::Draw()
{
	mDirectX.Draw();

	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = mVertexBuffer->GetGpuAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(Vertex)*mModels["car"]->mVertices.size();

	D3D12_INDEX_BUFFER_VIEW ibv = {};
	ibv.BufferLocation = mIndexBuffer->GetGpuAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = sizeof(uint16_t)*mModels["car"]->mIndices.size();

	//vertex buffer, index buffer 바인딩.
	ID3D12GraphicsCommandList* cmdList = mDirectX.GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);

	cmdList->DrawIndexedInstanced(mModels["car"]->mIndices.size(), 1, 0, 0, 0);

	mDirectX.TransitionToPresent();
	mDirectX.CloseAndExecute();

	mDirectX.DrawFinish();
}