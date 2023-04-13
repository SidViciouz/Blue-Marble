#pragma once

#include "InputComponent.h"
#include "Engine.h"

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

			InventoryNode* parentNode = dynamic_cast<InventoryNode*>(node->mParentNode);
			parentNode->SetClickedMesh(node->GetIndex());
			printf("clicked Mesh is %s\n", node->GetMeshName().c_str());
		}
	}

	virtual void								OnMouseUp() override
	{
		if (mIsClicked)
		{
			ItemNode* node = dynamic_cast<ItemNode*>(mNodeAttachedTo.get());
			if (node->GetDraw())
			{
				InventoryNode* parentNode = dynamic_cast<InventoryNode*>(node->mParentNode);
				if (!parentNode->StoreItem(node->GetIndex()))
				{
					parentNode->DropItem(node->GetIndex());
				}
				parentNode->UnsetHaveClickedMesh();
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
				* z��ǥ�� -1�� inventory���� ������� ��ǥ�̴�.
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

		//viewport���� view coordinate���� ��ȯ
		newPos.x = (2.0f * x / (float)Engine::mWidth - 1.0f) / p00 *4.0f;
		newPos.y = (-2.0f * y / (float)Engine::mHeight + 1.0f) / p11 *4.0f;
		newPos.z = z;

		x = newPos.x;
		y = newPos.y;
		z = newPos.z;
	}
};

