Texture2DArray<float> depthMap : register(t0);

cbuffer constant : register(b0)
{
	int currentLevel;
}

float4 VS(float3 pos : POSITION) : SV_POSITION
{
	return float4(pos.x,pos.y, (pos.z + 1.0f) * 0.5f,1.0f);

}

void PS( float4 pos : SV_POSITION)
{
	if (currentLevel == 0)
	{
		return;
	}

	else
	{
		if (depthMap[int3(pos.x, pos.y, currentLevel - 1)] > pos.z || abs(depthMap[int3(pos.x, pos.y, currentLevel - 1)]-pos.z) < 0.001f)
			clip(-1);
	}
	return;
}