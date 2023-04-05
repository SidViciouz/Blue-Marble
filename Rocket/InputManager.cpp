#include "InputManager.h"

InputManager::InputManager()
{

}

void InputManager::Push(Message msg)
{
	mMessageQueue.push(msg);
}

void InputManager::Push(UINT msgType, UINT param1, UINT param2, UINT param3, UINT param4)
{
	mMessageQueue.push({msgType,param1,param2,param3,param4});
}

void InputManager::SetKeys(int idx,bool value)
{
	if (idx < 256)
		keys[idx] = value;
}

const bool& InputManager::GetKeys(int idx) const
{
	if (idx < 256)
		return keys[idx];

	return false;
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