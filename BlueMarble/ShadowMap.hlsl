struct Light
{
	float3 position;
	int type;
	float3 direction;
	int pad1;
	float3 color;
	int pad2;
	float4x4 lightView;
	float4x4 lightProjection;
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

cbuffer lightNum : register(b2)
{
	int lightIdx;
}


struct VertexIn
{
	float3 pos : POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posW : POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float4 posW = mul(float4(vin.pos, 1.0f), transpose(world));
	vout.posW = posW.xyz;
	vout.pos = mul(mul(posW, transpose(lights[lightIdx].lightView)),
		transpose(lights[lightIdx].lightProjection));
	vout.normal = mul(vin.normal, transpose(world));
	vout.tex = vin.tex;

	return vout;
}

void PS(VertexOut pin)
{
	
}

