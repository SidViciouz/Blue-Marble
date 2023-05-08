#include "Scene.h"
#include "Engine.h"

Scene::Scene() :
	mSceneRoot(make_shared<SceneNode>()),
	mShadowMap(make_shared<ShadowMap>(1024,1024)),
	mPhysicsManager(make_shared<PhysicsManager>())
{

}

void Scene::UpdateScene(const Timer& timer)
{
	static float time = 0;
	for (int i = 0; i < mLightNodes.size(); ++i)
	{
		envFeature.lights[i] = mLightNodes[i]->GetLight();
		//envFeature.lights[i].mPosition = mLightNodes[i]->GetAccumulatedPosition().Get();
	}
	envFeature.view = mCameraNode->GetView();
	envFeature.projection = mCameraNode->GetProjection();
	envFeature.cameraPosition = mCameraNode->GetAccumulatedPosition().v;
	envFeature.cameraFront = mCameraNode->GetFront();
	envFeature.invViewProjection = mCameraNode->GetInvVIewProjection();
	envFeature.currentTime = time;// timer.GetTime();
	time += 0.025f;
	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mEnvConstantBufferIdx, &envFeature, sizeof(env), 0);

	mPhysicsManager->Update();
}

void Scene::DrawScene() const
{
	mSceneRoot->Draw();
}

void Scene::RenderShadowMap() const
{
	mShadowMap->PipelineSetting();
	mSceneRoot->DrawWithoutSetting();
}