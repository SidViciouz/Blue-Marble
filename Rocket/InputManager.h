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
	* inputComponent를 상속받는 T타입을 생성한다.
	*/
	template<typename T>
	shared_ptr<InputComponentBase>				Build(shared_ptr<SceneNode> NodeAttachedTo,const string& sceneName)
	{
		shared_ptr<InputComponentBase> newInputComponentBase = make_shared<T>(NodeAttachedTo);

		mInputComponents.push_back({ newInputComponentBase,sceneName});

		return newInputComponentBase;
	}
	/*
	* msg를 넣는다.
	*/
	void										Push(Message msg);
	void										Push(UINT msgType,int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0);
	/*
	* 메세지를 pop한다.
	*/
	Message										Pop();
	/*
	* 메세지를 모두 pop한다.
	*/
	void										Clear();
	/*
	* 특정한 키가 입력되었는지를 확인할 수 있는 keys 배열을 정해진 value로 set한다.
	*/
	void										SetKeys(int idx,bool value);
	/*
	* keys 배열의 element하나의 값을 얻는다.
	*/
	const bool&									GetKeys(int idx) const;
	/*
	* mouse left버튼이 눌렸는지 여부를 value로 설정한다.
	*/
	void										SetMouseLeftDown(bool value);
	/*
	* mouse left버튼이 눌렸는지 여부를 반환한다.
	*/
	bool										GetMouseLeftDown() const;
	/*
	* 아무 키나 입력되었는지 여부를 value로 설정한다.
	*/
	void										SetKeyDown(bool value);
	/*
	* 아무 키나 입력되었는지 여부를 반환한다.
	*/
	bool										GetKeyDown() const;
	/*
	* 큐에 들어있는 모든 메세지들을 각 inputComponent들에 적절하게 분배한다.
	*/
	void										Dispatch();

protected:
	/*
	* 화면 공간의 x,y위치에 있는 오브젝트의 OnClick 메서드를 실행시킨다.
	*/
	void										Select(int x, int y);
	/*
	* 마우스가 어떤 오브젝트의 위를 움직이면 해당 오브젝트의 OnMouseHover 메서드를 실행시킨다.
	*/
	void										Hover(int x, int y);
	/*
	* input message를 저장하고 있는 queue이다.
	*/
	queue<Message>								mMessageQueue;
	/*
	* key값이 눌렸는지 여부를 저장하고 있는 배열이다.
	*/
	bool										mKeys[256] = { false, };
	/*
	* mouse left button이 눌렸는지 여부를 저장한다.
	*/
	bool										mMouseLeftDown = false;
	/*
	* 어떤 키가 눌렸는지 여부를 저장한다.
	*/
	bool										mkeyDown = false;
	/*
	* input component를 가진 노드가 여기에 모두 등록된다.
	*/
	vector<pair<shared_ptr<InputComponentBase>,string>>		mInputComponents;
};

