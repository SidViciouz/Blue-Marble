#include "MainScene.h"
#include "Engine.h"
#include "CameraInputComponent.h"
#include "UIInputComponent.h"
#include "ItemInputComponent.h"
#include "HollowSphereVolumeNode.h"
#include "WorldInputComponent.h"
#include "InventoryNode.h"
#include "TextNode.h"
#include "ItemNode.h"
#include "WorldNode.h"
#include "DiceNode.h"
#include "../Physics/PhysicsSystem.h"
#include "../Physics/PhysicsWorld.h"
#include "../Physics/AABBCollider.h"

using namespace Physics;

MainScene::MainScene()
	: Scene()
{
	mPhysicsWorld = make_shared<PhysicsWorld>();
	mPhysicsSystem = make_shared<PhysicsSystem>(mPhysicsWorld);

	mPhysicsSystem->UseGravity(true);

	mBlooming = make_shared<Blooming>(Engine::mWidth, Engine::mHeight);
	Initialize();
	InitWorld();
}

void MainScene::Initialize()
{
	boxMesh = make_shared<DiceNode>("box");
	boxMesh->SetTextureName("dice");
	boxMesh->SetRelativePosition(9.5f, 5.0f, 1.0f);
	boxMesh->SetRelativeQuaternion(0.0f, sinf(2.0f), 0.0f, cosf(2.0f));

	boxMesh2 = make_shared<DiceNode>("box");
	boxMesh2->SetTextureName("dice");
	boxMesh2->SetRelativePosition(9.5f, 5.0f, 1.0f);
	boxMesh2->SetRelativeQuaternion(0.0f, sinf(2.0f), 0.0f, cosf(2.0f));

	ballMesh = make_shared<DiceNode>("box");
	ballMesh->SetTextureName("dice");
	ballMesh->SetRelativePosition(5.0f, 6.0f, 0.0f);
	ballMesh->SetRelativeQuaternion(0.0f, sinf(1.0f), 0.0f, cosf(1.0f));

	groundMesh = make_shared<MeshNode>("box");
	groundMesh->SetTextureName("stone");
	groundMesh->SetRelativePosition(5.0f, -5.0f, 0.0f);
	groundMesh->SetScale(10.0f, 1.0f, 10.0f);


	shared_ptr<CameraNode> camera = make_shared<CameraNode>(800,600);
	//camera->mInputComponent = Engine::mInputManager->Build<CameraInputComponent>(camera,"MainScene");
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
	light1->mObjFeature.diffuseAlbedo = { 10.0f,1.0f,1.0f };
	light1->SetColor( 0.1f,0.1f,0.1f );
	light1->SetRelativePosition(0.0, 10.0f, 0.0f);
	mLightNodes.push_back(light1);

	shared_ptr<LightNode> light2 = make_shared<LightNode>("ball", Directional);
	light2->SetTextureName("sun");
	light2->mObjFeature.diffuseAlbedo = { 10.0f,10.0f,10.0f };
	light2->SetColor(0.1f, 0.1f, 0.1f);
	light2->SetRelativePosition(5.0, 5.0f, 0.0f);
	light2->SetDirection(-1.0f, -1.0f, 0.0f);
	mLightNodes.push_back(light2);

	shared_ptr<LightNode> light3 = make_shared<LightNode>("ball", Directional);
	light3->SetTextureName("sun");
	light3->SetColor(0.1f, 0.1f, 0.1f);
	light3->SetRelativePosition(0.0, 0.0f, 0.0f);
	light3->SetDirection(0.0f, 0.0f, 1.0f);
	mLightNodes.push_back(light3);
	
	worldMesh = make_shared<WorldNode>("ball");
	worldMesh->SetTextureName("earth");
	worldMesh->mObjFeature.diffuseAlbedo = { 1.0f,1.0f,1.0f };
	worldMesh->SetRelativePosition(0.0f, 0.0f, 30.0f);
	//worldMesh->SetRelativeQuaternion(0.0f, sinf(XMConvertToRadians(90.0f)), 0.0f,cosf(XMConvertToRadians(90.0f)));
	worldMesh->SetScale(30.0f, 30.0f, 30.0f);
	worldMesh->mInputComponent = Engine::mInputManager->Build<WorldInputComponent>(worldMesh, "MainScene");

	shared_ptr<HollowSphereVolumeNode> cloud = make_shared<HollowSphereVolumeNode>(35.0f, 30.0f);
	worldMesh->AddChild(cloud);

	mSceneRoot->AddChild(boxMesh);
	mSceneRoot->AddChild(boxMesh2);
	mSceneRoot->AddChild(ballMesh);
	mSceneRoot->AddChild(groundMesh);
	mSceneRoot->AddChild(camera);
	mSceneRoot->AddChild(light1);
	mSceneRoot->AddChild(light2);
	mSceneRoot->AddChild(light3);
	mSceneRoot->AddChild(worldMesh);
	mSceneRoot->Update();
}

