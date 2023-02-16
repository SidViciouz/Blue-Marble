#pragma once
#include "framework.h"
#include "Pipeline.h"
#include "Model.h"
#include <map>
#include "Camera.h"
#include "Constant.h"
#include "Timer.h"

class Game
{
	//game ��ü �ܺο��� ����� �� �ִ� �������̽�
public:
	Game(const Game& game) = delete;
	Game(Game&& game) = delete;
	Game& operator=(const Game& game) = delete;
	Game& operator=(Game&& game) = delete;

	Game(HINSTANCE hInstance);

	//������ ���� �޼���
	static Game* Get();
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void Initialize();

	void Run();


	//game ��ü ���������� ���Ǵ� �޼���� ��� ����
private:
	//������ ���� �޼���
	void InitializeWindow();

	void LoadModel();
	void CreateVertexIndexBuffer();
	void Update();
	void Draw();

	static Game* mLatestWindow;
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	std::wstring mWindowCaption = L"Rocket Game";

	Timer mTimer;

	Pipeline mDirectX;

	int mWidth = 800;
	int mHeight = 600;

	unique_ptr<Buffer> mVertexBuffer = nullptr;
	unique_ptr<Buffer> mIndexBuffer = nullptr;

	vector<Vertex> allVertices;
	vector<uint16_t> allIndices;

	unordered_map<string, unique_ptr<Model>> mModels;
	
	unique_ptr<Camera> mCamera = nullptr;
};