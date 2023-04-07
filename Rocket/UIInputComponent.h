#pragma once

#include "InputComponent.h"

class UIInputComponent : public InputComponent<UIInputComponent>
{
public:
												UIInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<UIInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnClick() override
	{
		printf("UI is clicked\n");
	}
};
