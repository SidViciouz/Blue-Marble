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

cbuffer lightNum : register(b2)
{
	int lightIdx;
}


Texture1D<float4> gradients : register(t0);
Texture1D<int> permutation : register(t1);

texture2D<float> shadowMap : register(t2);

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

float smoothstep(float t)
{
	return t * t * (3 - 2 * t);
}

int hash(int x, int y, int z)
{
	return permutation[permutation[permutation[x] + y] + z];
}

float evalDensity(float3 position)
{
	int x0 = (int)floor(position.x) & 127;
	int y0 = (int)floor(position.y) & 127;
	int z0 = (int)floor(position.z) & 127;

	int x1 = (x0 + 1) & 127;
	int y1 = (y0 + 1) & 127;
	int z1 = (z0 + 1) & 127;

	float tx = position.x - (int)floor(position.x);
	float ty = position.y - (int)floor(position.y);
	float tz = position.z - (int)floor(position.z);

	float u = smoothstep(tx);
	float v = smoothstep(ty);
	float w = smoothstep(tz);

	float3 c000 = gradients[hash(x0, y0, z0)].xyz;
	float3 c100 = gradients[hash(x1, y0, z0)].xyz;
	float3 c010 = gradients[hash(x0, y1, z0)].xyz;
	float3 c110 = gradients[hash(x1, y1, z0)].xyz;
	float3 c001 = gradients[hash(x0, y0, z1)].xyz;
	float3 c101 = gradients[hash(x1, y0, z1)].xyz;
	float3 c011 = gradients[hash(x0, y1, z1)].xyz;
	float3 c111 = gradients[hash(x1, y1, z1)].xyz;

	float u0 = tx;
	float u1 = tx - 1;
	float v0 = ty;
	float v1 = ty - 1;
	float w0 = tz;
	float w1 = tz - 1;

	float3 p000 = float3(u0, v0, w0);
	float3 p100 = float3(u1, v0, w0);
	float3 p010 = float3(u0, v1, w0);
	float3 p110 = float3(u1, v1, w0);
	float3 p001 = float3(u0, v0, w1);
	float3 p101 = float3(u1, v0, w1);
	float3 p011 = float3(u0, v1, w1);
	float3 p111 = float3(u1, v1, w1);

	float a = lerp(dot(c000, p000), dot(c100, p100), u);
	float b = lerp(dot(c010, p010), dot(c110, p110), u);
	float c = lerp(dot(c001, p001), dot(c101, p101), u);
	float d = lerp(dot(c011, p011), dot(c111, p111), u);

	float e = lerp(a, b, v);
	float f = lerp(c, d, v);

	return lerp(e, f, w);
}

float noise(float3 position)
{
	float noiseSum = 0.0f;
	float amplitude = 1.0f;
	float frequency = 1.0f;
	int layers = 7;

	position.x += currentTime * 10.0f;

	for (int i = 0; i < layers; ++i)
	{
		noiseSum += evalDensity(position * frequency) * amplitude;
		amplitude *= 2.0f;
		frequency *= 0.5f;
	}

	return noiseSum / (float)layers;
}

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

	pt.edgeTess[0] = 15;
	pt.edgeTess[1] = 15;
	pt.edgeTess[2] = 15;
	pt.insideTess = 15;

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


struct GeoOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 posW : POSITION;
	float3 posL : POSITIONL;
	uint id : SV_PrimitiveID;
	float4 lightTex : LIGHTTEX;
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
		float height = max(noise(float3(gin[i].tex*256, 0.0f)),0);
		gout.posL = gin[i].posL;
		gout.posW = gin[i].posW + gin[i].normal * height;
		gout.pos = mul(mul(float4(gout.posW,1.0f), transpose(view)), transpose(projection));
		gout.normal = gin[i].normal;
		gout.id = id;
		gout.lightTex = mul(mul(float4(gout.posW, 1.0f), transpose(lights[lightIdx].lightView)),
			transpose(lights[lightIdx].lightProjection));
		gout.lightTex = mul(gout.lightTex, toTexCoord);

		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
	float3 L = { 0.0f,0.0f,1.0f };
	float rambertTerm = 0.0f;
	float4 color = float4(diffuseAlbedo * float3(0.1f, 0.1f, 0.1f), 0.0f);
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

		color += float4(rambertTerm * lights[i].color * (diffuseAlbedo + fresnelTerm * roughnessTerm), 0.0f);
	}

	float4 lightNDCPixel = mul(mul(float4(pin.posW, 1.0f), transpose(lights[lightIdx].lightView)),
		transpose(lights[lightIdx].lightProjection));

	if (shadowMap.Sample(textureSampler, pin.lightTex) < lightNDCPixel.z)
		color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	return color;
}