#include "MainScene.h"
#include "BoxCollisionComponent.h"

MainScene::MainScene()
	: Scene()
{
	Initialize();
}

void MainScene::Initialize()
{
	boxMesh = make_shared<MeshNode>("box");
	boxMesh->SetRelativePosition(4.5f, 5.0f, 0.0f);
	boxMesh->SetRelativeQuaternion(0.0f, sinf(1.0f), 0.0f, cosf(1.0f));
	boxMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(boxMesh, 3.0f, 3.0f, 3.0f);
	boxMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(boxMesh, 1.0f);
	boxMesh->mRigidBodyComponent->AddForce(Vector3(-300.0f, 0.0f, 0.0f), Vector3());

	ballMesh = make_shared<MeshNode>("ball");
	ballMesh->SetRelativePosition(0.0f, 5.0f, 0.0f);
	ballMesh->SetRelativeQuaternion(0.0f, sinf(1.0f), 0.0f, cosf(1.0f));
	ballMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(ballMesh, 3.0f, 3.0f, 3.0f);
	ballMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(ballMesh, 1.0f);
	ballMesh->mRigidBodyComponent->AddForce(Vector3(300.0f, 0.0f, 0.0f), Vector3());
	
	shared_ptr<MeshNode> ballMesh2 = make_shared<MeshNode>("ball");
	ballMesh2->SetRelativePosition(5.0f, 5.0f, 0.0f);
	ballMesh2->mCollisionComponent = make_shared<BoxCollisionComponent>(ballMesh2, 3.0f, 3.0f, 3.0f);
	ballMesh2->mRigidBodyComponent = make_shared<RigidBodyComponent>(ballMesh2, 1.0f);
	ballMesh->AddChild(ballMesh2);

	groundMesh = make_shared<MeshNode>("box");
	groundMesh->SetRelativePosition(0.0f, -5.0f, 0.0f);
	groundMesh->SetScale(1.0f, 1.0f, 1.0f);
	groundMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(groundMesh, 20.0f, 1.0f, 20.0f);
	groundMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(groundMesh, 1.0f);

	shared_ptr<VolumeNode> cloudVolume = make_shared<VolumeNode>(50.0f, 5.0f, 50.0f);
	cloudVolume->SetRelativePosition(9.0f, 30.0f, 0.0f);

	mSceneRoot->AddChild(boxMesh);
	mSceneRoot->AddChild(ballMesh);
	mSceneRoot->AddChild(groundMesh);
	mSceneRoot->AddChild(cloudVolume);
	mSceneRoot->Update();
}

void MainScene::UpdateScene(float deltaTime)
{
	ballMesh->mRigidBodyComponent->Update(deltaTime);
	boxMesh->mRigidBodyComponent->Update(deltaTime);

	mSceneRoot->Update();

	ballMesh->mRigidBodyComponent->AddForce(Vector3(0.0f, -9.8f * 1.0f, 0.0f), Vector3());
	boxMesh->mRigidBodyComponent->AddForce(Vector3(0.0f, -9.8f * 1.0f, 0.0f), Vector3());

	CollisionInfo collisionInfo;
	if (ballMesh->IsColliding(boxMesh.get(), collisionInfo))
	{
		ballMesh->AddRelativePosition((collisionInfo.normal * collisionInfo.penetration * -0.5f).v);
		ballMesh->mRigidBodyComponent->AddImpulse(collisionInfo, boxMesh->mRigidBodyComponent);
	}
	if (boxMesh->IsColliding(ballMesh.get(), collisionInfo))
	{
		boxMesh->AddRelativePosition((collisionInfo.normal * collisionInfo.penetration * -0.5f).v);
		boxMesh->mRigidBodyComponent->AddImpulse(collisionInfo, ballMesh->mRigidBodyComponent);
	}
	if (boxMesh->IsColliding(groundMesh.get(), collisionInfo))
	{
		boxMesh->AddRelativePosition((collisionInfo.normal * collisionInfo.penetration * -0.5f).v);
		boxMesh->mRigidBodyComponent->AddImpulse(collisionInfo, groundMesh->mRigidBodyComponent);
	}
	if (ballMesh->IsColliding(groundMesh.get(), collisionInfo))
	{
		ballMesh->AddRelativePosition((collisionInfo.normal * collisionInfo.penetration * -0.5f).v);
		ballMesh->mRigidBodyComponent->AddImpulse(collisionInfo, groundMesh->mRigidBodyComponent);
	}
}