void MainScene::UpdateScene(const Timer& timer)
{
	Scene::UpdateScene(timer);

	mPhysicsSystem->Update(Engine::mTimer.GetDeltaTime());

	groundMesh->SetAccumulatedPosition(mFloor->GetTransform().GetPosition().v);
	groundMesh->SetRelativeQuaternion(
		mFloor->GetTransform().GetOrientation().x,
		mFloor->GetTransform().GetOrientation().y,
		mFloor->GetTransform().GetOrientation().z,
		mFloor->GetTransform().GetOrientation().w
	);

	boxMesh->SetAccumulatedPosition(mCube->GetTransform().GetPosition().v);
	boxMesh->SetRelativeQuaternion(
		mCube->GetTransform().GetOrientation().x,
		mCube->GetTransform().GetOrientation().y,
		mCube->GetTransform().GetOrientation().z,
		mCube->GetTransform().GetOrientation().w
	);

	ballMesh->SetAccumulatedPosition(mCube2->GetTransform().GetPosition().v);
	ballMesh->SetRelativeQuaternion(
		mCube2->GetTransform().GetOrientation().x,
		mCube2->GetTransform().GetOrientation().y,
		mCube2->GetTransform().GetOrientation().z,
		mCube2->GetTransform().GetOrientation().w);


	boxMesh2->SetAccumulatedPosition(mCube3->GetTransform().GetPosition().v);
	boxMesh2->SetRelativeQuaternion(
		mCube3->GetTransform().GetOrientation().x,
		mCube3->GetTransform().GetOrientation().y,
		mCube3->GetTransform().GetOrientation().z,
		mCube3->GetTransform().GetOrientation().w
	);

	mSceneRoot->Update();
	
}

void MainScene::DrawScene() const
{
	Scene::DrawScene();

	mBlooming->DownScalePass();
	mBlooming->BrightPass();
	mBlooming->BlurPass();
}

void MainScene::InitWorld()
{
	mCube = AddCubeToWorld(Vector3(0,0,0), Vector3(1, 1, 1));

	mCube2 = AddCubeToWorld(Vector3(1, 3, 0), Vector3(1, 1, 1));

	mCube3 = AddCubeToWorld(Vector3(-0.5, 5, 0), Vector3(1, 1, 1));

	mFloor = AddFloorToWorld(Vector3(0, -20, 0));
}

PhysicsObject* MainScene::AddFloorToWorld(const Vector3& position)
{
	PhysicsObject* floor = new PhysicsObject();

	Vector3 floorSize = Vector3(40, 2, 40);
	AABBCollider* collider = new AABBCollider(floorSize);
	floor->SetCollider((Collider*)collider);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetInverseMass(0);
	floor->InitCubeInertia();

	floor->SetPhysicsType(PhysicsType::Static);

	mPhysicsWorld->AddGameObject(floor);

	return floor;
}

PhysicsObject* MainScene::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool bStatic, float elasticity) {
	PhysicsObject* cube = new PhysicsObject();

	AABBCollider* collider = new AABBCollider(dimensions);

	cube->SetCollider((Collider*)collider);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetInverseMass(inverseMass);
	cube->InitCubeInertia();
	cube->SetElasticity(elasticity);

	if (bStatic) {
		cube->SetPhysicsType(PhysicsType::Static);
	}

	mPhysicsWorld->AddGameObject(cube);

	return cube;
}