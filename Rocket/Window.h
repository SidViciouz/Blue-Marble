#pragma once
#include "framework.h"
#include <string>

class Window
{
public:
	Window(HINSTANCE hInstance,const int& width,const int& height);
	void Initialize();
	HWND GetWindowHandle();
	static Window* Get();
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	static Window* mLatestWindow;
	HINSTANCE mInstance;
	HWND mWindowHandle = nullptr;
	const int& mWidth;
	const int& mHeight;
	std::wstring mWindowCaption = L"Rocket Game";
};