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
		int data[10] = { 10,11,12,13,14,15,16,17,18,19 };
		mTextureResource->Copy((void*)data, 10, 10, 10, 4);
		++cnt;
	}
	Pipeline::mCommandList->DrawInstanced(36, 1, 0, 0);
}

int VolumeCube::cnt = 0;