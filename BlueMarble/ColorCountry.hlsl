#define MAX_NUM_POINT 250

struct Point
{
	float x;
	float y;
};

struct CountryInfo
{
	int countryIndex;
	int areaIndex;
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
	float idX = (float)id.x * 0.1f - 180.0f; //경도
	float idY = (float)id.y * 0.1f - 90.0f; //위도

	ColorCountry[int2(id.x,1799-id.y)] = -1;

	for (int i = 0; i < countryNum; ++i)
	{
		if ((float)idX >= countryInfos[i].minBound.y &&
			(float)idX <= countryInfos[i].maxBound.y &&
			(float)idY >= countryInfos[i].minBound.x &&
			(float)idY <= countryInfos[i].maxBound.x)
		{
			int cnt = 0;

			int end = i + 1;
			while (countryInfos[end].countryIndex == countryInfos[i].countryIndex &&
				countryInfos[end].areaIndex == countryInfos[i].areaIndex)
				++end;
			for (int index = i; index < end; ++index)
			{
				for (int j = 0; j < countryInfos[index].numOfPoint; ++j)
				{
					float xs, ys, xl, yl;
					//시작점
					if (j == 0)
					{
						if (index == i)
						{
							xs = countryInfos[end-1].points[countryInfos[end - 1].numOfPoint - 1].x;
							ys = countryInfos[end-1].points[countryInfos[end - 1].numOfPoint - 1].y;
						}
						else
						{
							xs = countryInfos[index-1].points[countryInfos[index-1].numOfPoint-1].x;
							ys = countryInfos[index-1].points[countryInfos[index-1].numOfPoint-1].y;
						}
					}
					else
					{
						xs = countryInfos[index].points[j-1].x;
						ys = countryInfos[index].points[j-1].y;
					}

					xl = countryInfos[index].points[j].x;
					yl = countryInfos[index].points[j].y;

					if (xs > xl)
					{
						swap(xs, xl);
						swap(ys, yl);
					}
					//if (longitude >= ys && longitude <= yl && latitude <= xl)
					// 다음의 조건을 만족하면서 영역 밖에 있는 경우를 조건에 추가해야함.
					float yMax = max(ys, yl);
					if (idY > xs && idY <= xl && idX < yMax)
					{
						float tilt = (yl - ys) / (xl - xs);
						float curPointTilt = (idX - ys) / (idY - xs);

						if (tilt > curPointTilt)
							++cnt;
					}
				}
			}

			if (cnt % 2 == 1)
			{
				if(ColorCountry[int2(id.x, 1799 - id.y)] == -1)
					ColorCountry[int2(id.x, 1799 - id.y)] = countryInfos[i].countryIndex;
				return;
			}
			i = end - 1;
		}	
	}
}