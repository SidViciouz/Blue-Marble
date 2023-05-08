#pragma once

class SceneNode;

class InputComponentBase
{
public:
												InputComponentBase(shared_ptr<SceneNode> NodeAttachedTo)
		: mNodeAttachedTo{ NodeAttachedTo }
	{

	}
	virtual void								OnClick() = 0;
	virtual void								OnMouseDown(const int& x,const int& y) = 0;
	virtual void								OnMouseUp() = 0;
	virtual void								OnMouseMove(const int& x, const int& y) = 0;
	virtual void								OnKeyDown(const int& key) = 0;
	virtual void								OnMouseHover() = 0;
	virtual void								OnMouseRightDown(const int& x, const int& y) = 0;

	virtual bool								IsOnClickOverriden() const = 0;
	virtual bool								IsOnMouseDownOverriden() const = 0;
	virtual bool								IsOnMouseUpOverriden() const = 0;
	virtual bool								IsOnMouseMoveOverriden() const = 0;
	virtual bool								IsOnKeyDownOverriden() const = 0;
	virtual bool								IsOnMouseHoverOverriden() const = 0;
	virtual bool								IsOnMouseRightDownOverriden() const = 0;

	shared_ptr<SceneNode>						mNodeAttachedTo;
};
