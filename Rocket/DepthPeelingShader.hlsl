RWTexture2DArray<float> depthMap : register(u0);

cbuffer constant : register(b0)
{
	int currentLevel;
}

float4 VS(float3 pos : POSITION) : SV_POSITION
{
	return float4(pos.x,pos.y,pos.z+1.0f ,1.0f);

}

void PS( float4 pos : SV_POSITION)
{
	if (currentLevel == 0)
	{
		float d = depthMap[int3(pos.x, pos.y, currentLevel)];
		if(d > pos.z)
			depthMap[int3(pos.x, pos.y, currentLevel)] = pos.z;

		return;
	}
	else if (depthMap[int3(pos.x, pos.y, currentLevel-1)] < pos.z)
	{
		float d = depthMap[int3(pos.x, pos.y, currentLevel)];
		//if (d > pos.z)
		depthMap[int3(pos.x, pos.y, currentLevel)] = pos.z;
	}
	
	return;
}