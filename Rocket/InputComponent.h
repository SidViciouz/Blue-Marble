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
	* ���콺�� �� ������Ʈ�� ����� ��ü�� Ŭ������ �� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnClick() override {};
	/*
	* ���콺�� ������ �� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnMouseDown(const int& x, const int& y) override {};
	/*
	* ���콺�� �ö�� �� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnMouseUp() override {};
	/*
	* ���콺�� �������� �� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnMouseMove(const int& x, const int& y) override {};
	/*
	* Ű�� �ԷµǾ��� �� �����ϴ� �޼����̴�.
	* Ű ���� key �Ķ���͸� ���� ���޵ȴ�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnKeyDown(const int& key) override {};
	/*
	* ���콺�� �� ������Ʈ�� ����� ��ü�� ������ �����϶� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnMouseHover() override {};
	/*
	* ���콺�� ������ ��ư�� ������ �� �����ϴ� �޼����̴�. �̸� �����ε��ؼ� ����ϸ�ȴ�.
	*/
	virtual void								OnMouseRightDown(const int& x, const int& y) override {};
	/*
	* �� input component�� Ȱ��ȭ�Ѵ�.
	*/
	virtual void								Activate() override { mActivated = true; }
	/*
	* �� input component�� ��Ȱ��ȭ�Ѵ�.
	*/
	virtual void								Deactivate() override { mActivated = false; }


	/*
	* OnClick �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnMouseDown �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnMouseUp �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnMouseMove �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnKeyDown �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnMouseHover �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
	* OnMouseRightDown �޼��尡 �ڽ� Ŭ�������� �����ε�Ǿ����� ���θ� Ȯ���ؼ� true,false�� ��ȯ�Ѵ�.
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
