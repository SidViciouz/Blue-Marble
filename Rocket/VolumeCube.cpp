#include "VolumeCube.h"
#include "Game.h"

VolumeCube::VolumeCube() :
	Volume()
{
	mScale = { 5.0f,5.0f,5.0f };
}

void VolumeCube::Draw()
{
	Game::mCommandList->DrawInstanced(36, 1, 0, 0);
}
