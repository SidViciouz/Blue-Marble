/*
* depth Map을 이용해서 particle을 생성해 particleMap에 업로드한다.
*/

Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);

RWTexture2DArray<float> rigidBodyMap : register(u1);

cbuffer constantBuffer : register(b0)
{
	int objIndex;
}

[numthreads(5,5,1)]
void CS(int id : SV_GroupIndex)
{
	int particleIdx = objIndex * 3 * 125 + id * 5 * 3;
	int idx = id % 5;
	int idy = id / 5;
	int particleIdxX;
	int particleIdxY;

	float length = 0.0f;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(idx, idy, i*2, 0));
		float exit = depthMap.Load(int4(idx, idy, i*2+1, 0));

		//length += (exit - enter)*5.0f;
		for (float j = enter; j <= exit; j += 1.0f)
		{
			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX,particleIdxY,0)] = ((float)idx / 5.0f) * 2.0f - 0.8f;
			particleIdx += 1;

			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = ((float)idy / 5.0f) * 2.0f - 0.8f;
			particleIdx += 1;

			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = (j / 5.0f) * 2.0f - 1.0f;
			particleIdx += 1;
		}
	}
}