#pragma once
#include "Defines.h"
#include "framework.h"
#include "Pipeline.h"
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

class Game
{
	//game ��ü �ܺο��� ����� �� �ִ� �������̽�
public:
												Game(const Game& game) = delete;
												Game(Game&& game) = delete;
	Game&										operator=(const Game& game) = delete;
	Game&										operator=(Game&& game) = delete;

												Game(HINSTANCE hInstance);

												//������ ���� �޼���
	static Game*								Get();
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

	//game ��ü ���������� ���Ǵ� �޼���� ��� ����
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

	//������ ���� ������
	static Game*								mLatestWindow;
	HINSTANCE									mInstance;
	HWND										mWindowHandle = nullptr;
	std::wstring								mWindowCaption = L"Rocket Game";

	int											mWidth = 800;
	int											mHeight = 600;

	Timer										mTimer;

	//shared_ptr�� �ϴ°� ���� �� ����.
	Pipeline									mDirectX;

	vector<unique_ptr<Frame>>					mFrames;
	int											mNumberOfFrames = 3;
	int											mCurrentFrame = 0;


	bool										mIsModelSelected = false;
	string										mSelectedModelName;
	shared_ptr<Model>							mSelectedModel;

	bool										mIsInventorySelected = false;
	
	ComPtr<ID3D12Debug>							debugController;
	unique_ptr<ParticleField>					mParticleField;

};