#pragma once
#include "Defines.h"
#include "framework.h"
#include <map>
#include "Constant.h"
#include "Timer.h"
#include "Scene.h"
#include "Entity.h"
#include <random>
#include "d3dx12.h"
#include "Frame.h"
#include "DescriptorManager.h"
#include "ResourceManager.h"
#include "PerlinMap.h"
#include "MeshManager.h"
#include "MeshNode.h"
#include "VolumeNode.h"
#include "BoxCollisionComponent.h"
#include "InputManager.h"
#include "MainScene.h"
#include "TextManager.h"
#include "MenuScene.h"

using PSOs = unordered_map<string, ComPtr< ID3D12PipelineState>>;
using RootSigs = unordered_map<string, ComPtr<ID3D12RootSignature>>;
using Shaders = unordered_map<string, ComPtr<ID3DBlob>>;

class Engine
{
	//Engine 객체 외부에서 사용할 수 있는 인터페이스
public:
												Engine(const Engine& engine) = delete;
												Engine(Engine&& engine) = delete;
	Engine&										operator=(const Engine& engine) = delete;
	Engine&										operator=(Engine&& engine) = delete;

												Engine(HINSTANCE hInstance);

												//윈도우 관련 메서드
	static Engine*								Get();
	LRESULT										WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void										Initialize();
	void										Run();

	/*
	* 현재 프레임 전까지의 프레임에서 GPU가 모든 연산을 마칠 때까지 기다린다.
	*/
	void										WaitUntilPrevFrameComplete();

	static ComPtr<ID3D12GraphicsCommandList>	mCommandList;
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator;

	static vector<unique_ptr<Frame>>			mFrames;
	int											mNumberOfFrames = 3;
	static int									mCurrentFrame;
	static Timer								mTimer;

	static int									mWidth;
	static int									mHeight;

	//Engine 객체 내부적으로 사용되는 메서드와 멤버 변수
private:
	//윈도우 관련 메서드
	void										InitializeWindow();
	void										DebugEnable();
	/*
	* numObjConstant의 수만큼 obj constant buffer의 크기를 만든다.
	*/
	void										CreateFrames();
	void										CreateCommandObjects();
	void										Update();
	void										Draw();

	//윈도우 관련 데이터
	static Engine*								mLatestWindow;
	HINSTANCE									mInstance;
	HWND										mWindowHandle = nullptr;
	std::wstring								mWindowCaption = L"Rocket Game";

	ComPtr<ID3D12Debug>							debugController;

	//--------------------------------pipeline에 있던 멤버들------------------------------
public:

	static ComPtr<ID3D12Device>					mDevice;
	static ComPtr<ID3D12CommandQueue>			mCommandQueue;
	ComPtr<ID3D12Fence>							mFence = nullptr;
	UINT64										mFenceValue = 0;
	ComPtr<IDXGISwapChain>						mSwapChain;
	//ComPtr<ID3D12Resource>						mBackBuffers[2];
	static int									mCurrentBackBuffer;
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
//-----------------------------pipeline 끝----------------------

	static unique_ptr<DescriptorManager>		mDescriptorManager;
	static unique_ptr<ResourceManager>			mResourceManager;
	
	int											mDepthBufferIndex;
	int											mBackBufferOffset;

	static unique_ptr<PerlinMap>				mPerlinMap;

	static unique_ptr<MeshManager>				mMeshManager;

	static unordered_map<string,shared_ptr<Scene>>	mAllScenes;
	static string								mCurrentSceneName;

	static shared_ptr<InputManager>				mInputManager;
	static shared_ptr<TextManager>				mTextManager;

	static void									ChangeScene(const string& sceneName);
};
