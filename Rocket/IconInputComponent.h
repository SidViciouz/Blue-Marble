#pragma once

#include "InputComponent.h"
#include "Engine.h"

class IconInputComponent : public InputComponent<IconInputComponent>
{
public:
	IconInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<IconInputComponent>(NodeAttachedTo)
	{

	}

	virtual void								OnClick() override
	{
		IconNode* node = dynamic_cast<IconNode*>(mNodeAttachedTo.get());
		if (node->GetDraw())
		{
			printf("icon is clicked\n");
			mIsClicked = true;
		}
	}

	virtual void								OnMouseUp() override
	{
		mIsClicked = false;
	}

	virtual void								OnMouseMove(const int& x, const int& y) override
	{
		IconNode* node = dynamic_cast<IconNode*>(mNodeAttachedTo.get());
		if (node->GetDraw())
		{
			if (mIsClicked)
			{
				printf("x : %d, y : %d\n",x,y);
				float worldX = (float)x;
				float worldY = (float)y;
				float worldZ = 9.0f;
				ScreenToWorld(worldX, worldY, worldZ);
				/*
				* x좌표의 3과 z좌표의 -1은 inventory와의 상대적인 좌표이다.
				*/
				node->SetRelativePosition(worldX + 3.0f, worldY, -1.0f);
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
		newPos.x = (2.0f * x / (float)Engine::mWidth - 1.0f) / p00 *9.0f;
		newPos.y = (-2.0f * y / (float)Engine::mHeight + 1.0f) / p11 *9.0f;
		newPos.z = z;

		x = newPos.x;
		y = newPos.y;
		z = newPos.z;
	}
};

