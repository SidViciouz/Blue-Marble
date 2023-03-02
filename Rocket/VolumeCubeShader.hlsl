struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 posW : POSITION;
};

struct Coord
{
	float x;
	float y;
	float z;
};

struct Light
{
	float3 position;
	int type;
	float3 direction;
	int pad1;
	float3 color;
	int pad2;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	float roughness;
	float3 fresnel;
	int pad;
}

cbuffer trans : register(b1)
{
	float4x4 view;
	float4x4 projection;
	Light lights[3];
	float3 cameraPosition;
	int pad1;
	float3 cameraFront;
	int pad2;
	float4x4 InvViewProjection;
}

static Coord coord[36] = {
	{1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,1.0f},
	{1.0f,1.0f,1.0f},

	{1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,1.0f},

	{1.0f,1.0f,-1.0f},
	{1.0f,1.0f,1.0f},
	{1.0f,-1.0f,-1.0f},

	{1.0f,-1.0f,-1.0f},
	{1.0f,1.0f,1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,1.0f,-1.0f},
	{1.0f,1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},

	{-1.0f,-1.0f,-1.0f},
	{-1.0f,1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,1.0f,-1.0f},
	{-1.0f,-1.0f,-1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,-1.0f,-1.0f},
	{-1.0f,-1.0f,1.0f},

	{-1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,1.0f},
	{-1.0f,-1.0f,1.0f},

	{1.0f,1.0f,1.0f},
	{-1.0f,1.0f,1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,-1.0f,1.0f},
	{1.0f,-1.0f,1.0f}
};

VertexOut VS( uint vertexId : SV_VertexID)
{
	VertexOut vout;

	vout.posW = mul(float4(coord[vertexId % 36],1.0f), transpose(world));

	vout.pos = mul(mul(vout.posW, transpose(view)), transpose(projection));

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return float4(1.0f,0.0f,0.0f,1.0f);
}