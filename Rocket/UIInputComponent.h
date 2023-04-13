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
			InventoryNode* node = dynamic_cast<InventoryNode*>(mNodeAttachedTo.get());
			node->ToggleIsShowUp();

			Engine::mAllScenes[Engine::mCurrentSceneName]->mCameraNode->ToggleActivate();
		}
	}

	virtual void								OnClick() override
	{
		InventoryNode* node = dynamic_cast<InventoryNode*>(mNodeAttachedTo.get());
		if (node->GetIsShowUp())
		{
			//node->StoreItem(node->GetClickedMeshIndex());
		}
	}


	virtual void								OnMouseHover() override
	{
		InventoryNode* node = dynamic_cast<InventoryNode*>(mNodeAttachedTo.get());
		if (node->GetIsShowUp())
		{
			printf("hover on ui input component\n");

		}
	}
};
