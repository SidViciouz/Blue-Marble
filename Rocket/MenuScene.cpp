#include "MenuScene.h"
#include "StartButtonInputComponent.h"

MenuScene::MenuScene()
{
	Initialize();
}

void MenuScene::Initialize()
{
	shared_ptr<CameraNode> camera = make_shared<CameraNode>(800, 600);
	mCameraNode = camera;

	shared_ptr<ClickableNode> menu = make_shared<ClickableNode>("menu");
	menu->SetRelativePosition(0.0f, 0.0f, 15.0f);
	menu->SetScale(12.0f, 12.0f, 12.0f);
	camera->AddChild(menu);

	shared_ptr<ClickableNode> startButton = make_shared<ClickableNode>("inventory");
	startButton->SetRelativePosition(0.0f, 0.0f, 11.0f);
	startButton->SetScale(0.5f, 0.5f, 0.5f);
	startButton->mInputComponent = Engine::mInputManager->Build<StartButtonInputComponent>(startButton,"MenuScene");
	camera->AddChild(startButton);

	shared_ptr<TextNode> text1 = make_shared<TextNode>();
	text1->SetText(" start");
	text1->SetScale(5.0f, 5.0f, 5.0f);
	text1->SetRelativePosition(1.0f, -2.3f, -1.0f);
	startButton->AddChild(text1);

	shared_ptr<LightNode> light1 = make_shared<LightNode>("ball", Directional);
	light1->SetColor(1.0f, 1.0f, 1.0f);
	light1->SetRelativePosition(0.0f, 0.0f, -20.0f);
	light1->SetDirection(0.0f, 0.0f, 1.0f);
	mLightNodes.push_back(light1);

	mSceneRoot->AddChild(camera);
	mSceneRoot->AddChild(light1);
	mSceneRoot->Update();
}

void MenuScene::UpdateScene(const Timer& timer)
{
	Scene::UpdateScene(timer);

	mSceneRoot->Update();
}