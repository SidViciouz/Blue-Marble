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


	/*
	if (currentLevel == 0)
	{
		//앞쪽 삼각형과 뒤쪽 삼각형 사이에 race condition이 발생한다.
		float d = depthMap[int3(pos.x, pos.y, currentLevel)];
		if(d > pos.z)
			depthMap[int3(pos.x, pos.y, currentLevel)] = pos.z;

		return;
	}
	
	else if (depthMap[int3(pos.x, pos.y, currentLevel-1)] < pos.z)
	{
		float d = depthMap[int3(pos.x, pos.y, currentLevel)];
		if (d > pos.z)
			depthMap[int3(pos.x, pos.y, currentLevel)] = pos.z;
	}
	*/
	return;
}