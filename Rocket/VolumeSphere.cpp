#include "VolumeSphere.h"
#include "Pipeline.h"

void VolumeSphere::Draw()
{
	Pipeline::mCommandList->DrawInstanced(6, 1, 0, 0);
}
