#pragma once
#include "framework.h"
#include "Window.h"
#include "Pipeline.h"
#include "Model.h"
#include <map>
#include "Camera.h"
#include "Constant.h"

class Game
{
	//game ��ü �ܺο��� ����� �� �ִ� �������̽�
public:
	Game(const Game& game) = delete;
	Game(Game&& game) = delete;
	Game& operator=(const Game& game) = delete;
	Game& operator=(Game&& game) = delete;

	Game(HINSTANCE hInstance);

	void Initialize();

	void Run();


	//game ��ü ���������� ���Ǵ� �޼���� ��� ����
private:
	void LoadModel();
	void CreateVertexIndexBuffer();
	void Update();
	void Draw();

	Window mWindow;

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