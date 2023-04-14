#include "Scene.h"
#include "Engine.h"

Scene::Scene() :
	mSceneRoot(make_shared<SceneNode>()),
	mShadowMap(make_shared<ShadowMap>(500,500))
{

}

void Scene::UpdateScene(const Timer& timer)
{
	for (int i = 0; i < mLightNodes.size(); ++i)
	{
		envFeature.lights[i] = mLightNodes[i]->GetLight();
		//envFeature.lights[i].mPosition = mLightNodes[i]->GetAccumulatedPosition().Get();
	}
	envFeature.view = mCameraNode->GetView();
	envFeature.projection = mCameraNode->GetProjection();
	envFeature.cameraPosition = mCameraNode->GetAccumulatedPosition().Get();
	envFeature.cameraFront = mCameraNode->GetFront();
	envFeature.invViewProjection = mCameraNode->GetInvVIewProjection();
	envFeature.currentTime = timer.GetTime();

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mEnvConstantBufferIdx, &envFeature, sizeof(env), 0);
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