#pragma once

#include "Util.h"
#include "Constant.h"
#include <queue>
#include "InputComponentBase.h"

class SceneNode;
//class InputComponentBase;

class InputManager
{
public:
												InputManager();
	/*
	* InputComponentBase �����ڸ� protected�� �����ϰ� friends�� �����ϴ� ���� ���� �� ����.
	*/
	template<typename T>
	shared_ptr<InputComponentBase>				Build(shared_ptr<SceneNode> NodeAttachedTo,const string& sceneName)
	{
		shared_ptr<InputComponentBase> newInputComponentBase = make_shared<T>(NodeAttachedTo);

		mInputComponents.push_back({ newInputComponentBase,sceneName});

		return newInputComponentBase;
	}

	void										Push(Message msg);
	void										Push(UINT msgType,int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0);
	Message										Pop();
	void										Clear();

	void										SetKeys(int idx,bool value);
	const bool&									GetKeys(int idx) const;
	void										SetMouseLeftDown(bool value);
	bool										GetMouseLeftDown() const;
	void										SetKeyDown(bool value);
	bool										GetKeyDown() const;

	void										Dispatch();

protected:

	void										Select(int x, int y);

	queue<Message>								mMessageQueue;
	bool										mKeys[256] = { false, };
	bool										mMouseLeftDown = false;
	bool										mkeyDown = false;
	PrevMousePosition							mPrevMousePosition;
	/*
	* input component�� ���� ��尡 ���⿡ ��� ��ϵȴ�.
	* ���� scene�� �����ϴ� node�� �ƴ� ��� �̸� �����ϵ��� �ؾ��Ѵ�.
	*/
	vector<pair<shared_ptr<InputComponentBase>,string>>		mInputComponents;
};

