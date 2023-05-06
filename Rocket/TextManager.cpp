#include "TextManager.h"
#include "Engine.h"

TextManager::TextManager()
{
	int error = 0;

	if (error = FT_Init_FreeType(&mLibrary))
	{
		printf("FT_Init_FreeType Error!\n");
	}

	error = FT_New_Face(mLibrary, "../Font/ChrustyRock-ORLA.ttf", 0, &mFace);

	if (error == FT_Err_Unknown_File_Format)
	{
		printf("FT_New_Face unknown file format Error!\n");
	}
	else if (error != 0)
	{
		printf("FT_New_Face Error!\n");
	}

	error = FT_Set_Pixel_Sizes(mFace, 32, 32);
	if (error)
	{
		printf("FT_Set_Pixel_Sizes error!\n");
	}

	//if (mFace->available_sizes == NULL)
	//	printf("available size is NULL\n");


	//font 비트맵을 저장할 텍스처 생성
	mTextureIdx = Engine::mResourceManager->CreateTexture2D(
		256,
		256,
		DXGI_FORMAT_R8_UNORM
	);

	mUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(
		Engine::mResourceManager->CalculateAlignment(256, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
		* 256
	);


	//text info를 저장할 버퍼 생성
	mTextInfoBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(128 * sizeof(TextInfo));
	mTextInfoUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(128 * sizeof(TextInfo));

	
	//텍스처에 데이터 저장
	int offsetX = 0;
	int offsetY = 0;
	for (unsigned char c = 0; c < 128; ++c)
	{
		if (FT_Load_Char(mFace, c, FT_LOAD_RENDER))
			continue;

		int width = mFace->glyph->bitmap.width;
		int height = mFace->glyph->bitmap.rows;
	
		if (offsetX + width >= 256)
		{
			offsetX = 0;
			offsetY += 22;
		}

		Engine::mResourceManager->UploadTexture2D(
			mUploadBufferIdx,
			mFace->glyph->bitmap.buffer,
			width,
			height,
			offsetX,
			offsetY
		);

		mTextInfo[c].offsetX = offsetX;
		mTextInfo[c].offsetY = offsetY;
		mTextInfo[c].width = width;
		mTextInfo[c].height = height;

		offsetX += mFace->glyph->bitmap.width;
	}

	Engine::mResourceManager->Upload(mTextInfoUploadBufferIdx, mTextInfo, 128 * sizeof(TextInfo), 0);


	D3D12_RESOURCE_BARRIER b[2];
	b[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(mTextureIdx),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	b[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(mTextInfoBufferIdx),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	Engine::mCommandList->ResourceBarrier(2, b);


	Engine::mResourceManager->CopyUploadToTexture(
		mUploadBufferIdx,
		mTextureIdx,
		256,
		256,
		1,
		DXGI_FORMAT_R8_UNORM,
		1);
	Engine::mResourceManager->CopyUploadToBuffer(mTextInfoUploadBufferIdx, mTextInfoBufferIdx);

	b[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(mTextureIdx),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	b[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		Engine::mResourceManager->GetResource(mTextInfoBufferIdx),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COMMON
	);
	Engine::mCommandList->ResourceBarrier(2, b);

	mTextureSrvIdx =  Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mTextureIdx),
		DXGI_FORMAT_R8_UNORM,
		D3D12_SRV_DIMENSION_TEXTURE2D
	);
	
	mTextInfoSrvIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mTextInfoBufferIdx),
		DXGI_FORMAT_UNKNOWN,
		D3D12_SRV_DIMENSION_BUFFER,
		1,
		128,
		sizeof(TextInfo)
		);
}

int	TextManager::GetTextureSrvIdx() const
{
	return mTextureSrvIdx;
}

int	TextManager::GetTextInfoSrvIdx() const
{
	return mTextInfoSrvIdx;
}