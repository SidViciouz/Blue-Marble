struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

/*
struct Coord
{
	float x;
	float y;
};
*/

/*
Coord coord[6] = {
	{-1.0f,1.0f},
	{1.0f,1.0f},
	{1.0f,-1.0f},
	{-1.0f,1.0f},
	{1.0f,-1.0f},
	{-1.0f,-1.0f}
};
*/
VertexOut VS( uint vertexId : SV_VertexID )
{
	VertexOut vout;

	if (vertexId == 0)
	{
		vout.pos = float4(-1.0f,1.0f,0.0f,1.0f);
	}
	else if (vertexId == 1)
	{
		vout.pos = float4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (vertexId == 2)
	{
		vout.pos = float4(1.0f, -1.0f, 0.0f, 1.0f);
	}
	else if (vertexId == 3)
	{
		vout.pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (vertexId == 4)
	{
		vout.pos = float4(1.0f, -1.0f, 0.0f, 1.0f);
	}
	else
	{
		vout.pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
	}

	vout.color = float4(0.5f, 0.5f, 0.5f, 1.0f);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{


	return pin.color;
}