#pragma once

#include "Engine.h"
#include "../Physics/PhysicsSystem.h"
#include "../Physics/PhysicsWorld.h"
#include "DiceSystem.h"
#include "GameState.h"
#include "Character.h"

/*
* ������ ���� scene�̴�.
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
	* �� scene�� ���� ������Ʈ���� ���¸� �ʱ�ȭ�Ѵ�.
	*/
	void										Initialize();
	/*
	* scene�� ��� ������Ʈ���� �����Ӹ��� ������Ʈ�Ѵ�.
	*/
	virtual void								Update(const Timer& timer) override;
	/*
	* Scene�� �׸���.
	*/
	virtual void								Draw() const override;

	void										NextGameState();

	shared_ptr<DiceSystem>						GetDiceSystem() const;

	shared_ptr<WorldNode>						GetEarth() const;

protected:

	//shared_ptr<InventoryNode>					inventory;
	shared_ptr<WorldNode>						mEarth;
	shared_ptr<DiceSystem>						mDiceSystem;
	shared_ptr<GameState>						mGameState;
	shared_ptr<Character>						mCharacter;

};
