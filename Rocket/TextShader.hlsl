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

struct TextInfo
{
	int offsetX;
	int offsetY;
	int width;
	int height;
};

struct Code
{
	int code;
};

cbuffer charNum : register(b2)
{
	int charNum;
}

StructuredBuffer<TextInfo> textInfos : register(t0);
StructuredBuffer<Code> codes : register(t1);
texture2D<float> charMap : register (t2);

SamplerState textureSampler : register(s0);

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXTURE;
};

static float2 coord[6] = {
	{-0.5f,0.5f},
	{0.5f,0.5f},
	{0.5f,-0.5f},

	{-0.5f,0.5f},
	{0.5f,-0.5f},
	{-0.5f,-0.5f}
};

static float2 tex[6] = {
	{0.0f,0.0f},
	{1.0f,0.0f},
	{1.0f,1.0f},

	{0.0f,0.0f},
	{1.0f,1.0f},
	{0.0f,1.0f},
};

VertexOut VS(uint vertexId : SV_VertexID)
{
	VertexOut vout;

	float4 posW = mul(float4(coord[vertexId % 6], 0.0f,1.0f), transpose(world));

	vout.pos = mul(mul(posW, transpose(view)), transpose(projection));

	vout.tex = tex[vertexId % 6];

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// 몇번째 문자를 출력할지 index를 tex를 통해서 계산한다

	float nX = pin.tex.x * 10.0f;
	float nY = pin.tex.y * 10.0f;

	int indexX = (int)nX;
	int indexY = (int)nY;

	float xf = nX - (float)indexX;
	float yf = nY - (float)indexY;

	int index = indexX + indexY * 10;

	if (index >= charNum)
	{
		clip(-1);
	}

	TextInfo ti = textInfos[codes[index].code];

	float offsetX = (float)ti.offsetX + xf * ti.width;
	float offsetY = (float)ti.offsetY + yf * ti.height;
	float2 offset = float2(offsetX, offsetY);

	float intensity = charMap.Load(float3(offset,0.0f));

	return float4(intensity, intensity, intensity, 1.0f);
}