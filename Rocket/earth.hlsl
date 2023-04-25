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
	float currentTime;
	int pad3;
	int pad4;
	int pad5;
}

cbuffer constant : register(b2)
{
	int lightIdx;
}

texture2D<float> shadowMap : register(t0);

texture2D<float4> textureMap : register(t1);

texture2D<float4> heightMap : register(t2);

texture2D<float> borderMap : register(t3);

SamplerState textureSampler : register(s0);

struct PatchTess
{
	float edgeTess[3] : SV_TessFactor;
	float insideTess : SV_InsideTessFactor;
};

struct VertexOut
{
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posL : POSITIONL;
};

struct VertexIn
{
	float3 pos : POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.posL = vin.pos;

	//vout.posW = mul(float4(vin.pos,1.0f), transpose(world));

	vout.normal = mul(vin.normal, (float3x3)transpose(world));

	//uv좌표계
	vout.tex = vin.tex;

	return vout;
}

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch, uint id : SV_PrimitiveID)
{
	PatchTess pt;

	pt.edgeTess[0] = 100;
	pt.edgeTess[1] = 100;
	pt.edgeTess[2] = 100;
	pt.insideTess = 100;

	return pt;
}

struct HullOut
{
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posL : POSITIONL;
};


[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
HullOut HS(InputPatch<VertexOut, 3> patch, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	hout.tex = patch[i].tex;
	hout.normal = patch[i].normal;
	hout.posL = patch[i].posL;

	return hout;
}


struct DomainOut
{
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posL : POSITIONL;
	float3 posW : POSITION;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 uv : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
	DomainOut dout;

	dout.posL = uv.x * tri[0].posL + uv.y * tri[1].posL + uv.z * tri[2].posL;

	dout.tex = uv.x * tri[0].tex + uv.y * tri[1].tex + uv.z * tri[2].tex;

	dout.normal = normalize(uv.x * tri[0].normal + uv.y * tri[1].normal + uv.z * tri[2].normal);

	dout.posW = mul(float4(dout.posL, 1.0f), transpose(world));

	return dout;
}


struct GeoOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 posW : POSITION;
	float3 posL : POSITIONL;
	uint id : SV_PrimitiveID;
	float4 lightTex : LIGHTTEX;
	float2 tex : TEXTURE;
};

[maxvertexcount(3)]
void GS(triangle DomainOut gin[3], uint id : SV_PrimitiveID, inout TriangleStream<GeoOut> triStream)
{
	GeoOut gout;

	float4x4 toTexCoord = {
	0.5f, 0, 0, 0,
	0, -0.5f, 0, 0,
	0, 0, 1.0f, 0,
	0.5f, 0.5f, 0, 1.0f
	};

	for (int i = 0; i < 3; ++i)
	{
		float2 tex = gin[i].tex;
		/*
		float2 tex = gin[i].tex * float2(16383, 10799);
		int2 tex1 = (int2)tex;
		int2 tex2 = tex1 + int2(1, 0);
		float t = tex.x - (float)tex1.x;
		float height1 = heightMap.Load(float3(tex1, 0.0f)).x;
		float height2 = heightMap.Load(float3(tex2, 0.0f)).x;
		float height = ((1 - t) * height1 + t * height2)*5.0f;
		*/
		float height = (heightMap.Load(float3(tex*float2(16383,10799),0.0f)).x)*5.0f;
		gout.posL = gin[i].posL;
		gout.posW = gin[i].posW + gin[i].normal * height;
		gout.pos = mul(mul(float4(gout.posW, 1.0f), transpose(view)), transpose(projection));
		gout.normal = gin[i].normal;
		gout.id = id;
		gout.lightTex = mul(mul(float4(gout.posW, 1.0f), transpose(lights[lightIdx].lightView)),
			transpose(lights[lightIdx].lightProjection));
		gout.lightTex = mul(gout.lightTex, toTexCoord);
		gout.tex = gin[i].tex;

		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
	float3 diffuse = diffuseAlbedo * textureMap.Sample(textureSampler, pin.tex);
	float3 L = { 0.0f,0.0f,1.0f };
	float rambertTerm = 0.0f;
	float4 color = float4(diffuse * float3(0.1f, 0.1f, 0.1f), 0.0f);
	float3 fresnelTerm;
	float roughnessTerm;

	/*
	if (pin.tex.x < 0.05f && pin.tex.y < 0.05f)
	{
		color = float4(1.0f, 0.0f, 0.0f, 0.0f);
		return color;
	}
	else if (pin.tex.x > 0.95f && pin.tex.y < 0.05f)
	{
		color = float4(0.0f, 1.0f, 0.0f, 0.0f);
		return color;
	}
	*/
	pin.normal = normalize(pin.normal);

	for (int i = 0; i < 3; ++i)
	{
		if (lights[i].type == 0) //directional light
		{
			L = -normalize(lights[i].direction);
		}
		else if (lights[i].type == 1) // point light
		{
			L = normalize(lights[i].position - pin.posW);
		}
		else //spot light
		{
			L = normalize(lights[i].position - pin.posW);

			if (dot(-L, lights[i].direction) < cos(3.14f / 6.0f))
			{
				continue;
			}
		}

		//빛의 입사각에 따라 단위면적당 들어오는 빛의 양이 달라짐을 표현
		rambertTerm = max(dot(L, pin.normal), 0.0f);

		//빛의 입사각에 따라 반사되는 빛의 양이 달라지는 것을 표현(반영 조명에서 사용)
		float cosTerm = 1.0f - saturate(dot(pin.normal, L));
		fresnelTerm = fresnel + (1.0f - fresnel) * (cosTerm * cosTerm * cosTerm * cosTerm * cosTerm);

		//표면 거칠기를 표현(반영 조명에서 사용)
		float3 halfway = normalize(normalize(cameraPosition - pin.posW) + L);
		float m = (1.0f - roughness) * 32.0f;
		roughnessTerm = (m + 8.0f) * pow(max(dot(halfway, pin.normal), 0.0f), m) / 8.0f;

		color += float4(rambertTerm * lights[i].color * (diffuse + fresnelTerm * roughnessTerm), 0.0f);
	}

	float4 lightNDCPixel = mul(mul(float4(pin.posW, 1.0f), transpose(lights[lightIdx].lightView)),
		transpose(lights[lightIdx].lightProjection));

	if (shadowMap.Sample(textureSampler, pin.lightTex) < lightNDCPixel.z)
		color *= 0.2f;
	
	float border = borderMap.Sample(textureSampler, pin.tex);

	if (border != 0)
	{
		color.x = border;
		color.y = 0;
		color.z = 0;
	}

	//float4 color = float4(textureMap.Load(float3(pin.tex * float2(16383, 10799), 0.0f)).xyz,0.0f);

	return color;
}