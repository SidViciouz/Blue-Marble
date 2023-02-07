#pragma once
#include "framework.h"
#include "Window.h"
#include "DirectX.h"

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
	Window mWindow;
	DirectX mDirectX;
};