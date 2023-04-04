#pragma once
#include "Defines.h"
#include "framework.h"
#include "Model.h"
#include <map>
#include "Camera.h"
#include "Constant.h"
#include "Timer.h"
#include "Scene.h"
#include "Entity.h"
#include "Clickable.h"
#include "Unclickable.h"
#include "Inventory.h"
#include "Button.h"
#include "VolumeCube.h"
#include "VolumeSphere.h"
#include "RigidBodySystem.h"
#include <random>
#include "d3dx12.h"
#include "Frame.h"
#include "DescriptorManager.h"
#include "ResourceManager.h"
#include "PerlinMap.h"
#include "MeshManager.h"
#include "MeshNode.h"
#include "VolumeNode.h"
#include "CapsuleCollisionComponent.h"
#include "SphereCollisionComponent.h"
#include "BoxCollisionComponent.h"

using PSOs = unordered_map<string, ComPtr< ID3D12PipelineState>>;
using RootSigs = unordered_map<string, ComPtr<ID3D12RootSignature>>;
using Shaders = unordered_map<string, ComPtr<ID3DBlob>>;

class Engine
{
	//Engine ��ü �ܺο��� ����� �� �ִ� �������̽�
public:
												Engine(const Engine& engine) = delete;
												Engine(Engine&& engine) = delete;
	Engine&										operator=(const Engine& engine) = delete;
	Engine&										operator=(Engine&& engine) = delete;

												Engine(HINSTANCE hInstance);

												//������ ���� �޼���
	static Engine*								Get();
	LRESULT										WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void										Initialize();
	void										Run();
	void										ChangeScene(int dstScene);
	void										MoveObject(int x, int y);
	void										SelectObject(int x,int y);
	void										SelectInventory(int x, int y);
	/*
	* ���� ������ �������� �����ӿ��� GPU�� ��� ������ ��ĥ ������ ��ٸ���.
	*/
	void										WaitUntilPrevFrameComplete();

	static vector<unique_ptr<Scene>>			mScenes;
	static int									mCurrentScene;
	static ComPtr<ID3D12GraphicsCommandList>	mCommandList;
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator;

	static vector<unique_ptr<Frame>>			mFrames;
	int											mNumberOfFrames = 3;
	static int									mCurrentFrame;


	//Engine ��ü ���������� ���Ǵ� �޼���� ��� ����
private:
	//������ ���� �޼���
	void										InitializeWindow();
	void										DebugEnable();
	void										LoadScene();
	/*
	* numObjConstant�� ����ŭ obj constant buffer�� ũ�⸦ �����.
	*/
	void										CreateFrames();
	void										CreateCommandObjects();
	unique_ptr<Clickables>						CreateModel(int sceneIndex);
	unique_ptr<Unclickables>					CreateWorld(int sceneIndex);
	unique_ptr<Volumes>							CreateVolume(int sceneIndex);
	env											SetLight();
	void										Update();
	void										Draw();
	void										Input();

	//������ ���� ������
	static Engine*								mLatestWindow;
	HINSTANCE									mInstance;
	HWND										mWindowHandle = nullptr;
	std::wstring								mWindowCaption = L"Rocket Game";

	int											mWidth = 800;
	int											mHeight = 600;

	Timer										mTimer;

	bool										mIsInventorySelected = false;
	
	ComPtr<ID3D12Debug>							debugController;

	unique_ptr<RigidBodySystem>					mRigidBodySystem;






	//--------------------------------pipeline�� �ִ� �����------------------------------
public:

	static ComPtr<ID3D12Device>					mDevice;
	static ComPtr<ID3D12CommandQueue>			mCommandQueue;
	ComPtr<ID3D12Fence>							mFence = nullptr;
	UINT64										mFenceValue = 0;
	ComPtr<IDXGISwapChain>						mSwapChain;
	//ComPtr<ID3D12Resource>						mBackBuffers[2];
	int											mCurrentBackBuffer = 0;
	static PSOs									mPSOs;
	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;
	static RootSigs								mRootSignatures;

	void										InitializePipeline();
	void										CreateObjects();


	void										CreateShaderAndRootSignature();
	void										CreatePso();
	void										SetViewportAndScissor();

	static ComPtr<IDXGIFactory4>				mFactory;

	DXGI_FORMAT									mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT									mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	bool										mMsaaEnable = false;
	UINT										mMsaaQuality = 0;

	Shaders										mShaders;
//-----------------------------pipeline ��----------------------

	static unique_ptr<DescriptorManager>		mDescriptorManager;
	static unique_ptr<ResourceManager>			mResourceManager;
	
	int											mDepthBufferIndex;
	int											mBackBufferOffset;

	static unique_ptr<PerlinMap>				mPerlinMap;

	static unique_ptr<MeshManager>				mMeshManager;

	shared_ptr<MeshNode> boxMesh;
	shared_ptr<MeshNode> ballMesh;
	shared_ptr<MeshNode> groundMesh;
};
