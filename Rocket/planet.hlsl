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

Texture2D<float4> noiseMap : register(t0);

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

PatchTess ConstantHS(InputPatch<VertexOut,3> patch, uint id : SV_PrimitiveID)
{
	PatchTess pt;

	pt.edgeTess[0] = 3;
	pt.edgeTess[1] = 3;
	pt.edgeTess[2] = 3;
	pt.insideTess = 3;

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
DomainOut DS(PatchTess patchTess, float3 uv : SV_DomainLocation, const OutputPatch<HullOut,3> tri)
{
	DomainOut dout;

	dout.posL = uv.x * tri[0].posL + uv.y * tri[1].posL + uv.z * tri[2].posL;

	dout.tex = uv.x * tri[0].tex + uv.y * tri[1].tex + uv.z * tri[2].tex;

	dout.normal = normalize(uv.x * tri[0].normal + uv.y * tri[1].normal + uv.z * tri[2].normal);

	dout.posW = mul(float4(dout.posL, 1.0f), transpose(world));

	return dout;
}

float smoothstep(float t)
{
	return t * t * (3 - 2 * t);
}

float eval(float2 uv)
{
	int x0 = (int)floor(uv.x) & 127;
	int y0 = (int)floor(uv.y) & 127;

	int x1 = (x0 + 1) & 127;
	int y1 = (y0 + 1) & 127;

	float tx = uv.x - (int)floor(uv.x);
	float ty = uv.y - (int)floor(uv.y);

	float t0 = smoothstep(tx);
	float t1 = smoothstep(ty);

	float4 gradient00 = noiseMap.Load(int3(x0, y0, 0));
	float4 gradient01 = noiseMap.Load(int3(x0, y1, 0));
	float4 gradient10 = noiseMap.Load(int3(x1, y0, 0));
	float4 gradient11 = noiseMap.Load(int3(x1, y1, 0));
	
	float a = lerp(dot(gradient00.xy,float2(uv.x-x0,uv.y-y0)), dot(gradient01.xy, float2(uv.x - x0, uv.y - y1)), t0);
	float b = lerp(dot(gradient10.xy, float2(uv.x - x1, uv.y - y0)), dot(gradient11.xy, float2(uv.x - x1, uv.y - y1)), t0);


	return lerp(a, b, t1);
}

float layerEval(float2 uv)
{
	float fractal = 0.0f;
	float amplitude = 1.0f;

	for (int k = 0; k < 5; ++k) {
		fractal += (1 + eval(uv)) * 0.5 * amplitude;
		uv *= 2.0f;
		amplitude *= 0.2;
	}

	return fractal;
}

struct GeoOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 posW : POSITION;
	float3 posL : POSITIONL;
	uint id : SV_PrimitiveID;
};

[maxvertexcount(3)]
void GS(triangle DomainOut gin[3], uint id : SV_PrimitiveID, inout TriangleStream<GeoOut> triStream)
{
	GeoOut gout;
	for (int i = 0; i < 3; ++i)
	{
		float height = layerEval(gin[i].tex*127.0f)/50.0f;
		gout.posL = gin[i].posL;
		gout.posW = gin[i].posW + gin[i].normal * height;
		gout.pos = mul(mul(float4(gout.posW,1.0f), transpose(view)), transpose(projection));
		gout.normal = gin[i].normal;
		gout.id = id;
		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
	float3 L = { 0.0f,0.0f,1.0f };
	float rambertTerm = 0.0f;
	float4 color = float4(diffuseAlbedo * float3(0.1f, 0.1f, 0.1f), 1.0f);
	float3 fresnelTerm;
	float roughnessTerm;

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

		color += float4(rambertTerm * lights[i].color * (diffuseAlbedo + fresnelTerm * roughnessTerm), 1.0f);
	}

	return color;
}