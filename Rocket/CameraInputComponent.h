#pragma once

#include "InputComponent.h"

class CameraInputComponent : public InputComponent<CameraInputComponent>
{
public:
												CameraInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
													: InputComponent<CameraInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnMouseDown(const int& x, const int& y) override
	{
		mMouseDown = true;
		prevX = x;
		prevY = y;
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
			
			if (!camera->GetActivate())
				return;

			camera->TurnX(XMConvertToRadians(0.25f * static_cast<float>(y - prevY)));
			camera->TurnY(XMConvertToRadians(0.25f * static_cast<float>(x - prevX)));

			prevX = x;
			prevY = y;
		}
	}

protected:
	bool										mMouseDown = false;
	int											prevX;
	int											prevY;
};
