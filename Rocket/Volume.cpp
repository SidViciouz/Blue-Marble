#include "Volume.h"

Volume::Volume()
	:Model(-1, nullptr, nullptr) // Load�� ������� �ʱ� ������ � ���� �־ �������.
	, mTextureResource(make_unique<TextureResource>())
{
}

void Volume::Draw()
{

}

int Volume::volumeIndex = 0;
