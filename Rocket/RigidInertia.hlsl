RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2D<float4> rigidInertiaMap : register(u9);
cbuffer constant : register(b0)
{
	int objNum;
}
[numthreads(1024, 1, 1)]
void CS(int id : SV_GroupIndex)
{
	//if()
}