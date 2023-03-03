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
	float3 localPos : POSITION;
};

struct VertexIn
{
	float3 pos : POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

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

TextureCube textureCube : register(t0);

SamplerState textureSampler : register(s0);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4x4 WorldMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f,0.0f, 0.0f, 1.0f
	};

	float4 Translated = mul(float4(vin.pos, 1.0f), WorldMatrix);

	Translated += float4(cameraPosition, 0.0f);

	float4 posH = mul(mul(Translated, transpose(view)), transpose(projection));

	vout.pos = posH;
	vout.localPos = vin.pos;
	vout.tex = vin.tex;
	vout.normal = vin.normal;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return textureCube.Sample(textureSampler,pin.localPos);
}
