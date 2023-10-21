#include "Collision.h"
#include "../Maths/Maths.h"
#include "../Maths/Plane.h"

#define GJK_MAX_NUM_ITERATIONS 64

bool Physics::GJK(CollisionInfo& collisionInfo)
{
	Vector3 objectAPos = collisionInfo.a->GetTransform().GetPosition();
	Vector3 objectBPos = collisionInfo.b->GetTransform().GetPosition();

	Point a, b, c, d;
	Vector3 searchDirection = objectAPos - objectBPos;

	SearchPoint(c, searchDirection, collisionInfo.a, collisionInfo.b);
	searchDirection = -c.difference;

	SearchPoint(b, searchDirection, collisionInfo.a, collisionInfo.b);

	//simplex 내부에 원점이 존재하지 않는 경우
	if (Vector3::Dot(b.difference, searchDirection) < 0)
		return false;

	//직선 bc에 수직이고 원점을 향하는 벡터
	searchDirection = Vector3::Cross(Vector3::Cross(c.difference - b.difference, -b.difference), c.difference - b.difference);
	if (searchDirection == Vector3(0, 0, 0))  //직선 bc 위에 원점이 존재하는 경우
	{ 
		searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(1, 0, 0));
		if (searchDirection == Vector3(0, 0, 0))
			searchDirection = Vector3::Cross(c.difference - b.difference, Vector3(0, 0, -1));
	}
	int dimension = 2;

	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
	{
		SearchPoint(a, searchDirection, collisionInfo.a, collisionInfo.b);

		//simplex 내부에 원점이 존재하지 않는 경우
		if (Vector3::Dot(a.difference, searchDirection) < 0)
			return false;

		dimension++;
		if (dimension == 3) 
			UpdateSimplex3(a, b, c, d, dimension, searchDirection);

		else if (UpdateSimplex4(a, b, c, d, dimension, searchDirection))
		{
			EPA(a, b, c, d, collisionInfo);
			return true;
		}
	}

	return false;
}

void Physics::UpdateSimplex3(Point& a, Point& b, Point& c, Point& d, int& dimension, Vector3& searchDirection)
{
	// 반시계 방향으로 winding된다.

	Vector3 n = Vector3::Cross(b.difference - a.difference, c.difference - a.difference);
	Vector3 AO = -a.difference;

	dimension = 2;
	// 원점이 AB 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(b.difference - a.difference, n), AO) > 0) {
		c = a;
		searchDirection = Vector3::Cross(Vector3::Cross(b.difference - a.difference, AO), b.difference - a.difference);
		return;
	}
	// 원점이 AC 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(n, c.difference - a.difference), AO) > 0) {
		b = a;
		searchDirection = Vector3::Cross(Vector3::Cross(c.difference - a.difference, AO), c.difference - a.difference);
		return;
	}

	dimension = 3;
	//원점이 삼각형 위쪽에 존재
	if (Vector3::Dot(n, AO) > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDirection = n;
		return;
	}
	
	//원점이 삼각형 아래쪽에 존재
	d = b;
	b = a;
	searchDirection = -n;
	return;
}

bool Physics::UpdateSimplex4(Point& a, Point& b, Point& c, Point& d, int& dimension, Vector3& searchDirection)
{
	Vector3 ABC = Vector3::Cross(b.difference - a.difference, c.difference - a.difference);
	Vector3 ACD = Vector3::Cross(c.difference - a.difference, d.difference - a.difference);
	Vector3 ADB = Vector3::Cross(d.difference - a.difference, b.difference - a.difference);

	Vector3 AO = -a.difference;
	dimension = 3;

	// 원점이 ABC 바깥에 존재
	if (Vector3::Dot(ABC, AO) > 0) {
		d = c;
		c = b;
		b = a;
		searchDirection = ABC;
		return false;
	}

	// 원점이 ACD 바깥에 존재
	if (Vector3::Dot(ACD, AO) > 0) {
		b = a;
		searchDirection = ACD;
		return false;
	}

	// 원점이 ADB 바깥에 존재
	if (Vector3::Dot(ADB, AO) > 0) {
		c = d;
		d = b;
		b = a;
		searchDirection = ADB;
		return false;
	}

	return true;
}

