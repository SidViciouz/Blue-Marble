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
#include "ParticleField.h"
#include "RigidBodySystem.h"
#include <random>
#include "d3dx12.h"
#include "Frame.h"

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

	//Engine ��ü ���������� ���Ǵ� �޼���� ��� ����
private:
	//������ ���� �޼���
	void										InitializeWindow();
	void										DebugEnable();
	void										LoadScene();
	/*
	* numObjConstant�� ����ŭ obj constant buffer�� ũ�⸦ �����.
	*/
	void										CreateFrames(int numObjConstant);
	void										CreateCommandObjects();
	unique_ptr<Clickables>						CreateModel(int sceneIndex);
	void										LoadCopyModelToBuffer();
	unique_ptr<Unclickables>					CreateWorld(int sceneIndex);
	unique_ptr<Volumes>							CreateVolume(int sceneIndex);
	trans										SetLight();
	void										SetObjConstantIndex(int index);
	void										Update();
	void										Draw();
	void										Input();
	//
	void										CreateNoiseMap();
	unique_ptr<TextureResource>					mNoiseMap;
	//

	//������ ���� ������
	static Engine*								mLatestWindow;
	HINSTANCE									mInstance;
	HWND										mWindowHandle = nullptr;
	std::wstring								mWindowCaption = L"Rocket Game";

	int											mWidth = 800;
	int											mHeight = 600;

	Timer										mTimer;

	vector<unique_ptr<Frame>>					mFrames;
	int											mNumberOfFrames = 3;
	int											mCurrentFrame = 0;


	bool										mIsModelSelected = false;
	string										mSelectedModelName;
	shared_ptr<Model>							mSelectedModel;

	bool										mIsInventorySelected = false;
	
	ComPtr<ID3D12Debug>							debugController;
	unique_ptr<ParticleField>					mParticleField;

	unique_ptr<RigidBodySystem>					mRigidBodySystem;






	//--------------------------------pipeline�� �ִ� �����------------------------------
public:

	static ComPtr<ID3D12Device>					mDevice;
	ComPtr<ID3D12CommandQueue>					mCommandQueue;
	ComPtr<ID3D12Fence>							mFence = nullptr;
	UINT64										mFenceValue = 0;
	ComPtr<IDXGISwapChain>						mSwapChain;
	ComPtr<ID3D12Resource>						mBackBuffers[2];
	int											mCurrentBackBuffer = 0;
	static PSOs									mPSOs;
	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;
	ComPtr<ID3D12DescriptorHeap>				mRtvHeap;
	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	static RootSigs								mRootSignatures;

	void										InitializePipeline();
	void										CreateObjects(HWND windowHandle);


	void										CreateBackBuffersAndDepthBufferAndViews();
	void										CreateDescriptorHeaps();
	void										CreateSwapChain(HWND windowHandle);
	void										CreateShaderAndRootSignature();
	void										CreatePso();
	void										SetViewportAndScissor();

	ComPtr<IDXGIFactory4>						mFactory = nullptr;

	DXGI_FORMAT									mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT									mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	bool										mMsaaEnable = false;
	UINT										mMsaaQuality = 0;

	ComPtr<ID3D12Resource>						mDepthBuffer;

	Shaders										mShaders;
};
