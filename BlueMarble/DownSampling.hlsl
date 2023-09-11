Texture2D<float3> srcTexture : register (t0);
RWTexture2D<float3> dstTexture : register (u0);

[numthreads(100, 10, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	uint3 srcId = id * 2;
	uint3 lu = srcId + uint3(-1, -1, 0);
	uint3 ru = srcId + uint3(1, -1, 0);
	uint3 ld = srcId + uint3(-1, 1, 0);
	uint3 rd = srcId + uint3(1, 1, 0);

	float3 a = srcTexture.Load(int3(srcId.xy,0)) * 0.5f;
	float3 b = srcTexture.Load(int3(lu.xy,0)) * 0.125f;
	float3 c = srcTexture.Load(int3(ru.xy,0)) * 0.125f;
	float3 d = srcTexture.Load(int3(ld.xy,0)) * 0.125f;
	float3 e = srcTexture.Load(int3(rd.xy,0)) * 0.125f;

	dstTexture[id.xy] = a + b + c + d + e;
}