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
	virtual void								OnClick() override {};
	virtual void								OnMouseDown(const int& x, const int& y) override {};
	virtual void								OnMouseUp() override {};
	virtual void								OnMouseMove(const int& x, const int& y) override {};

	virtual bool								IsOnClickOverriden() const override
	{
		if (&InputComponent::OnClick != &Derived::OnClick)
			return true;
		else
			return false;
	}
	virtual bool								IsOnMouseDownOverriden() const override
	{
		if (&InputComponent::OnMouseDown != &Derived::OnMouseDown)
			return true;
		else
			return false;
	}
	virtual bool								IsOnMouseUpOverriden() const override
	{
		if (&InputComponent::OnMouseUp != &Derived::OnMouseUp)
			return true;
		else
			return false;
	}
	virtual bool								IsOnMouseMoveOverriden() const override
	{
		if (&InputComponent::OnMouseMove != &Derived::OnMouseMove)
			return true;
		else
			return false;
	}

};
