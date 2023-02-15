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
};

struct VertexIn
{
	float3 pos : POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	int pad1;
}

cbuffer trans : register(b1)
{
	float4x4 viewProjection;
	Light lights[3];
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4 posW = mul(float4(vin.pos,1.0f), transpose(mul(world,viewProjection)));

	//homogeneous clip space로 ps에 보내야함.(SV_POSITION semantic으로 지정된 값)
	//vout.pos = mul(posW, viewProjection);
	vout.pos = posW;

	//non uniform scaling이라고 가정했을때 world matrix를 곱한다. 그렇지 않은 경우에는 inverse-transpose를 이용해야함.
	//빛 계산에 이용할 것이므로 world space로 변환
	vout.normal = mul(vin.normal,transpose((float3x3)world));
	
	//uv좌표계
	vout.tex = vin.tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//normal vector 정규화를 해야하나?
	float3 L = normalize(lights[0].position - pin.pos);
	float factor = max(dot(L, pin.normal),0.0f);

	return float4(factor*lights[0].color*diffuseAlbedo,1.0f);
}