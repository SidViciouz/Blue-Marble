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
	* inputComponent�� ��ӹ޴� TŸ���� �����Ѵ�.
	*/
	template<typename T>
	shared_ptr<InputComponentBase>				Build(shared_ptr<SceneNode> NodeAttachedTo,const string& sceneName)
	{
		shared_ptr<InputComponentBase> newInputComponentBase = make_shared<T>(NodeAttachedTo);

		mInputComponents.push_back({ newInputComponentBase,sceneName});

		return newInputComponentBase;
	}
	/*
	* msg�� �ִ´�.
	*/
	void										Push(Message msg);
	void										Push(UINT msgType,int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0);
	/*
	* �޼����� pop�Ѵ�.
	*/
	Message										Pop();
	/*
	* �޼����� ��� pop�Ѵ�.
	*/
	void										Clear();
	/*
	* Ư���� Ű�� �ԷµǾ������� Ȯ���� �� �ִ� keys �迭�� ������ value�� set�Ѵ�.
	*/
	void										SetKeys(int idx,bool value);
	/*
	* keys �迭�� element�ϳ��� ���� ��´�.
	*/
	const bool&									GetKeys(int idx) const;
	/*
	* mouse left��ư�� ���ȴ��� ���θ� value�� �����Ѵ�.
	*/
	void										SetMouseLeftDown(bool value);
	/*
	* mouse left��ư�� ���ȴ��� ���θ� ��ȯ�Ѵ�.
	*/
	bool										GetMouseLeftDown() const;
	/*
	* �ƹ� Ű�� �ԷµǾ����� ���θ� value�� �����Ѵ�.
	*/
	void										SetKeyDown(bool value);
	/*
	* �ƹ� Ű�� �ԷµǾ����� ���θ� ��ȯ�Ѵ�.
	*/
	bool										GetKeyDown() const;
	/*
	* ť�� ����ִ� ��� �޼������� �� inputComponent�鿡 �����ϰ� �й��Ѵ�.
	*/
	void										Dispatch();

protected:
	/*
	* ȭ�� ������ x,y��ġ�� �ִ� ������Ʈ�� OnClick �޼��带 �����Ų��.
	*/
	void										Select(int x, int y);
	/*
	* ���콺�� � ������Ʈ�� ���� �����̸� �ش� ������Ʈ�� OnMouseHover �޼��带 �����Ų��.
	*/
	void										Hover(int x, int y);
	/*
	* input message�� �����ϰ� �ִ� queue�̴�.
	*/
	queue<Message>								mMessageQueue;
	/*
	* key���� ���ȴ��� ���θ� �����ϰ� �ִ� �迭�̴�.
	*/
	bool										mKeys[256] = { false, };
	/*
	* mouse left button�� ���ȴ��� ���θ� �����Ѵ�.
	*/
	bool										mMouseLeftDown = false;
	/*
	* � Ű�� ���ȴ��� ���θ� �����Ѵ�.
	*/
	bool										mkeyDown = false;
	/*
	* input component�� ���� ��尡 ���⿡ ��� ��ϵȴ�.
	*/
	vector<pair<shared_ptr<InputComponentBase>,string>>		mInputComponents;
};

