Texture2D<float3> srcTexture : register (t0);
RWTexture2D<float3> dstTexture : register (u0);

static float weight[5] =
{
	0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216
};

[numthreads(100, 10, 1)]
void HorizontalBlurCS( uint3 id : SV_DispatchThreadID )
{
	dstTexture[id.xy] = srcTexture[id.xy] * weight[0];

	for (int i = 1; i < 3; ++i)
	{
		dstTexture[id.xy] += srcTexture[int2(id.x + i,id.y)] * weight[i];
		dstTexture[id.xy] += srcTexture[int2(id.x - i, id.y)] * weight[i];
	}
}

[numthreads(100, 10, 1)]
void VerticalBlurCS(uint3 id : SV_DispatchThreadID)
{
	dstTexture[id.xy] = srcTexture[id.xy] * weight[0];

	for (int i = 1; i < 3; ++i)
	{
		dstTexture[id.xy] += srcTexture[int2(id.x, id.y + i)] * weight[i];
		dstTexture[id.xy] += srcTexture[int2(id.x, id.y - i)] * weight[i];
	}
}