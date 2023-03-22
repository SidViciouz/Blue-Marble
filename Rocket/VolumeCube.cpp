#include "VolumeCube.h"
#include "Engine.h"

VolumeCube::VolumeCube() :
	Volume()
{
	mScale = { 5.0f,5.0f,5.0f };
}

void VolumeCube::Draw()
{
	Engine::mCommandList->DrawInstanced(36, 1, 0, 0);
}
