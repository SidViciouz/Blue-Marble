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

	//simplex ���ο� ������ �������� �ʴ� ���
	if (Vector3::Dot(b.difference, c.difference) > 0)
		return false;

	//���� bc�� �����̰� ������ ���ϴ� ����
	searchDirection = Vector3::Cross(Vector3::Cross(c.difference - b.difference, -b.difference), c.difference - b.difference);
	if (searchDirection.Zero())  //���� bc ���� ������ �����ϴ� ���
	{ 
		searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(1, 0, 0));

		if (searchDirection.Zero())
			searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(0, 0, -1));
	}

	int dimension = 2;

	for (int i = 0; i < 30; i++)
	{
		SearchPoint(a, searchDirection, collisionInfo.a, collisionInfo.b);

		//simplex ���ο� ������ �������� �ʴ� ���
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
	// �ݽð� �������� winding�ȴ�.

	Vector3 normal = Vector3::Cross(b.difference - a.difference, c.difference - a.difference);

	dimension = 2;
	// ������ AB �ٱ��� ����
	if (Vector3::Dot(Vector3::Cross(b.difference - a.difference, normal), -a.difference) > 0)
	{
		c = a;
		searchDirection = Vector3::Cross(Vector3::Cross(b.difference - a.difference, -a.difference), b.difference - a.difference);
		return;
	}
	// ������ AC �ٱ��� ����
	if (Vector3::Dot(Vector3::Cross(normal, c.difference - a.difference), -a.difference) > 0)
	{
		b = a;
		searchDirection = Vector3::Cross(Vector3::Cross(c.difference - a.difference, -a.difference), c.difference - a.difference);
		return;
	}

	dimension = 3;
	//������ �ﰢ�� ���ʿ� ����
	if (Vector3::Dot(normal, -a.difference) > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDirection = normal;
		return;
	}
	
	//������ �ﰢ�� �Ʒ��ʿ� ����
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

	// ������ ABC �ٱ��� ����
	if (Vector3::Dot(ABC, -a.difference) > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDirection = ABC;
		return false;
	}

	// ������ ACD �ٱ��� ����
	if (Vector3::Dot(ACD, -a.difference) > 0)
	{
		b = a;
		searchDirection = ACD;
		return false;
	}

	// ������ ADB �ٱ��� ����
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
	//face���� �迭, 3���� ����, 1���� normal
	Triangle triangle[30];

	triangle[0].Set(a, b, c, (Vector3::Cross(b.difference - a.difference, c.difference - a.difference)).Normalized());
	triangle[1].Set(a, c, d, (Vector3::Cross(c.difference - a.difference, d.difference - a.difference)).Normalized());
	triangle[2].Set(a, d, b, (Vector3::Cross(d.difference - a.difference, b.difference - a.difference)).Normalized());
	triangle[3].Set(b, d, c, (Vector3::Cross(d.difference - b.difference, c.difference - b.difference)).Normalized());


	int numFaces = 4;
	int closestFace = 0;

	// ������ Ƚ����ŭ iteration�Ѵ�. ũ�� ��Ȯ���� ������, ��������.
	for (int it = 0; it < 30; it++)
	{

		// ������ ���� ����� face�� ã�´�.
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


		//�������� ���� ����� face�� normal�� ã�´�

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

		
		Edge edges[30]; //face�� �����ϰ� fix�ؾ��ϴ� edge���� ����
		int numEdges = 0;
		Edge currentEdge;

		//p�� �ٶ󺸴� ��� �ﰢ���� ã�´�.
		for (int i = 0; i < numFaces; i++)
		{
			if (Vector3::Dot(triangle[i].normal, p.difference - triangle[i].a.difference) > 0)
			{
				// loose edge list�� ���ŵ� �ﰢ�� edge���� �߰��Ѵ�.
				// �̹� �����ϸ�, �����Ѵ�.
				for (int j = 0; j < 3; j++) //�� face�� ���� 3���� edge
				{
					if (j == 0)
						currentEdge.Set(triangle[i].a, triangle[i].b);
					else if (j == 1)
						currentEdge.Set(triangle[i].b, triangle[i].c);
					else
						currentEdge.Set(triangle[i].c, triangle[i].a);

					bool found = false;
					for (int k = 0; k < numEdges; k++) //���� edge�� �̹� list�� �ִ��� Ȯ���Ѵ�
					{
						if (edges[k].second.difference == currentEdge.first.difference && edges[k].first.difference == currentEdge.second.difference)
						{
							edges[k] = edges[--numEdges]; //���� edge�� list�� ������ edge�� �����

							found = true;

							k = numEdges; //edge�� �ѹ��� ������ �� �ֱ� ������ loop�� ������
						}
					}

					if (!found)
					{
						//���� edge�� list�� �߰��Ѵ�.
						if (numEdges >= 30) 
							break;

						edges[numEdges] = currentEdge;

						++numEdges;
					}
				}

				//�ﰢ�� i�� list���� ����
				triangle[i] = triangle[numFaces - 1];

				numFaces--;
				i--;
			}//p�� �ﰢ�� i�� �� �� ������ ����
		}

		//polytope�� p�� �߰��ؼ� �ٽ� �����Ѵ�
		for (int i = 0; i < numEdges; i++)
		{
			if (numFaces >= 30)
				break;

			triangle[numFaces].Set(edges[i].first, edges[i].second, p, Vector3::Cross(edges[i].first.difference - edges[i].second.difference, edges[i].first.difference - p.difference).Normalized());
			
			//ccw winding�� �����ϱ� ���ؼ� �߸��� normal�� Ȯ���Ѵ�.
			float bias = 0.000001; // dot ����� ���� ���� < 0 �� ��� (������ face�� �����ؼ�)
			if (Vector3::Dot(triangle[numFaces].a.difference, triangle[numFaces].normal) + bias < 0)
			{
				triangle[numFaces].Set(triangle[numFaces].b, triangle[numFaces].a, triangle[numFaces].c, -triangle[numFaces].normal);
			}
			numFaces++;
		}
	}

	//���� �ֱ��� ����� ���� ��ȯ�Ѵ�

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
