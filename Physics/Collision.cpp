#include "Collision.h"
#include "../Maths/Maths.h"
#include "../Maths/Plane.h"

#define GJK_MAX_NUM_ITERATIONS 64

bool Physics::GJKCalculation(PhysicsObject* coll1, PhysicsObject* coll2, CollisionInfo& collisionInfo)
{
	collisionInfo.a = coll1;
	collisionInfo.b = coll2;

	Vector3* mtv;
	
	Vector3 coll1Pos = coll1->GetTransform().GetPosition();
	Vector3 coll2Pos = coll2->GetTransform().GetPosition();


	Point a, b, c, d;
	Vector3 search_dir = coll1Pos - coll2Pos;

	CalculateSearchPoint(c, search_dir, coll1, coll2);
	search_dir = -c.p;

	CalculateSearchPoint(b, search_dir, coll1, coll2);

	//simplex 내부에 원점이 존재하지 않는 경우
	if (Vector3::Dot(b.p, search_dir) < 0)
		return false;

	//직선 bc에 수직이고 원점을 향하는 벡터
	search_dir = Vector3::Cross(Vector3::Cross(c.p - b.p, -b.p), c.p - b.p);
	if (search_dir == Vector3(0, 0, 0))  //직선 bc 위에 원점이 존재하는 경우
	{ 
		search_dir = Vector3::Cross(c.p - b.p, Vector3(1, 0, 0));
		if (search_dir == Vector3(0, 0, 0))
			search_dir = Vector3::Cross(c.p - b.p, Vector3(0, 0, -1));
	}
	int simp_dim = 2;

	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
	{
		CalculateSearchPoint(a, search_dir, coll1, coll2);

		//simplex 내부에 원점이 존재하지 않는 경우
		if (Vector3::Dot(a.p, search_dir) < 0)
			return false;

		simp_dim++;
		if (simp_dim == 3) 
			update_simplex3(a, b, c, d, simp_dim, search_dir);

		else if (update_simplex4(a, b, c, d, simp_dim, search_dir))
		{
			EPA(a, b, c, d, coll1, coll2, collisionInfo);
			return true;
		}
	}

	return false;
}

void Physics::update_simplex3(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir)
{
	/* Required winding order:
	   //  b
	   //  | \
	   //  |   \
	   //  |    a
	   //  |   /
	   //  | /
	   //  c
	   */
	Vector3 n = Vector3::Cross(b.p - a.p, c.p - a.p);
	Vector3 AO = -a.p;

	simp_dim = 2;
	// 원점이 AB 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(b.p - a.p, n), AO) > 0) {
		c = a;
		search_dir = Vector3::Cross(Vector3::Cross(b.p - a.p, AO), b.p - a.p);
		return;
	}
	// 원점이 AC 바깥에 존재
	if (Vector3::Dot(Vector3::Cross(n, c.p - a.p), AO) > 0) {
		b = a;
		search_dir = Vector3::Cross(Vector3::Cross(c.p - a.p, AO), c.p - a.p);
		return;
	}

	simp_dim = 3;
	//원점이 삼각형 위쪽에 존재
	if (Vector3::Dot(n, AO) > 0)
	{
		d = c;
		c = b;
		b = a;
		search_dir = n;
		return;
	}
	
	//원점이 삼각형 아래쪽에 존재
	d = b;
	b = a;
	search_dir = -n;
	return;
}

bool Physics::update_simplex4(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir)
{
	Vector3 ABC = Vector3::Cross(b.p - a.p, c.p - a.p);
	Vector3 ACD = Vector3::Cross(c.p - a.p, d.p - a.p);
	Vector3 ADB = Vector3::Cross(d.p - a.p, b.p - a.p);

	Vector3 AO = -a.p;
	simp_dim = 3;

	// 원점이 ABC 바깥에 존재
	if (Vector3::Dot(ABC, AO) > 0) {
		d = c;
		c = b;
		b = a;
		search_dir = ABC;
		return false;
	}

	// 원점이 ACD 바깥에 존재
	if (Vector3::Dot(ACD, AO) > 0) {
		b = a;
		search_dir = ACD;
		return false;
	}

	// 원점이 ADB 바깥에 존재
	if (Vector3::Dot(ADB, AO) > 0) {
		c = d;
		d = b;
		b = a;
		search_dir = ADB;
		return false;
	}

	return true;
}

