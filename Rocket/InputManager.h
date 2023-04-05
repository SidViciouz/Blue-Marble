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

protected:
	queue<Message>								mMessageQueue;
};
