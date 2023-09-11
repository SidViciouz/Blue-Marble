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
#include "DiceSystemInputComponent.h"
#include "TestNode.h"

using namespace Physics;

MainScene::MainScene()
	: Scene()
{
	/*
	mCharacter = make_shared<Character>();
	mCharacter->AddAnimationLayer("Model/ALS_Mannequin_T_Pose.FBX","TPose");
	mCharacter->AddAnimationLayer("Model/ALS_CLF_GetUp_Back.FBX", "GetUpBack");
	mCharacter->AddAnimationLayer("Model/ALS_N_Walk_F.FBX", "WalkF");
	mCharacter->AddAnimationLayer("Model/ALS_N_WalkPose_F.FBX", "WalkPoseF");
	mCharacter->AddAnimationLayer("Model/ALS_N_Run_F.FBX", "RunF");
	mCharacter->AddAnimationLayer("Model/ALS_CLF_Walk_F.FBX", "CLFWalkF");
	
	mCharacter->Initialize("Model/AnimMan.FBX");
	mCharacter->Upload();
	mCharacter->SetScale(0.1f, 0.1f, 0.1f);
	mCharacter->SetRelativePosition(0, -10, -3);
	mCharacter->SetRelativeQuaternion(sinf(XMConvertToRadians(-45.0f)), 0.0f, 0.0f, cosf(XMConvertToRadians(-45.0f)));
	*/
	mGameState = make_shared<GameState>(*this);
	mBlooming = make_shared<Blooming>(Engine::mWidth, Engine::mHeight);
	Initialize();
}

void MainScene::Initialize()
{
	mDiceSystem = make_shared<DiceSystem>(mPhysicsManager);
	mDiceSystem->mInputComponent = Engine::mInputManager->Build<DiceSystemInputComponent>(mDiceSystem,"MainScene");

	shared_ptr<CameraNode> camera = make_shared<CameraNode>(800,600);
	//camera->mInputComponent = Engine::mInputManager->Build<CameraInputComponent>(camera,"MainScene");
	mCameraNode = camera;
	/*
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
	*/
	shared_ptr<LightNode> light1 = make_shared<LightNode>("ball", Directional);
	light1->SetTextureName("sun");
	light1->SetDiffuseAlbedo( 10.0f,1.0f,1.0f);
	light1->SetColor( 0.1f,0.1f,0.1f );
	light1->SetRelativePosition(0.0, 50.0f, 0.0f);
	mLightNodes.push_back(light1);

	shared_ptr<LightNode> light2 = make_shared<LightNode>("ball", Directional);
	light2->SetTextureName("sun");
	light2->SetDiffuseAlbedo( 10.0f,10.0f,10.0f);
	light2->SetColor(0.1f, 0.1f, 0.1f);
	light2->SetRelativePosition(0.0, 0.0f, -50.0f);
	light2->SetDirection(0.0f, 0.0f, 1.0f);
	mLightNodes.push_back(light2);

	shared_ptr<LightNode> light3 = make_shared<LightNode>("ball", Directional);
	light3->SetTextureName("sun");
	light3->SetColor(0.1f, 0.1f, 0.1f);
	light3->SetDiffuseAlbedo(10.0f,10.0f,10.0f );
	light3->SetRelativePosition(40.0, 40.0f, 50.0f);
	light3->SetScale(2.0f, 2.0f, 2.0f);
	light3->SetDirection(-1.0f, -1.0f, 0.0f);
	mLightNodes.push_back(light3);
	
	mEarth = make_shared<WorldNode>("ball");
	mEarth->SetTextureName("earth");
	mEarth->SetDiffuseAlbedo( 1.0f,1.0f,1.0f);
	mEarth->SetRelativePosition(0.0f, 0.0f, 30.0f);
	//worldMesh->SetRelativeQuaternion(0.0f, sinf(XMConvertToRadians(90.0f)), 0.0f,cosf(XMConvertToRadians(90.0f)));
	mEarth->SetScale(30.0f, 30.0f, 30.0f);
	mEarth->mInputComponent = Engine::mInputManager->Build<WorldInputComponent>(mEarth, "MainScene");

	
	shared_ptr<TestNode> lTestNode = make_shared<TestNode>();
	lTestNode->SetScale(1.0f, 1.0f, 1.0f);
	lTestNode->SetDiffuseAlbedo(1.0f, 1.0f, 1.0f);
	lTestNode->SetRelativePosition(5.0f, 5.0f, 0.0f);
	

	shared_ptr<HollowSphereVolumeNode> cloud = make_shared<HollowSphereVolumeNode>(32.0f, 30.0f);
	mEarth->AddChild(cloud);
	
	//mSceneRoot->AddChild(lTestNode);
	//mSceneRoot->AddChild(mCharacter);
	mSceneRoot->AddChild(mDiceSystem);
	mSceneRoot->AddChild(camera);
	mSceneRoot->AddChild(light1);
	mSceneRoot->AddChild(light2);
	mSceneRoot->AddChild(light3);
	mSceneRoot->AddChild(mEarth);
	mSceneRoot->Update();

}

void MainScene::Update(const Timer& timer)
{
	mGameState->Update();

	Scene::Update(timer);

	mSceneRoot->Update();
}

void MainScene::Draw() const
{
	Scene::Draw();

	mBlooming->Pass();
}
void MainScene::NextGameState()
{
	mGameState->Next();
}

shared_ptr<DiceSystem> MainScene::GetDiceSystem() const
{
	return mDiceSystem;
}

shared_ptr<WorldNode> MainScene::GetEarth() const
{
	return mEarth;
}