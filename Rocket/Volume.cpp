#include "Volume.h"

Volume::Volume()
	:Model(nullptr, nullptr) // Load�� ������� �ʱ� ������ � ���� �־ �������.
	, mTextureResource(make_unique<TextureResource>())
{
}

void Volume::Draw()
{

}

int Volume::volumeIndex = 0;
