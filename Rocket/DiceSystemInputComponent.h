#pragma once

#include "InputComponent.h"
#include "Engine.h"
#include "DiceSystem.h"

class DiceSystemInputComponent : public InputComponent<DiceSystemInputComponent>
{
public:
	DiceSystemInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<DiceSystemInputComponent>(NodeAttachedTo)
	{
		mNode = dynamic_cast<DiceSystem*>(NodeAttachedTo.get());
	}

	virtual void								OnMouseDown(const int& x, const int& y) override
	{
		mNode->Roll();
	}

protected:

	DiceSystem* mNode;
};
