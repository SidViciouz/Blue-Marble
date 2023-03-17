Texture2DArray<float> depthMap : register(t0);

cbuffer constant : register(b0)
{
	int currentLevel;
}

float4 VS(float3 pos : POSITION) : SV_POSITION
{
	/*
	* right,left, top, bottom, near, far로 정의되는 육면체를 [-1,1]*[-1,1]*[0,1]의 크기의 육면체로 변환한다.
	*/
	float right = 3.0f;
	float left = -3.0f;
	float top = 3.0f;
	float bottom = -3.0f;
	float near = -3.0f;
	float far = 3.0f;

	float4x4 orthoM = {
		2.0f/(right-left), 0.0f ,0.0f, 0.0f,
		0.0f, 2.0f/(top-bottom),0.0f,0.0f,
		0.0f,0.0f,1.0f/(far-near),0.0f,
		-(right+left)/(right-left),-(top+bottom)/(top-bottom),0.5f,1.0f
	};

	return mul(float4(pos, 1.0f), orthoM);
}

void PS( float4 pos : SV_POSITION)
{
	if (currentLevel == 0)
	{
		return;
	}

	else
	{
		if (depthMap.Load(int4(pos.x, pos.y, currentLevel - 1,0)) > pos.z || abs(depthMap.Load(int4(pos.x, pos.y, currentLevel - 1,0))-pos.z) < 0.001f)
			clip(-1);
	}
	return;
}