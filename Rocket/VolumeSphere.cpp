#include "VolumeSphere.h"
#include "Engine.h"

void VolumeSphere::Draw()
{
	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(6, 1, 0, 0);
}
