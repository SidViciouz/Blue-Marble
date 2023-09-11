Texture2D<float3> srcTexture : register (t0);
RWTexture2D<float3> dstTexture : register (u0);

[numthreads(100, 10, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	float3 srcPixel = srcTexture.Load(int3(id.xy, 0));

	dstTexture[id.xy] = 0;

	if (srcPixel.x > 1 || srcPixel.y > 1 || srcPixel.z > 1)
	{
		dstTexture[id.xy] = srcPixel;
	}
}