#pragma once

#include "Engine.h"
#include "../Physics/PhysicsSystem.h"
#include "../Physics/PhysicsWorld.h"
#include "DiceSystem.h"
#include "GameState.h"

//using namespace Physics;
/*
* 게임의 메인 scene이다.
*/

class InventoryNode;
class TextNode;
class ItemNode;
class WorldNode;

using namespace Physics;

class MainScene : public Scene
{
public:
												MainScene();
	/*
	* 이 scene의 각종 오브젝트들의 상태를 초기화한다.
	*/
	void										Initialize();
	/*
	* scene의 모드 오브젝트들을 프레임마다 업데이트한다.
	*/
	virtual void								UpdateScene(const Timer& timer) override;

	virtual void								DrawScene() const override;

	void										NextGameState();

protected:

	shared_ptr<InventoryNode>					inventory;
	shared_ptr<WorldNode>						worldMesh;
	shared_ptr<DiceSystem>						mDiceSystem;
	shared_ptr<GameState>						mGameState;
};
