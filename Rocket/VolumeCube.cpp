#include "VolumeCube.h"
#include "Engine.h"

VolumeCube::VolumeCube() :
	Volume()
{
	mScale = { 5.0f,5.0f,5.0f };
}

void VolumeCube::Draw()
{
	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(36, 1, 0, 0);
}
