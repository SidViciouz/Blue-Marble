#include "MainScene.h"
#include "Engine.h"
#include "CameraInputComponent.h"
#include "HollowSphereVolumeNode.h"
#include "WorldInputComponent.h"
#include "TextNode.h"
#include "WorldNode.h"
#include "DiceNode.h"
#include "AnimationNode.h"
#include "DiceSystemInputComponent.h"

using namespace Physics;

MainScene::MainScene()
	: Scene()
{
	mGameState = make_shared<GameState>(*this);
	mBlooming = make_shared<Blooming>(Engine::mWidth, Engine::mHeight);
	Initialize();
}

void MainScene::Initialize()
{
	mDiceSystem = make_shared<DiceSystem>(mPhysicsManager);
	mDiceSystem->mInputComponent = Engine::mInputManager->Build<DiceSystemInputComponent>(mDiceSystem,"MainScene");

	shared_ptr<CameraNode> camera = make_shared<CameraNode>(800,600);
	mCameraNode = camera;

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
	mEarth->SetScale(30.0f, 30.0f, 30.0f);
	mEarth->mInputComponent = Engine::mInputManager->Build<WorldInputComponent>(mEarth, "MainScene");


	shared_ptr<HollowSphereVolumeNode> cloud = make_shared<HollowSphereVolumeNode>(32.0f, 30.0f);
	mEarth->AddChild(cloud);
	
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