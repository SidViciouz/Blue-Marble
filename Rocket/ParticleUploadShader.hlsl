Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);

float4 VS( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}