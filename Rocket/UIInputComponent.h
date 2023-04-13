#pragma once

#include "InputComponent.h"

class UIInputComponent : public InputComponent<UIInputComponent>
{
public:
												UIInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<UIInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnKeyDown(const int& key) override
	{
		if (key == 0x45) //E key
		{
			ClickableNode* node = dynamic_cast<ClickableNode*>(mNodeAttachedTo.get());
			node->ToggleIsShowUp();

			Engine::mAllScenes[Engine::mCurrentSceneName]->mCameraNode->ToggleActivate();
		}
	}

};
