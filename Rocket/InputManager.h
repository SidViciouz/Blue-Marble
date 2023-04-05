#pragma once

#include "Util.h"
#include "Constant.h"
#include <queue>

class InputManager
{
public:
												InputManager();
	void										Push(Message msg);
	void										Push(UINT msgType,UINT param1 = 0, UINT param2 = 0, UINT param3 = 0, UINT param4 = 0);
	Message										Pop();
	void										SetKeys(int idx,bool value);
	const bool&									GetKeys(int idx) const;

protected:
	queue<Message>								mMessageQueue;
	bool										keys[256] = { false, };
};
