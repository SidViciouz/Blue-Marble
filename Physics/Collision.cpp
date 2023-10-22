#include "Collision.h"
#include "../Maths/Maths.h"
#include "../Maths/Plane.h"


bool Physics::GJK(CollisionInfo& collisionInfo)
{
	Point a, b, c, d;

	Vector3 searchDirection = collisionInfo.a->GetTransform().GetPosition() - collisionInfo.b->GetTransform().GetPosition();

	SearchPoint(c, searchDirection, collisionInfo.a, collisionInfo.b);

	searchDirection = -c.difference;

	SearchPoint(b, searchDirection, collisionInfo.a, collisionInfo.b);

	//simplex 내부에 원점이 존재하지 않는 경우
	if (Vector3::Dot(b.difference, c.difference) > 0)
		return false;

	//직선 bc에 수직이고 원점을 향하는 벡터
	searchDirection = Vector3::Cross(Vector3::Cross(c.difference - b.difference, -b.difference), c.difference - b.difference);
	if (searchDirection.Zero())  //직선 bc 위에 원점이 존재하는 경우
	{ 
		searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(1, 0, 0));

		if (searchDirection.Zero())
			searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(0, 0, -1));
	}

	int dimension = 2;

	for (int i = 0; i < 30; i++)
	{
		SearchPoint(a, searchDirection, collisionInfo.a, collisionInfo.b);

		//simplex 내부에 원점이 존재하지 않는 경우
		if (Vector3::Dot(a.difference, searchDirection) < 0)
			return false;

		dimension++;

		if (dimension == 3) 
			UpdateSimplex3(a, b, c, d, dimension, searchDirection);
		else
		{
			dimension = 3;
			if (UpdateSimplex4(a, b, c, d, searchDirection))
			{
				EPA(a, b, c, d, collisionInfo);
				return true;
			}
		}
	}

	return false;
}

void Physics::UpdateSimplex3(Point& a, Point& b, Point& c, Point& d, int& dimension, Vector3& searchDirection)
{
	// 반시계 방향으로 winding된다.

	Vector3 normal = Vector3::Cross(b.difference - a.difference, c.difference - a.difference);

	dimension = 2;
	// 원점이 AB 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(b.difference - a.difference, normal), -a.difference) > 0)
	{
		c = a;
		searchDirection = Vector3::Cross(Vector3::Cross(b.difference - a.difference, -a.difference), b.difference - a.difference);
		return;
	}
	// 원점이 AC 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(normal, c.difference - a.difference), -a.difference) > 0)
	{
		b = a;
		searchDirection = Vector3::Cross(Vector3::Cross(c.difference - a.difference, -a.difference), c.difference - a.difference);
		return;
	}

	dimension = 3;
	//원점이 삼각형 위쪽에 존재
	if (Vector3::Dot(normal, -a.difference) > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDirection = normal;
		return;
	}
	
	//원점이 삼각형 아래쪽에 존재
	d = b;
	b = a;
	searchDirection = -normal;
	return;
}

bool Physics::UpdateSimplex4(Point& a, Point& b, Point& c, Point& d, Vector3& searchDirection)
{
	Vector3 ABC = Vector3::Cross(b.difference - a.difference, c.difference - a.difference);
	Vector3 ACD = Vector3::Cross(c.difference - a.difference, d.difference - a.difference);
	Vector3 ADB = Vector3::Cross(d.difference - a.difference, b.difference - a.difference);

	// 원점이 ABC 바깥에 존재
	if (Vector3::Dot(ABC, -a.difference) > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDirection = ABC;
		return false;
	}

	// 원점이 ACD 바깥에 존재
	if (Vector3::Dot(ACD, -a.difference) > 0)
	{
		b = a;
		searchDirection = ACD;
		return false;
	}

	// 원점이 ADB 바깥에 존재
	if (Vector3::Dot(ADB, -a.difference) > 0)
	{
		c = d;
		d = b;
		b = a;
		searchDirection = ADB;
		return false;
	}

	return true;
}

