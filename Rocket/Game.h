#pragma once
#include "framework.h"
#include "Pipeline.h"
#include "Model.h"
#include <map>
#include "Camera.h"
#include "Constant.h"
#include "Timer.h"
#include "Scene.h"

class Game
{
	//game 객체 외부에서 사용할 수 있는 인터페이스
public:
	Game(const Game& game) = delete;
	Game(Game&& game) = delete;
	Game& operator=(const Game& game) = delete;
	Game& operator=(Game&& game) = delete;

	Game(HINSTANCE hInstance);

	//윈도우 관련 메서드
	static Game* Get();
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void Initialize();

	void Run();


	//game 객체 내부적으로 사용되는 메서드와 멤버 변수
private:
	//윈도우 관련 메서드
	void InitializeWindow();

	void LoadModel();
	void SetLight();
	void CreateVertexIndexBuffer();
	void Update();
	void Draw();
	void Input();

	static Game* mLatestWindow;
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	std::wstring mWindowCaption = L"Rocket Game";

	int mWidth = 800;
	int mHeight = 600;

	Timer mTimer;

	Pipeline mDirectX;

	vector<unique_ptr<Scene>> mScenes;

	unordered_map<string, unique_ptr<Model>> mModels;
	
	unique_ptr<Camera> mCamera = nullptr;

	trans envFeature;
};