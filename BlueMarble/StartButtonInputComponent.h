#pragma once

#include "InputComponent.h"
#include "Engine.h"

class StartButtonInputComponent : public InputComponent<StartButtonInputComponent>
{
public:
												StartButtonInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
													: InputComponent<StartButtonInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnClick() override
	{
		printf("start button is clicked\n");
		Engine::ChangeScene("MainScene");
	}
};
