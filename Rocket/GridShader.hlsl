RWTexture2DArray<float> particleMap : register(u0);

RWTexture2DArray<float> rigidBodyMap : register(u1);

RWTexture2D<int> rigidInfoMap : register(u2);

RWTexture3D<int> gridMap : register(u3);

[numthreads(512, 1, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	//particleMap[id]
}