#define MAX_NUM_POINT 250

struct Point
{
	float x;
	float y;
};

struct CountryInfo
{
	int countryIndex;
	Point minBound;
	Point maxBound;
	int numOfPoint;
	Point points[MAX_NUM_POINT];
};

cbuffer constant : register(b0)
{
	int countryNum;
}

StructuredBuffer<CountryInfo> countryInfos : register(t0);
RWTexture2D<int> ColorCountry : register(u0);

void swap(inout float a, inout float b)
{
	float c = a;
	a = b;
	b = c;
}

[numthreads(32, 32, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	float idX = id.x / 10.0f - 180.0f; //경도
	float idY = id.y / 10.0f - 90.0f; //위도

	for (int i = 0; i < countryNum; ++i)
	{
		if ((float)idX >= countryInfos[i].minBound.y &&
			(float)idX <= countryInfos[i].maxBound.y &&
			(float)idY >= countryInfos[i].minBound.x &&
			(float)idY <= countryInfos[i].maxBound.x)
		{
			int cnt = 0;

			for (int j = 0; j < countryInfos[i].numOfPoint; ++j)
			{
				float xs = countryInfos[i].points[j].x;
				float ys = countryInfos[i].points[j].y;
				float xl = countryInfos[i + 1].points[j].x;
				float yl = countryInfos[i + 1].points[j].y;

				if(xs > xl)
					swap(xs, xl);
				if (ys > yl)
					swap(ys, yl);

				//if (longitude >= ys && longitude <= yl && latitude <= xl)
				// 다음의 조건을 만족하면서 영역 밖에 있는 경우를 조건에 추가해야함.
				if (idY >= xs && idY < xl && idX <= yl) 
				{
					++cnt;
				}
			}
			
			if (cnt % 2 == 1)
			{
				ColorCountry[id.xy] = countryInfos[i].countryIndex;
				return;
			}
		}	
	}

	ColorCountry[id.xy] = -1;
}