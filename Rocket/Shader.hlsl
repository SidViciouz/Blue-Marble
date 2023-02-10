struct VertexOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

struct VertexIn
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

cbuffer obj : register(b1)
{
	float4x4 world;
}

cbuffer trans : register(b2)
{
	float4x4 viewProjection;
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4 posW = mul(vin.pos, world);

	//homogeneous clip space로 ps에 보내야함.(SV_POSITION semantic으로 지정된 값)
	vout.pos = mul(posW, viewProjection);

	//non uniform scaling이라고 가정했을때 world matrix를 곱한다. 그렇지 않은 경우에는 inverse-transpose를 이용해야함.
	//빛 계산에 이용할 것이므로 world space로 변환
	vout.normal = mul(vin.normal, (float3x3)world);
	
	//uv좌표계
	vout.tex = vin.tex;

	return vout;
}