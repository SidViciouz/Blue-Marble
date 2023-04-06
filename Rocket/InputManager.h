#pragma once

#include "Util.h"
#include "Constant.h"
#include <queue>

class InputManager
{
public:
												InputManager();
	void										Push(Message msg);
	void										Push(UINT msgType,int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0);
	Message										Pop();
	void										Clear();

	void										SetKeys(int idx,bool value);
	const bool&									GetKeys(int idx) const;
	void										SetMouseLeftDown(bool value);
	bool										GetMouseLeftDown() const;

protected:
	queue<Message>								mMessageQueue;
	bool										mKeys[256] = { false, };
	bool										mMouseLeftDown = false;
	PrevMousePosition							mPrevMousePosition;
};

