struct VertexIn
{
	float3 position : POSITION;
	float3 velocity : VELOCITY;
};

struct VertexOut
{
	float4 position : SV_POSITION;
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
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 posW = mul(float4(vin.position, 1.0f), transpose(world));

	vout.position = mul(mul(posW, transpose(view)), transpose(projection));

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return float4(1.0f,0.0f,0.0f,1.0f);
}