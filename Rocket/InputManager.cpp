#include "InputManager.h"

InputManager::InputManager()
{

}

void InputManager::Push(Message msg)
{
	mMessageQueue.push(msg);
}

void InputManager::Push(UINT msgType, int param1, int param2, int param3, int param4)
{
	mMessageQueue.push({msgType,param1,param2,param3,param4});
}

void InputManager::Clear()
{
	while (!mMessageQueue.empty())
		mMessageQueue.pop();
}

void InputManager::SetKeys(int idx,bool value)
{
	if (idx < 256)
		mKeys[idx] = value;
}

const bool& InputManager::GetKeys(int idx) const
{
	if (idx < 256)
		return mKeys[idx];

	return false;
}

void InputManager::SetMouseLeftDown(bool value)
{
	mMouseLeftDown = value;
}

bool InputManager::GetMouseLeftDown() const
{
	return mMouseLeftDown;
}

Message InputManager::Pop()
{
	if (mMessageQueue.empty())
	{
		return { 0 };
	}

	Message front = mMessageQueue.front();
	mMessageQueue.pop();

	return front;
}