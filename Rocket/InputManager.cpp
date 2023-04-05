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

Message InputManager::Pop()
{
	Message front = mMessageQueue.front();
	mMessageQueue.pop();

	return front;
}