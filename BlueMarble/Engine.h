#pragma once
#include "Defines.h"
#include "framework.h"
#include <map>
#include "IfError.h"
#include "Timer.h"
#include "Scene.h"
#include <random>
#include "d3dx12.h"
#include "Frame.h"
#include "DescriptorManager.h"
#include "ResourceManager.h"
#include "PerlinMap.h"
#include "MeshManager.h"
#include "InputManager.h"
#include "TextManager.h"
#include "TextureManager.h"

using PSOs = unordered_map<string, ComPtr< ID3D12PipelineState>>;
using RootSigs = unordered_map<string, ComPtr<ID3D12RootSignature>>;
using Shaders = unordered_map<string, ComPtr<ID3DBlob>>;

class Engine
{
public:
												Engine(const Engine& engine) = delete;
												Engine(Engine&& engine) = delete;
	Engine&										operator=(const Engine& engine) = delete;
	Engine&										operator=(Engine&& engine) = delete;
												Engine(HINSTANCE hInstance);
	/*
	* engine의 실행에 필요한 모든 데이터들을 초기화한다.
	*/
	void										Initialize();
	/*
	* engine의 loop를 실행시킨다.
	*/
	void										Run();

	/*
	* engine 인스턴스를 반환한다.
	*/
	static Engine*								Get();
	/*
	* 키 입력을 처리한다.
	*/
	LRESULT										WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/*
	* 현재 프레임 전까지의 프레임에서 GPU가 모든 연산을 마칠 때까지 기다린다.
	*/
	void										WaitUntilPrevFrameComplete();
	/*
	* device등을 업데이트한다.
	*/
	void										InitializePipeline();
	/*
	* swapchain, depth buffer, pso등의 객체들을 생성한다.
	*/
	void										CreateObjects();
	/*
	* shader를 컴파일하고, root signature를 생성한다.
	*/
	void										CreateShaderAndRootSignature();
	/*
	* pso를 생성한다.
	*/
	void										CreatePso();
	/*
	* viewport와 scissor rectangle의 값을 저장한다.
	*/
	void										SetViewportAndScissor();
	/*
	* 현재 scene을 전환시킨다.
	*/
	static void									ChangeScene(const string& sceneName);
	/*
	* command list이다.
	*/
	static ComPtr<ID3D12GraphicsCommandList>	mCommandList;
	/*
	* command allocator이다.
	*/
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator;
	/*
	* 각 프레임마다 업데이트되는 정보를 저장하고 있는 변수이다.
	*/
	static vector<unique_ptr<Frame>>			mFrames;
	/*
	* frame의 개수를 나타낸다.
	*/
	int											mNumberOfFrames = 3;
	/*
	* 현재 frame의 번호를 나타낸다.
	*/
	static int									mCurrentFrame;
	/*
	* 게임 전체적인 시간을 관리하는 객체이다.
	*/
	static Timer								mTimer;
	/*
	* 윈도우의 width를 나타낸다.
	*/
	static int									mWidth;
	/*
	* 윈도우의 height를 나타낸다.
	*/
	static int									mHeight;
	/*
	* gpu의 논리적인 장치에 해당한다.
	*/
	static ComPtr<ID3D12Device>					mDevice;
	/*
	* command queue이다.
	*/
	static ComPtr<ID3D12CommandQueue>			mCommandQueue;
	/*
	* fence 객체이다.
	*/
	ComPtr<ID3D12Fence>							mFence = nullptr;
	/*
	* 현재 fence의 값이다.
	*/
	UINT64										mFenceValue = 0;
	/*
	* 현재 back buffer의 번호를 나타낸다.
	*/
	static int									mCurrentBackBuffer;
	/*
	* pso들을 저장하고 있는 자료구조이다.
	*/
	static PSOs									mPSOs;
	/*
	* viewport정보를 저장하고 있는 객체이다.
	*/
	D3D12_VIEWPORT								mViewport;
	/*
	* scissor rectangle 정보를 가지고 있는 객체이다.
	*/
	D3D12_RECT									mScissor;
	/*
	* root signature들의 정보를 저장하고 있는 자료구조이다.
	*/
	static RootSigs								mRootSignatures;
	/*
	* factory 객체이다.
	*/
	static ComPtr<IDXGIFactory4>				mFactory;
	/*
	* back buffer의 format을 나타낸다.
	*/
	static DXGI_FORMAT							mBackBufferFormat;
	/*
	* depth stencil buffer의 format을 나타낸다.
	*/
	DXGI_FORMAT									mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	/*
	* 컴파일된 shader들의 정보를 저장하고 있는 자료구조이다.
	*/
	Shaders										mShaders;
	/*
	* descriptor들을 관리하는 객체이다.
	*/
	static unique_ptr<DescriptorManager>		mDescriptorManager;
	/*
	* resource들을 관리하는 객체이다.
	*/
	static unique_ptr<ResourceManager>			mResourceManager;
	/*
	* depth buffer의 인덱스(handle)를 나타내는 객체이다.
	*/
	int											mDepthBufferIndex;
	/*
	* 여러개의 back buffer중 가장 첫번째의 인덱스(handle)을 나타내는 객체이다.
	*/
	static int									mBackBufferOffset;
	/*
	* noise를 표현하기 위한 perlin noise를 저장하고있는 객체이다.
	*/
	static unique_ptr<PerlinMap>				mPerlinMap;
	/*
	* mesh 정보들을 저장하고 관리하는 객체이다.
	*/
	static unique_ptr<MeshManager>				mMeshManager;
	/*
	* scene들을 저장하고 있는 자료구조이다.
	*/
	static unordered_map<string,shared_ptr<Scene>>	mAllScenes;
	/*
	* 현재 scene의 이름을 나타낸다.
	*/
	static string								mCurrentSceneName;
	/*
	* input을 관리하고 적절하게 처리하는 객체이다.
	*/
	static shared_ptr<InputManager>				mInputManager;
	/*
	* text를 로드하고 관련 정보를 관리하는 객체이다.
	*/
	static shared_ptr<TextManager>				mTextManager;
	/*
	* 물체에 입혀지는 texture만을 관리하는 객체이다.
	*/
	static shared_ptr<TextureManager>			mTextureManager;

private:
	/*
	* 윈도우를 초기화한다.
	*/
	void										InitializeWindow();
	/*
	* debug를 layer를 활성화한다.
	*/
	void										DebugEnable();
	/*
	* numObjConstant의 수만큼 obj constant buffer의 크기를 만든다.
	*/
	void										CreateFrames();
	/*
	* command object를 생성한다.
	*/
	void										CreateCommandObjects();
	/*
	* 각 프레임마다 변화되는 데이터들(scene, input 등)을 모두 업데이트한다.
	*/
	void										Update();
	/*
	* 현재의 scene을 그린다.
	*/
	void										Draw();
	/*
	* 가장 최근에 생성된 윈도우를 나타낸다.
	*/
	static Engine*								mLatestWindow;
	/*
	* HINSTANCE를 저장하는 변수이다.
	*/
	HINSTANCE									mInstance;
	/*
	* window handle을 저장하는 변수이다.
	*/
	HWND										mWindowHandle = nullptr;
	/*
	* window의 caption을 저장하는 변수이다.
	*/
	std::wstring								mWindowCaption = L"Blue Marble";
	/*
	* debug controller이다.
	*/
	ComPtr<ID3D12Debug>							debugController;
};
