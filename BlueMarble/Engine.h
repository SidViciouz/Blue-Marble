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
	* engine�� ���࿡ �ʿ��� ��� �����͵��� �ʱ�ȭ�Ѵ�.
	*/
	void										Initialize();
	/*
	* engine�� loop�� �����Ų��.
	*/
	void										Run();

	/*
	* engine �ν��Ͻ��� ��ȯ�Ѵ�.
	*/
	static Engine*								Get();
	/*
	* Ű �Է��� ó���Ѵ�.
	*/
	LRESULT										WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/*
	* ���� ������ �������� �����ӿ��� GPU�� ��� ������ ��ĥ ������ ��ٸ���.
	*/
	void										WaitUntilPrevFrameComplete();
	/*
	* device���� ������Ʈ�Ѵ�.
	*/
	void										InitializePipeline();
	/*
	* swapchain, depth buffer, pso���� ��ü���� �����Ѵ�.
	*/
	void										CreateObjects();
	/*
	* shader�� �������ϰ�, root signature�� �����Ѵ�.
	*/
	void										CreateShaderAndRootSignature();
	/*
	* pso�� �����Ѵ�.
	*/
	void										CreatePso();
	/*
	* viewport�� scissor rectangle�� ���� �����Ѵ�.
	*/
	void										SetViewportAndScissor();
	/*
	* ���� scene�� ��ȯ��Ų��.
	*/
	static void									ChangeScene(const string& sceneName);
	/*
	* command list�̴�.
	*/
	static ComPtr<ID3D12GraphicsCommandList>	mCommandList;
	/*
	* command allocator�̴�.
	*/
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator;
	/*
	* �� �����Ӹ��� ������Ʈ�Ǵ� ������ �����ϰ� �ִ� �����̴�.
	*/
	static vector<unique_ptr<Frame>>			mFrames;
	/*
	* frame�� ������ ��Ÿ����.
	*/
	int											mNumberOfFrames = 3;
	/*
	* ���� frame�� ��ȣ�� ��Ÿ����.
	*/
	static int									mCurrentFrame;
	/*
	* ���� ��ü���� �ð��� �����ϴ� ��ü�̴�.
	*/
	static Timer								mTimer;
	/*
	* �������� width�� ��Ÿ����.
	*/
	static int									mWidth;
	/*
	* �������� height�� ��Ÿ����.
	*/
	static int									mHeight;
	/*
	* gpu�� ������ ��ġ�� �ش��Ѵ�.
	*/
	static ComPtr<ID3D12Device>					mDevice;
	/*
	* command queue�̴�.
	*/
	static ComPtr<ID3D12CommandQueue>			mCommandQueue;
	/*
	* fence ��ü�̴�.
	*/
	ComPtr<ID3D12Fence>							mFence = nullptr;
	/*
	* ���� fence�� ���̴�.
	*/
	UINT64										mFenceValue = 0;
	/*
	* ���� back buffer�� ��ȣ�� ��Ÿ����.
	*/
	static int									mCurrentBackBuffer;
	/*
	* pso���� �����ϰ� �ִ� �ڷᱸ���̴�.
	*/
	static PSOs									mPSOs;
	/*
	* viewport������ �����ϰ� �ִ� ��ü�̴�.
	*/
	D3D12_VIEWPORT								mViewport;
	/*
	* scissor rectangle ������ ������ �ִ� ��ü�̴�.
	*/
	D3D12_RECT									mScissor;
	/*
	* root signature���� ������ �����ϰ� �ִ� �ڷᱸ���̴�.
	*/
	static RootSigs								mRootSignatures;
	/*
	* factory ��ü�̴�.
	*/
	static ComPtr<IDXGIFactory4>				mFactory;
	/*
	* back buffer�� format�� ��Ÿ����.
	*/
	static DXGI_FORMAT							mBackBufferFormat;
	/*
	* depth stencil buffer�� format�� ��Ÿ����.
	*/
	DXGI_FORMAT									mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	/*
	* �����ϵ� shader���� ������ �����ϰ� �ִ� �ڷᱸ���̴�.
	*/
	Shaders										mShaders;
	/*
	* descriptor���� �����ϴ� ��ü�̴�.
	*/
	static unique_ptr<DescriptorManager>		mDescriptorManager;
	/*
	* resource���� �����ϴ� ��ü�̴�.
	*/
	static unique_ptr<ResourceManager>			mResourceManager;
	/*
	* depth buffer�� �ε���(handle)�� ��Ÿ���� ��ü�̴�.
	*/
	int											mDepthBufferIndex;
	/*
	* �������� back buffer�� ���� ù��°�� �ε���(handle)�� ��Ÿ���� ��ü�̴�.
	*/
	static int									mBackBufferOffset;
	/*
	* noise�� ǥ���ϱ� ���� perlin noise�� �����ϰ��ִ� ��ü�̴�.
	*/
	static unique_ptr<PerlinMap>				mPerlinMap;
	/*
	* mesh �������� �����ϰ� �����ϴ� ��ü�̴�.
	*/
	static unique_ptr<MeshManager>				mMeshManager;
	/*
	* scene���� �����ϰ� �ִ� �ڷᱸ���̴�.
	*/
	static unordered_map<string,shared_ptr<Scene>>	mAllScenes;
	/*
	* ���� scene�� �̸��� ��Ÿ����.
	*/
	static string								mCurrentSceneName;
	/*
	* input�� �����ϰ� �����ϰ� ó���ϴ� ��ü�̴�.
	*/
	static shared_ptr<InputManager>				mInputManager;
	/*
	* text�� �ε��ϰ� ���� ������ �����ϴ� ��ü�̴�.
	*/
	static shared_ptr<TextManager>				mTextManager;
	/*
	* ��ü�� �������� texture���� �����ϴ� ��ü�̴�.
	*/
	static shared_ptr<TextureManager>			mTextureManager;

private:
	/*
	* �����츦 �ʱ�ȭ�Ѵ�.
	*/
	void										InitializeWindow();
	/*
	* debug�� layer�� Ȱ��ȭ�Ѵ�.
	*/
	void										DebugEnable();
	/*
	* numObjConstant�� ����ŭ obj constant buffer�� ũ�⸦ �����.
	*/
	void										CreateFrames();
	/*
	* command object�� �����Ѵ�.
	*/
	void										CreateCommandObjects();
	/*
	* �� �����Ӹ��� ��ȭ�Ǵ� �����͵�(scene, input ��)�� ��� ������Ʈ�Ѵ�.
	*/
	void										Update();
	/*
	* ������ scene�� �׸���.
	*/
	void										Draw();
	/*
	* ���� �ֱٿ� ������ �����츦 ��Ÿ����.
	*/
	static Engine*								mLatestWindow;
	/*
	* HINSTANCE�� �����ϴ� �����̴�.
	*/
	HINSTANCE									mInstance;
	/*
	* window handle�� �����ϴ� �����̴�.
	*/
	HWND										mWindowHandle = nullptr;
	/*
	* window�� caption�� �����ϴ� �����̴�.
	*/
	std::wstring								mWindowCaption = L"Blue Marble";
	/*
	* debug controller�̴�.
	*/
	ComPtr<ID3D12Debug>							debugController;
};
