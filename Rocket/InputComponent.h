#pragma once

#include "InputComponentBase.h"

template<typename Derived>
class InputComponent : public InputComponentBase
{
public:
												InputComponent(shared_ptr<SceneNode> NodeAttachedTo)
	: InputComponentBase(NodeAttachedTo)
	{

	}
	/*
	* 마우스가 이 컴포넌트가 연결된 객체를 클릭했을 떄 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnClick() override {};
	/*
	* 마우스가 눌렸을 때 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnMouseDown(const int& x, const int& y) override {};
	/*
	* 마우스가 올라올 때 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnMouseUp() override {};
	/*
	* 마우스가 움직였을 때 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnMouseMove(const int& x, const int& y) override {};
	/*
	* 키가 입력되었을 떄 실행하는 메서드이다.
	* 키 값은 key 파라미터를 통해 전달된다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnKeyDown(const int& key) override {};
	/*
	* 마우스가 이 컴포넌트가 연결된 객체의 위에서 움직일때 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnMouseHover() override {};
	/*
	* 마우스가 오른쪽 버튼이 눌렸을 때 실행하는 메서드이다. 이를 오버로드해서 사용하면된다.
	*/
	virtual void								OnMouseRightDown(const int& x, const int& y) override {};
	/*
	* 이 input component를 활성화한다.
	*/
	virtual void								Activate() override { mActivated = true; }
	/*
	* 이 input component를 비활성화한다.
	*/
	virtual void								Deactivate() override { mActivated = false; }


	/*
	* OnClick 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnClickOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnClick != &Derived::OnClick)
			return true;
		else
			return false;
	}
	/*
	* OnMouseDown 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnMouseDownOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnMouseDown != &Derived::OnMouseDown)
			return true;
		else
			return false;
	}
	/*
	* OnMouseUp 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnMouseUpOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnMouseUp != &Derived::OnMouseUp)
			return true;
		else
			return false;
	}
	/*
	* OnMouseMove 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnMouseMoveOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnMouseMove != &Derived::OnMouseMove)
			return true;
		else
			return false;
	}
	/*
	* OnKeyDown 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnKeyDownOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnKeyDown != &Derived::OnKeyDown)
			return true;
		else
			return false;
	}
	/*
	* OnMouseHover 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnMouseHoverOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnMouseHover != &Derived::OnMouseHover)
			return true;
		else
			return false;
	}
	/*
	* OnMouseRightDown 메서드가 자식 클래스에서 오버로드되었는지 여부를 확인해서 true,false를 반환한다.
	*/
	virtual bool								IsOnMouseRightDownOverriden() const override
	{
		if (!mActivated)
			return false;

		if (&InputComponent::OnMouseRightDown != &Derived::OnMouseRightDown)
			return true;
		else
			return false;
	}
};
