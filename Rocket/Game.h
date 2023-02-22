#pragma once
#include "framework.h"
#include "Pipeline.h"
#include "Model.h"
#include <map>
#include "Camera.h"
#include "Constant.h"
#include "Timer.h"
#include "Scene.h"
#include "Entity.h"

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
	void ChangeScene(int dstScene);
	void SelectObject(int x,int y);

	static vector<unique_ptr<Scene>> mScenes;

	//game ��ü ���������� ���Ǵ� �޼���� ��� ����
private:
	//������ ���� �޼���
	void InitializeWindow();

	int LoadScene();
	unique_ptr<Models> LoadModel(int sceneIndex);
	trans SetLight();
	void Update();
	void Draw();
	void Input();

	//������ ���� ������
	static Game* mLatestWindow;
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	std::wstring mWindowCaption = L"Rocket Game";

	int mWidth = 800;
	int mHeight = 600;

	Timer mTimer;

	//shared_ptr�� �ϴ°� ���� �� ����.
	Pipeline mDirectX;

	int mCurrentScene = 0;

	bool mIsModelSelected = false;
	shared_ptr<Model> mSelectedModel;
};