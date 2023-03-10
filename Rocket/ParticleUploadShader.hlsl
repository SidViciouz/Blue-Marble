/*
* depth Map을 이용해서 particle을 생성해 particleMap에 업로드한다.
*/

Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);


[numthreads(32,32,1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	float length = 0.0f;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(id.x, id.y, i*2, 0));
		float exit = depthMap.Load(int4(id.x, id.y, i*2+1, 0));

		length += exit - enter;

	}
	particleMap[int3(id.x, id.y, 0)] = length*32.0f;
}