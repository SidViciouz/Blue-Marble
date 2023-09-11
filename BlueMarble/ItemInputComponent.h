#pragma once

#include "InputComponent.h"
#include "Engine.h"
#include "ItemNode.h"
#include "InventoryNode.h"

class ItemInputComponent : public InputComponent<ItemInputComponent>
{
public:
	ItemInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<ItemInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnClick() override
	{
		ItemNode* node = dynamic_cast<ItemNode*>(mNodeAttachedTo.get());
		if (node->GetDraw())
		{
			mIsClicked = true;

			
			InventoryNode* parentNode = dynamic_cast<InventoryNode*>(node->mParentNode);;
			printf("clicked Mesh is %s\n", node->GetMeshName().c_str());
			if (node->GetIsStored())
				parentNode->DropItem(node->GetIndex());
			else
				parentNode->StoreItem(node->GetIndex());
			
		}
	}

	virtual void								OnMouseUp() override
	{
		if (mIsClicked)
		{
			ItemNode* node = dynamic_cast<ItemNode*>(mNodeAttachedTo.get());
			if (node->GetDraw())
			{

			}
		}

		mIsClicked = false;
	}

	virtual void								OnMouseMove(const int& x, const int& y) override
	{
		ItemNode* node = dynamic_cast<ItemNode*>(mNodeAttachedTo.get());
		if (node->GetDraw())
		{
			if (mIsClicked)
			{
				float worldX = (float)x;
				float worldY = (float)y;
				float worldZ = 4.0f;
				ScreenToWorld(worldX, worldY, worldZ);
				/*
				* z좌표의 -1은 inventory와의 상대적인 좌표이다.
				*/
				node->SetRelativePosition(worldX, worldY, -1.0f);
			}
		}
	}

protected:
	bool										mIsClicked = false;

	void										ScreenToWorld(float& x,float& y,float& z)
	{
		float p00 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._11;
		float p11 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._22;
		XMFLOAT3 newPos;

		//viewport에서 view coordinate으로 변환
		newPos.x = (2.0f * x / (float)Engine::mWidth - 1.0f) / p00 *4.0f;
		newPos.y = (-2.0f * y / (float)Engine::mHeight + 1.0f) / p11 *4.0f;
		newPos.z = z;

		x = newPos.x;
		y = newPos.y;
		z = newPos.z;
	}
};

