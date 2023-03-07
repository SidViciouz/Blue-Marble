#include "VolumeSphere.h"
#include "Game.h"

void VolumeSphere::Draw()
{
	Game::mCommandList->DrawInstanced(6, 1, 0, 0);
}
