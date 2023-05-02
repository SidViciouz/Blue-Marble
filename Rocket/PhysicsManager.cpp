#include "PhysicsManager.h"
#include "PhysicsComponent.h"
#include "../Physics/AABBCollider.h"
#include "Engine.h"

PhysicsManager::PhysicsManager()
{
	mPhysicsWorld = make_shared<PhysicsWorld>();
	mPhysicsSystem = make_shared<PhysicsSystem>(mPhysicsWorld);
	mPhysicsSystem->UseGravity(true);
}

shared_ptr<PhysicsComponent> PhysicsManager::BuildCube(
	shared_ptr<SceneNode> NodeAttachedTo,
	PhysicsType physicsType,
	const Vector3& position,
	const Vector3& scale,
	float inverseMass,
	float elasticity)
{
	shared_ptr<PhysicsComponent> pc = make_shared<PhysicsComponent>(NodeAttachedTo, physicsType, ColliderType::AABB);
	AABBCollider* collider = new AABBCollider(scale);

	pc->mPhysicsObject->SetCollider((Collider*)collider);
	pc->mPhysicsObject->GetTransform()
		.SetPosition(position)
		.SetScale(scale*2);
	pc->mPhysicsObject->SetInverseMass(inverseMass);
	pc->mPhysicsObject->InitCubeInertia();
	pc->mPhysicsObject->SetElasticity(elasticity);
	pc->mPhysicsObject->SetPhysicsType(physicsType);

	mPhysicsWorld->AddGameObject(pc->mPhysicsObject.get());

	mPhysicsComponents.push_back(pc);

	return pc;
}

void PhysicsManager::Update()
{
	mPhysicsSystem->Update(Engine::mTimer.GetDeltaTime());

	for (auto physicsComponent : mPhysicsComponents)
	{
		const Transform& transform = physicsComponent->mPhysicsObject->GetTransform();
		physicsComponent->mNode->SetAccumulatedPosition(transform.GetPosition().v);
		physicsComponent->mNode->SetRelativeQuaternion(transform.GetOrientation().Get());
	}
}