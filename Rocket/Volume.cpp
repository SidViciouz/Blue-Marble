#include "Volume.h"

Volume::Volume()
	:Model(-1, nullptr, nullptr) // Load를 사용하지 않기 때문에 어떤 값을 넣어도 상관없다.
	, mTextureResource(make_unique<TextureResource>())
{
}

void Volume::Draw()
{

}

int Volume::volumeIndex = 0;
