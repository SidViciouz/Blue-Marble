#include "VolumeCube.h"
#include "Pipeline.h"

VolumeCube::VolumeCube() :
	Volume(), mTextureResource(make_unique<TextureResource>())
{
	mScale = { 5.0f,5.0f,5.0f };
}

void VolumeCube::Draw()
{
	if (cnt == 0)
	{
		mTextureResource->Copy(nullptr, 10, 10, 10,4);
		++cnt;
	}
	Pipeline::mCommandList->DrawInstanced(36, 1, 0, 0);
}

int VolumeCube::cnt = 0;