void Physics::EPA(Point& a, Point& b, Point& c, Point& d, CollisionInfo& collisionInfo)
{
	//face들의 배열, 3개의 점과, 1개의 normal
	Triangle triangle[30];

	triangle[0].Set(a, b, c, (Vector3::Cross(b.difference - a.difference, c.difference - a.difference)).Normalized());
	triangle[1].Set(a, c, d, (Vector3::Cross(c.difference - a.difference, d.difference - a.difference)).Normalized());
	triangle[2].Set(a, d, b, (Vector3::Cross(d.difference - a.difference, b.difference - a.difference)).Normalized());
	triangle[3].Set(b, d, c, (Vector3::Cross(d.difference - b.difference, c.difference - b.difference)).Normalized());


	int numFaces = 4;
	int closestFace = 0;

	// 적당한 횟수만큼 iteration한다. 크면 정확도가 높지만, 느려진다.
	for (int it = 0; it < 30; it++)
	{

		// 원점과 가장 가까운 face를 찾는다.
		closestFace = 0;

		float closestDistance = Vector3::Dot(triangle[0].a.difference, triangle[0].normal);

		for (int i = 1; i < numFaces; i++)
		{
			float distance = Vector3::Dot(triangle[i].a.difference, triangle[i].normal);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestFace = i;
			}
		}


		//원점에서 가장 가까운 face의 normal을 찾는다

		Point p;
		SearchPoint(p, triangle[closestFace].normal, collisionInfo.a, collisionInfo.b);

		if (Vector3::Dot(p.difference, triangle[closestFace].normal) - closestDistance < 0.0001f)
		{
			Vector3 localA, localB, normal;
			float depth;

			CalcCollisionData(localA, localB, depth, normal, triangle, closestFace);

			localA -= collisionInfo.a->GetTransform().GetPosition();
			localB -= collisionInfo.b->GetTransform().GetPosition();

			collisionInfo.AddContactInfo(localA, localB, normal, depth);

			return;
		}

		
		Edge edges[30]; //face를 제거하고 fix해야하는 edge들을 보관
		int numEdges = 0;
		Edge currentEdge;

		//p를 바라보는 모든 삼각형을 찾는다.
		for (int i = 0; i < numFaces; i++)
		{
			if (Vector3::Dot(triangle[i].normal, p.difference - triangle[i].a.difference) > 0)
			{
				// loose edge list에 제거된 삼각형 edge들을 추가한다.
				// 이미 존재하면, 제거한다.
				for (int j = 0; j < 3; j++) //한 face에 대한 3개의 edge
				{
					if (j == 0)
						currentEdge.Set(triangle[i].a, triangle[i].b);
					else if (j == 1)
						currentEdge.Set(triangle[i].b, triangle[i].c);
					else
						currentEdge.Set(triangle[i].c, triangle[i].a);

					bool found = false;
					for (int k = 0; k < numEdges; k++) //현재 edge가 이미 list에 있는지 확인한다
					{
						if (edges[k].second.difference == currentEdge.first.difference && edges[k].first.difference == currentEdge.second.difference)
						{
							edges[k] = edges[--numEdges]; //현재 edge를 list의 마지막 edge로 덮어쓴다

							found = true;

							k = numEdges; //edge는 한번만 공유될 수 있기 때문에 loop를 나간다
						}
					}

					if (!found)
					{
						//현재 edge를 list에 추가한다.
						if (numEdges >= 30) 
							break;

						edges[numEdges] = currentEdge;

						++numEdges;
					}
				}

				//삼각형 i를 list에서 제거
				triangle[i] = triangle[numFaces - 1];

				numFaces--;
				i--;
			}//p가 삼각형 i를 볼 수 있으면 종료
		}

		//polytope를 p를 추가해서 다시 구성한다
		for (int i = 0; i < numEdges; i++)
		{
			if (numFaces >= 30)
				break;

			triangle[numFaces].Set(edges[i].first, edges[i].second, p, Vector3::Cross(edges[i].first.difference - edges[i].second.difference, edges[i].first.difference - p.difference).Normalized());
			
			//ccw winding을 유지하기 위해서 잘못된 normal을 확인한다.
			float bias = 0.000001; // dot 결과가 아주 조금 < 0 인 경우 (원점이 face에 존재해서)
			if (Vector3::Dot(triangle[numFaces].a.difference, triangle[numFaces].normal) + bias < 0)
			{
				triangle[numFaces].Set(triangle[numFaces].b, triangle[numFaces].a, triangle[numFaces].c, -triangle[numFaces].normal);
			}
			numFaces++;
		}
	}

	//가장 최근의 가까운 점을 반환한다

	Point p;
	SearchPoint(p, triangle[closestFace].normal, collisionInfo.a, collisionInfo.b);

	Vector3 localA, localB, normal;
	float depth;

	CalcCollisionData(localA, localB, depth, normal,triangle,closestFace);


	collisionInfo.AddContactInfo(localA, localB, normal, depth);

	return;
}

void Physics::CalcCollisionData(Vector3& A, Vector3& B, float& depth, Vector3& normal, Triangle (& triangle)[30], int face)
{
	Plane closestPlane = Plane::MakePlane(triangle[face].a.difference, triangle[face].b.difference, triangle[face].c.difference);
	Vector3 projectionPoint = closestPlane.GetProjectedPoint(Vector3(0, 0, 0));

	float u, v, w;
	Barycentric(triangle[face].a.difference, triangle[face].b.difference, triangle[face].c.difference, projectionPoint, u, v, w);

	A = triangle[face].a.a * u + triangle[face].b.a * v + triangle[face].c.a * w;
	B = triangle[face].a.b * u + triangle[face].b.b * v + triangle[face].c.b * w;

	Vector3 V = A - B;

	depth = V.Length();

	normal = V.Normalized();
}

void Physics::SearchPoint(Point& point, Vector3& searchDirectionection, PhysicsObject* objectA, PhysicsObject* objectB)
{
	point.b = objectB->GetCollider()->Support(searchDirectionection, objectB->GetTransform());
	point.a = objectA->GetCollider()->Support(-searchDirectionection, objectA->GetTransform());
	point.difference = point.b - point.a;
}
