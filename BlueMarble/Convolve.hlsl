Texture2D<float3> srcTexture : register (t0);
Texture2D<float3> srcTexture2 : register (t1);
RWTexture2D<float3> dstTexture : register (u0);

[numthreads(100, 10, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	dstTexture[id.xy] = srcTexture.Load(int3(id.xy, 0)) + srcTexture2.Load(int3(id.xy/2, 0));
}