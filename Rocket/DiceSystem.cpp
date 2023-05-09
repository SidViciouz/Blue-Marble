#include "DiceSystem.h"
#include "../Physics/PhysicsSystem.h"
#include "PhysicsComponent.h"

using namespace Physics;

DiceSystem::DiceSystem(shared_ptr<PhysicsManager> physicsManager) :
	mPhysicsManager(physicsManager)
{
	SetRelativePosition(20, -15, 0);

	mDice1 = make_shared<DiceNode>("box");
	mDice1->SetTextureName("dice");
	mDice1->SetPhysicsComponent(mPhysicsManager->BuildCube(mDice1, PhysicsType::Dynamic, Vector3(0, 10, 0), Vector3(1, 1, 1)));

	mDice2 = make_shared<DiceNode>("box");
	mDice2->SetTextureName("dice");
	mDice2->SetPhysicsComponent(mPhysicsManager->BuildCube(mDice2, PhysicsType::Dynamic, Vector3(1, 13, 0), Vector3(1, 1, 1)));

	mDice3 = make_shared<DiceNode>("box");
	mDice3->SetTextureName("dice");
	mDice3->SetPhysicsComponent(mPhysicsManager->BuildCube(mDice3, PhysicsType::Dynamic, Vector3(-0.5, 15, 0), Vector3(1, 1, 1)));

	mGround = make_shared<MeshNode>("box");
	mGround->SetTextureName("stone");
	mGround->SetScale(10.0f, 1.0f, 10.0f);
	mGround->SetPhysicsComponent(mPhysicsManager->BuildCube(mGround, PhysicsType::Static, Vector3(0, 0, 0), Vector3(10, 1, 10), 0, 0));

	AddChild(mDice1);
	AddChild(mDice2);
	AddChild(mDice3);
	AddChild(mGround);
}

void DiceSystem::Roll()
{
	mDice1->GetPhysicsComponent()->ApplyLinearImpulse(Vector3(0, 3, 0));
	mDice1->GetPhysicsComponent()->ApplyAngularImpulse(Vector3(0, 3, 0));

	mDice2->GetPhysicsComponent()->ApplyLinearImpulse(Vector3(0, 3, 0));
	mDice2->GetPhysicsComponent()->ApplyAngularImpulse(Vector3(0, 3, 0));

	mDice3->GetPhysicsComponent()->ApplyLinearImpulse(Vector3(0, 3, 0));
	mDice3->GetPhysicsComponent()->ApplyAngularImpulse(Vector3(0, 3, 0));
}

int	DiceSystem::UpperSide()
{
	return mDice1->UpperSide() + mDice2->UpperSide() + mDice3->UpperSide();
}