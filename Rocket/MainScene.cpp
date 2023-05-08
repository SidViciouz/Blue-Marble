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
#include "Fbx.h"
#include "AnimationNode.h"

using namespace Physics;

MainScene::MainScene()
	: Scene()
{
	mBlooming = make_shared<Blooming>(Engine::mWidth, Engine::mHeight);
	Initialize();
}

void MainScene::Initialize()
{
	shared_ptr<MeshNode> boxMesh = make_shared<DiceNode>("box");
	boxMesh->SetTextureName("dice");
	boxMesh->SetPhysicsComponent(mPhysicsManager->BuildCube(boxMesh,PhysicsType::Dynamic,Vector3(10,-5,0),Vector3(1,1,1)));

	shared_ptr<MeshNode> boxMesh2 = make_shared<DiceNode>("box");
	boxMesh2->SetTextureName("dice");
	boxMesh2->SetPhysicsComponent(mPhysicsManager->BuildCube(boxMesh2, PhysicsType::Dynamic, Vector3(11, -2, 0), Vector3(1,1,1)));

	shared_ptr<MeshNode> boxMesh3 = make_shared<DiceNode>("box");
	boxMesh3->SetTextureName("dice");
	boxMesh3->SetPhysicsComponent(mPhysicsManager->BuildCube(boxMesh3, PhysicsType::Dynamic, Vector3(9.5, 0, 0), Vector3(1,1,1)));

	shared_ptr<MeshNode> groundMesh = make_shared<MeshNode>("box");
	groundMesh->SetTextureName("stone");
	groundMesh->SetScale(10.0f, 1.0f, 10.0f);
	groundMesh->SetPhysicsComponent(mPhysicsManager->BuildCube(groundMesh, PhysicsType::Static, Vector3(10, -15, 0), Vector3(10, 1, 10),0,0));

	shared_ptr<AnimatedNode> animated = make_shared<AnimatedNode>();
	animated->SetRelativePosition(30, 0, -10);
	animated->SetScale(0.1f, 0.1f, 0.1f);

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
	mSceneRoot->AddChild(boxMesh3);
	mSceneRoot->AddChild(groundMesh);
	mSceneRoot->AddChild(animated);
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

	mSceneRoot->Update();
	
}

void MainScene::DrawScene() const
{
	Scene::DrawScene();

	mBlooming->DownScalePass();
	mBlooming->BrightPass();
	mBlooming->BlurPass();
}