//Expanding Polytope Algorithm
#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
void Physics::EPA(Point& a, Point& b, Point& c, Point& d, CollisionInfo& collisionInfo)
{
	//face들의 배열, 3개의 점과, 1개의 normal
	Point faces[EPA_MAX_NUM_FACES][4];

	Vector3 VertexA[3];
	Vector3 VertexB[3];

	//GJK 알고리즘의 결과로 초기화
	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3].difference = (Vector3::Cross(b.difference - a.difference, c.difference - a.difference)).Normalized(); //ABC
	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3].difference = (Vector3::Cross(c.difference - a.difference, d.difference - a.difference)).Normalized(); //ACD
	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3].difference = (Vector3::Cross(d.difference - a.difference, b.difference - a.difference)).Normalized(); //ADB
	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3].difference = (Vector3::Cross(d.difference - b.difference, c.difference - b.difference)).Normalized(); //BDC

	int num_faces = 4;
	int closest_face;

	for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
		// 원점과 가장 가까운 face를 찾는다.
		float min_dist = Vector3::Dot(faces[0][0].difference, faces[0][3].difference);
		closest_face = 0;
		for (int i = 1; i < num_faces; i++) {
			float dist = Vector3::Dot(faces[i][0].difference, faces[i][3].difference);
			if (dist < min_dist) {
				min_dist = dist;
				closest_face = i;
			}
		}

		//원점에서 가장 가까운 face의 normal을 찾는다
		Vector3 searchDirection = faces[closest_face][3].difference;

		Point p;
		SearchPoint(p, searchDirection, collisionInfo.a, collisionInfo.b);

		if (Vector3::Dot(p.difference, searchDirection) - min_dist < EPA_TOLERANCE) {

			//충돌 정보 계산
			Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].difference, faces[closest_face][1].difference, faces[closest_face][2].difference); //가장 가까운 삼각형 face의 plane
			Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0)); //원점을 삼각형에 투영(민코프스키 공간에서)
			float u, v, w;
			Barycentric(faces[closest_face][0].difference, faces[closest_face][1].difference, faces[closest_face][2].difference,
				projectionPoint, u, v, w); //이 삼각형으로의 투영 점의 barycentric coordinate를 찾는다

			//contact points는 support function의 결과로 얻은 좌표로 구성된 삼각형과 같은 barycentric 좌표를 갖는다
			Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
			Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
			float penetration = (localA - localB).Length();
			Vector3 normal = (localA - localB).Normalized();

			//Convergence
			localA -= collisionInfo.a->GetTransform().GetPosition();
			localB -= collisionInfo.b->GetTransform().GetPosition();

			collisionInfo.AddContactInfo(localA, localB, normal, penetration);

			return;
		}

		
		Point loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //face를 제거하고 fix해야하는 edge들을 보관
		int num_loose_edges = 0;

		//p를 바라보는 모든 삼각형을 찾는다.
		for (int i = 0; i < num_faces; i++)
		{
			if (Vector3::Dot(faces[i][3].difference, p.difference - faces[i][0].difference) > 0) //triangle i faces p, remove it
			{
				// loose edge list에 제거된 삼각형 edge들을 추가한다.
				// 이미 존재하면, 제거한다.
				for (int j = 0; j < 3; j++) //한 face에 대한 3개의 edge
				{
					Point current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found_edge = false;
					for (int k = 0; k < num_loose_edges; k++) //현재 edge가 이미 list에 있는지 확인한다
					{
						if (loose_edges[k][1].difference == current_edge[0].difference && loose_edges[k][0].difference == current_edge[1].difference)
						{
							loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //현재 edge를 list의 마지막 edge로 덮어쓴다
							loose_edges[k][1] = loose_edges[num_loose_edges - 1][1];
							num_loose_edges--;
							found_edge = true;
							k = num_loose_edges; //edge는 한번만 공유될 수 있기 때문에 loop를 나간다
						}
					}

					if (!found_edge)
					{
						//현재 edge를 list에 추가한다.
						if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
						loose_edges[num_loose_edges][0] = current_edge[0];
						loose_edges[num_loose_edges][1] = current_edge[1];
						num_loose_edges++;
					}
				}

				//삼각형 i를 list에서 제거
				faces[i][0] = faces[num_faces - 1][0];
				faces[i][1] = faces[num_faces - 1][1];
				faces[i][2] = faces[num_faces - 1][2];
				faces[i][3] = faces[num_faces - 1][3];
				num_faces--;
				i--;
			}//p가 삼각형 i를 볼 수 있으면 종료
		}

		//polytope를 p를 추가해서 다시 구성한다
		for (int i = 0; i < num_loose_edges; i++)
		{
			if (num_faces >= EPA_MAX_NUM_FACES) break;
			faces[num_faces][0] = loose_edges[i][0];
			faces[num_faces][1] = loose_edges[i][1];
			faces[num_faces][2] = p;
			faces[num_faces][3].difference = Vector3::Cross(loose_edges[i][0].difference - loose_edges[i][1].difference, loose_edges[i][0].difference - p.difference).Normalized();

			//ccw winding을 유지하기 위해서 잘못된 normal을 확인한다.
			float bias = 0.000001; // dot 결과가 아주 조금 < 0 인 경우 (원점이 face에 존재해서)
			if (Vector3::Dot(faces[num_faces][0].difference, faces[num_faces][3].difference) + bias < 0) {
				Point temp = faces[num_faces][0];
				faces[num_faces][0] = faces[num_faces][1];
				faces[num_faces][1] = temp;
				faces[num_faces][3].difference = -faces[num_faces][3].difference;
			}
			num_faces++;
		}
	}
	printf("EPA did not converge\n");
	//가장 최근의 가까운 점을 반환한다
	Vector3 searchDirection = faces[closest_face][3].difference;

	Point p;
	SearchPoint(p, searchDirection, collisionInfo.a, collisionInfo.b);

	Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].difference, faces[closest_face][1].difference, faces[closest_face][2].difference);
	Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0));
	float u, v, w;
	Barycentric(faces[closest_face][0].difference, faces[closest_face][1].difference, faces[closest_face][2].difference,
		projectionPoint, u, v, w);
	Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
	Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
	float penetration = (localA - localB).Length();
	Vector3 normal = (localA - localB).Normalized();

	collisionInfo.AddContactInfo(localA, localB, normal, penetration);

	return;
}

void Physics::SearchPoint(Point& point, Vector3& searchDirectionection, PhysicsObject* objectA, PhysicsObject* objectB)
{
	point.b = objectB->GetCollider()->Support(searchDirectionection, objectB->GetTransform());
	point.a = objectA->GetCollider()->Support(-searchDirectionection, objectA->GetTransform());
	point.difference = point.b - point.a;
}
