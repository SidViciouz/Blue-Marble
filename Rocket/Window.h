#pragma once
#include "framework.h"
#include <string>

class Window
{
public:
	Window(HINSTANCE hInstance,const int& width,const int& height);
	void Initialize();
	HWND GetWindowHandle();

private:
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	const int& mWidth;
	const int& mHeight;
	std::wstring mWindowCaption = L"Rocket Game";
};