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
	* InputComponentBase 생성자를 protected로 변경하고 friends로 설정하는 것이 좋을 것 같다.
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
	* input component를 가진 노드가 여기에 모두 등록된다.
	* 현재 scene에 존재하는 node가 아닌 경우 이를 제외하도록 해야한다.
	*/
	vector<pair<shared_ptr<InputComponentBase>,string>>		mInputComponents;
};

