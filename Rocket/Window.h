#pragma once
#include "framework.h"
#include <string>

class Window
{
public:
	Window(HINSTANCE hInstance);
	void Initialize();

private:
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	int mWidth = 800;
	int mHeight = 600;
	std::wstring mWindowCaption = L"Rocket Game";
};