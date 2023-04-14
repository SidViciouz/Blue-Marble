#include "MainScene.h"
#include "Engine.h"
#include "CameraInputComponent.h"
#include "UIInputComponent.h"
#include "ItemInputComponent.h"

MainScene::MainScene()
	: Scene()
{
	Initialize();
}

void MainScene::Initialize()
{
	boxMesh = make_shared<MeshNode>("box");
	boxMesh->SetTextureName("stone");
	boxMesh->SetRelativePosition(4.5f, 5.0f, 0.0f);
	boxMesh->SetRelativeQuaternion(0.0f, sinf(1.0f), 0.0f, cosf(1.0f));
	boxMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(boxMesh, 3.0f, 3.0f, 3.0f);
	boxMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(boxMesh, 1.0f);
	boxMesh->mRigidBodyComponent->mVelocity.v.x = -3.0f;

	ballMesh = make_shared<MeshNode>("ball");
	ballMesh->SetTextureName("stone");
	ballMesh->SetRelativePosition(0.0f, 5.0f, 0.0f);
	ballMesh->SetRelativeQuaternion(0.0f, sinf(1.0f), 0.0f, cosf(1.0f));
	ballMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(ballMesh, 3.0f, 3.0f, 3.0f);
	ballMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(ballMesh, 1.0f);
	ballMesh->mRigidBodyComponent->mVelocity.v.x = 3.0f;

	groundMesh = make_shared<MeshNode>("box");
	groundMesh->SetTextureName("stone");
	groundMesh->SetRelativePosition(0.0f, -5.0f, 0.0f);
	groundMesh->SetScale(20.0f, 0.1f, 20.0f);
	groundMesh->mCollisionComponent = make_shared<BoxCollisionComponent>(groundMesh, 20.0f, 1.0f, 20.0f);
	groundMesh->mRigidBodyComponent = make_shared<RigidBodyComponent>(groundMesh, 1.0f);

	shared_ptr<VolumeNode> cloudVolume = make_shared<VolumeNode>(50.0f, 5.0f, 50.0f);
	cloudVolume->SetRelativePosition(9.0f, 30.0f, 0.0f);
	
	shared_ptr<CameraNode> camera = make_shared<CameraNode>(800,600);
	camera->mInputComponent = Engine::mInputManager->Build<CameraInputComponent>(camera,"MainScene");
	camera->mCollisionComponent = make_shared<BoxCollisionComponent>(camera, 5.0f, 5.0f, 5.0f);
	mCameraNode = camera;

	inventory = make_shared<InventoryNode>("menu");
	inventory->SetTextureName("backPack");
	inventory->SetRelativePosition(0.0f,2.0f,5.0f);
	inventory->mInputComponent = Engine::mInputManager->Build<UIInputComponent>(inventory,"MainScene");
	inventory->SetIsShowUp(false);
	camera->AddChild(inventory);

	shared_ptr<TextNode> text1 = make_shared<TextNode>();
	text1->SetText("inventory E");
	text1->SetScale(5.0f, 5.0f, 5.0f);
	text1->SetRelativePosition(-3.5, 3.5f, 11.0f);
	camera->AddChild(text1);

	shared_ptr<LightNode> light1 = make_shared<LightNode>("ball", Directional);
	light1->SetTextureName("sun");
	light1->SetColor( 1.0f,1.0f,1.0f );
	light1->SetRelativePosition(0.0, 10.0f, 0.0f);
	mLightNodes.push_back(light1);

	shared_ptr<LightNode> light2 = make_shared<LightNode>("ball", Directional);
	light2->SetTextureName("sun");
	light2->SetColor(1.0f, 1.0f, 1.0f);
	light2->SetRelativePosition(5.0, 5.0f, 0.0f);
	light2->SetDirection(-1.0f, -1.0f, 0.0f);
	mLightNodes.push_back(light2);

	shared_ptr<LightNode> light3 = make_shared<LightNode>("ball", Directional);
	light3->SetTextureName("sun");
	light3->SetColor(1.0f, 1.0f, 1.0f);
	light3->SetRelativePosition(-5.0, -5.0f, 0.0f);
	light3->SetDirection(1.0f, 0.0f, 0.0f);
	mLightNodes.push_back(light3);
	
	mSceneRoot->AddChild(boxMesh);
	mSceneRoot->AddChild(ballMesh);
	mSceneRoot->AddChild(groundMesh);
	mSceneRoot->AddChild(cloudVolume);
	mSceneRoot->AddChild(camera);
	mSceneRoot->AddChild(light1);
	mSceneRoot->AddChild(light2);
	mSceneRoot->AddChild(light3);
	mSceneRoot->Update();
}

void MainScene::UpdateScene(const Timer& timer)
{
	Scene::UpdateScene(timer);

	ballMesh->mRigidBodyComponent->Update(timer.GetDeltaTime());
	boxMesh->mRigidBodyComponent->Update(timer.GetDeltaTime());

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

	if (mCameraNode->IsColliding(ballMesh.get(), collisionInfo))
	{
		inventory->OverlappedNode(ballMesh);
	}

	if (mCameraNode->IsColliding(boxMesh.get(), collisionInfo))
	{
		inventory->OverlappedNode(boxMesh);
	}

}
