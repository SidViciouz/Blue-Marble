#pragma once
#include "framework.h"
#include "Window.h"
#include "Pipeline.h"
#include "Model.h"
#include <map>
#include "Camera.h"

class Game
{
	//game 객체 외부에서 사용할 수 있는 인터페이스
public:
	Game(const Game& game) = delete;
	Game(Game&& game) = delete;
	Game& operator=(const Game& game) = delete;
	Game& operator=(Game&& game) = delete;

	Game(HINSTANCE hInstance);

	void Initialize();

	void Run();


	//game 객체 내부적으로 사용되는 메서드와 멤버 변수
private:
	void LoadModel();
	void CreateVertexIndexBuffer();

	Window mWindow;

	Pipeline mDirectX;

	int mWidth = 800;
	int mHeight = 600;

	unique_ptr<Buffer> mVertexBuffer = nullptr;
	unique_ptr<Buffer> mIndexBuffer = nullptr;

	unordered_map<string, unique_ptr<Model>> mModels;
	
	unique_ptr<Camera> mCamera = nullptr;
};