//Expanding Polytope Algorithm
#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
void Physics::EPA(Point& a, Point& b, Point& c, Point& d, PhysicsObject* coll1, PhysicsObject* coll2, CollisionInfo& collisionInfo)
{
	//face들의 배열, 3개의 점과, 1개의 normal
	Point faces[EPA_MAX_NUM_FACES][4];

	Vector3 VertexA[3];
	Vector3 VertexB[3];

	//GJK 알고리즘의 결과로 초기화
	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3].p = (Vector3::Cross(b.p - a.p, c.p - a.p)).Normalized(); //ABC
	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3].p = (Vector3::Cross(c.p - a.p, d.p - a.p)).Normalized(); //ACD
	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3].p = (Vector3::Cross(d.p - a.p, b.p - a.p)).Normalized(); //ADB
	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3].p = (Vector3::Cross(d.p - b.p, c.p - b.p)).Normalized(); //BDC

	int num_faces = 4;
	int closest_face;

	for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
		// 원점과 가장 가까운 face를 찾는다.
		float min_dist = Vector3::Dot(faces[0][0].p, faces[0][3].p);
		closest_face = 0;
		for (int i = 1; i < num_faces; i++) {
			float dist = Vector3::Dot(faces[i][0].p, faces[i][3].p);
			if (dist < min_dist) {
				min_dist = dist;
				closest_face = i;
			}
		}

		//원점에서 가장 가까운 face의 normal을 찾는다
		Vector3 search_dir = faces[closest_face][3].p;

		Point p;
		CalculateSearchPoint(p, search_dir, coll1, coll2);

		if (Vector3::Dot(p.p, search_dir) - min_dist < EPA_TOLERANCE) {

			//충돌 정보 계산
			Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p); //가장 가까운 삼각형 face의 plane
			Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0)); //원점을 삼각형에 투영(민코프스키 공간에서)
			float u, v, w;
			Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
				projectionPoint, u, v, w); //이 삼각형으로의 투영 점의 barycentric coordinate를 찾는다

			//contact points는 support function의 결과로 얻은 좌표로 구성된 삼각형과 같은 barycentric 좌표를 갖는다
			Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
			Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
			float penetration = (localA - localB).Length();
			Vector3 normal = (localA - localB).Normalized();

			//Convergence
			localA -= coll1->GetTransform().GetPosition();
			localB -= coll2->GetTransform().GetPosition();

			collisionInfo.AddContactPoint(localA, localB, normal, penetration);

			return;
		}

		
		Point loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //face를 제거하고 fix해야하는 edge들을 보관
		int num_loose_edges = 0;

		//p를 바라보는 모든 삼각형을 찾는다.
		for (int i = 0; i < num_faces; i++)
		{
			if (Vector3::Dot(faces[i][3].p, p.p - faces[i][0].p) > 0) //triangle i faces p, remove it
			{
				// loose edge list에 제거된 삼각형 edge들을 추가한다.
				// 이미 존재하면, 제거한다.
				for (int j = 0; j < 3; j++) //한 face에 대한 3개의 edge
				{
					Point current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found_edge = false;
					for (int k = 0; k < num_loose_edges; k++) //현재 edge가 이미 list에 있는지 확인한다
					{
						if (loose_edges[k][1].p == current_edge[0].p && loose_edges[k][0].p == current_edge[1].p)
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
			faces[num_faces][3].p = Vector3::Cross(loose_edges[i][0].p - loose_edges[i][1].p, loose_edges[i][0].p - p.p).Normalized();

			//ccw winding을 유지하기 위해서 잘못된 normal을 확인한다.
			float bias = 0.000001; // dot 결과가 아주 조금 < 0 인 경우 (원점이 face에 존재해서)
			if (Vector3::Dot(faces[num_faces][0].p, faces[num_faces][3].p) + bias < 0) {
				Point temp = faces[num_faces][0];
				faces[num_faces][0] = faces[num_faces][1];
				faces[num_faces][1] = temp;
				faces[num_faces][3].p = -faces[num_faces][3].p;
			}
			num_faces++;
		}
	}
	printf("EPA did not converge\n");
	//가장 최근의 가까운 점을 반환한다
	Vector3 search_dir = faces[closest_face][3].p;

	Point p;
	CalculateSearchPoint(p, search_dir, coll1, coll2);

	Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p);
	Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0));
	float u, v, w;
	Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
		projectionPoint, u, v, w);
	Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
	Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
	float penetration = (localA - localB).Length();
	Vector3 normal = (localA - localB).Normalized();

	collisionInfo.AddContactPoint(localA, localB, normal, penetration);

	return;
}

void Physics::CalculateSearchPoint(Point& point, Vector3& search_dir, PhysicsObject* coll1, PhysicsObject* coll2)
{
	point.b = coll2->GetCollider()->Support(search_dir, coll2->GetTransform());
	point.a = coll1->GetCollider()->Support(-search_dir, coll1->GetTransform());
	point.p = point.b - point.a;
}
