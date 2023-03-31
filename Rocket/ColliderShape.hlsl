struct Light
{
	float3 position;
	int type;
	float3 direction;
	int pad1;
	float3 color;
	int pad2;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posW : POSITION;
};

struct VertexIn
{
	float3 pos : POSITION;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	float roughness;
	float3 fresnel;
	int pad;
}

cbuffer env : register(b1)
{
	float4x4 view;
	float4x4 projection;
	Light lights[3];
	float3 cameraPosition;
	int pad1;
	float3 cameraFront;
	int pad2;
}

cbuffer size : register(b2)
{
	int width;
	int height;
	int depth;
	int isColliding;
}

VertexOut VS(VertexIn vin, uint id : SV_VertexID)
{

	VertexOut vout;

	if (isColliding == 1 &&( id == 9 || id == 8))
	{
		float4x4 transMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		world[0][3], world[1][3], world[2][3], 1.0f
		};

		float4 posW = mul(float4(vin.pos, 1.0f), transMatrix);

		float4 posH = mul(mul(posW, transpose(view)), transpose(projection));

		vout.pos = posH;

		return vout;
	}
	else if (isColliding == 0 && (id == 9 || id == 8))
	{
		vout.pos = float4(0.0f,0.0f,0.0f,0.0f);

		return vout;
	}

	float4x4 scaledMatrix = {
		width, 0.0f, 0.0f, 0.0f,
		0.0f, height, 0.0f, 0.0f,
		0.0f, 0.0f, depth, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	float4 scaled = mul(float4(vin.pos, 1.0f), scaledMatrix);

	float4 posW = mul(scaled, transpose(world));

	float4 posH = mul(mul(posW, transpose(view)), transpose(projection));

	vout.pos = posH;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	if (isColliding)
		return float4(0.0f,1.0f,0.0f,1.0f);
	else
		return float4(1.0f,0.0f,0.0f,1.0f);
}