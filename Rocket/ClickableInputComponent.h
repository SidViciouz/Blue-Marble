#pragma once

#include "InputComponent.h"

class ClickableInputComponent : public InputComponent<ClickableInputComponent>
{
public:
												ClickableInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
													: InputComponent<ClickableInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnMouseDown(const int& x, const int& y) override
	{
		mMouseDown = true;
		mPrevMousePosition = { x, y };
	}

	virtual void								OnMouseUp() override
	{
		mMouseDown = false;
	}

	virtual void								OnMouseMove(const int& x, const int& y) override
	{
		if (mMouseDown)
		{
			SceneNode* node = mNodeAttachedTo.get();
			CameraNode* camera = dynamic_cast<CameraNode*>(node);

			camera->TurnX(XMConvertToRadians(0.25f * static_cast<float>(y - mPrevMousePosition.y)));
			camera->TurnY(XMConvertToRadians(0.25f * static_cast<float>(x - mPrevMousePosition.x)));

			mPrevMousePosition = { x, y};
		}
	}

protected:
	bool										mMouseDown = false;
	PrevMousePosition							mPrevMousePosition;
};
