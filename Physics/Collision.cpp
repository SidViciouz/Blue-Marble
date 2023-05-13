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

	//simplex ���ο� ������ �������� �ʴ� ���
	if (Vector3::Dot(b.p, search_dir) < 0)
		return false;

	//���� bc�� �����̰� ������ ���ϴ� ����
	search_dir = Vector3::Cross(Vector3::Cross(c.p - b.p, -b.p), c.p - b.p);
	if (search_dir == Vector3(0, 0, 0))  //���� bc ���� ������ �����ϴ� ���
	{ 
		search_dir = Vector3::Cross(c.p - b.p, Vector3(1, 0, 0));
		if (search_dir == Vector3(0, 0, 0))
			search_dir = Vector3::Cross(c.p - b.p, Vector3(0, 0, -1));
	}
	int simp_dim = 2;

	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
	{
		CalculateSearchPoint(a, search_dir, coll1, coll2);

		//simplex ���ο� ������ �������� �ʴ� ���
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
	// ������ AB �ٱ��� ����
	if (Vector3::Dot(Vector3::Cross(b.p - a.p, n), AO) > 0) {
		c = a;
		search_dir = Vector3::Cross(Vector3::Cross(b.p - a.p, AO), b.p - a.p);
		return;
	}
	// ������ AC �ٱ��� ����
	if (Vector3::Dot(Vector3::Cross(n, c.p - a.p), AO) > 0) {
		b = a;
		search_dir = Vector3::Cross(Vector3::Cross(c.p - a.p, AO), c.p - a.p);
		return;
	}

	simp_dim = 3;
	//������ �ﰢ�� ���ʿ� ����
	if (Vector3::Dot(n, AO) > 0)
	{
		d = c;
		c = b;
		b = a;
		search_dir = n;
		return;
	}
	
	//������ �ﰢ�� �Ʒ��ʿ� ����
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

	// ������ ABC �ٱ��� ����
	if (Vector3::Dot(ABC, AO) > 0) {
		d = c;
		c = b;
		b = a;
		search_dir = ABC;
		return false;
	}

	// ������ ACD �ٱ��� ����
	if (Vector3::Dot(ACD, AO) > 0) {
		b = a;
		search_dir = ACD;
		return false;
	}

	// ������ ADB �ٱ��� ����
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
	//face���� �迭, 3���� ����, 1���� normal
	Point faces[EPA_MAX_NUM_FACES][4];

	Vector3 VertexA[3];
	Vector3 VertexB[3];

	//GJK �˰����� ����� �ʱ�ȭ
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
		// ������ ���� ����� face�� ã�´�.
		float min_dist = Vector3::Dot(faces[0][0].p, faces[0][3].p);
		closest_face = 0;
		for (int i = 1; i < num_faces; i++) {
			float dist = Vector3::Dot(faces[i][0].p, faces[i][3].p);
			if (dist < min_dist) {
				min_dist = dist;
				closest_face = i;
			}
		}

		//�������� ���� ����� face�� normal�� ã�´�
		Vector3 search_dir = faces[closest_face][3].p;

		Point p;
		CalculateSearchPoint(p, search_dir, coll1, coll2);

		if (Vector3::Dot(p.p, search_dir) - min_dist < EPA_TOLERANCE) {

			//�浹 ���� ���
			Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p); //���� ����� �ﰢ�� face�� plane
			Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0)); //������ �ﰢ���� ����(��������Ű ��������)
			float u, v, w;
			Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
				projectionPoint, u, v, w); //�� �ﰢ�������� ���� ���� barycentric coordinate�� ã�´�

			//contact points�� support function�� ����� ���� ��ǥ�� ������ �ﰢ���� ���� barycentric ��ǥ�� ���´�
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

		
		Point loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //face�� �����ϰ� fix�ؾ��ϴ� edge���� ����
		int num_loose_edges = 0;

		//p�� �ٶ󺸴� ��� �ﰢ���� ã�´�.
		for (int i = 0; i < num_faces; i++)
		{
			if (Vector3::Dot(faces[i][3].p, p.p - faces[i][0].p) > 0) //triangle i faces p, remove it
			{
				// loose edge list�� ���ŵ� �ﰢ�� edge���� �߰��Ѵ�.
				// �̹� �����ϸ�, �����Ѵ�.
				for (int j = 0; j < 3; j++) //�� face�� ���� 3���� edge
				{
					Point current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found_edge = false;
					for (int k = 0; k < num_loose_edges; k++) //���� edge�� �̹� list�� �ִ��� Ȯ���Ѵ�
					{
						if (loose_edges[k][1].p == current_edge[0].p && loose_edges[k][0].p == current_edge[1].p)
						{
							loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //���� edge�� list�� ������ edge�� �����
							loose_edges[k][1] = loose_edges[num_loose_edges - 1][1];
							num_loose_edges--;
							found_edge = true;
							k = num_loose_edges; //edge�� �ѹ��� ������ �� �ֱ� ������ loop�� ������
						}
					}

					if (!found_edge)
					{
						//���� edge�� list�� �߰��Ѵ�.
						if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
						loose_edges[num_loose_edges][0] = current_edge[0];
						loose_edges[num_loose_edges][1] = current_edge[1];
						num_loose_edges++;
					}
				}

				//�ﰢ�� i�� list���� ����
				faces[i][0] = faces[num_faces - 1][0];
				faces[i][1] = faces[num_faces - 1][1];
				faces[i][2] = faces[num_faces - 1][2];
				faces[i][3] = faces[num_faces - 1][3];
				num_faces--;
				i--;
			}//p�� �ﰢ�� i�� �� �� ������ ����
		}

		//polytope�� p�� �߰��ؼ� �ٽ� �����Ѵ�
		for (int i = 0; i < num_loose_edges; i++)
		{
			if (num_faces >= EPA_MAX_NUM_FACES) break;
			faces[num_faces][0] = loose_edges[i][0];
			faces[num_faces][1] = loose_edges[i][1];
			faces[num_faces][2] = p;
			faces[num_faces][3].p = Vector3::Cross(loose_edges[i][0].p - loose_edges[i][1].p, loose_edges[i][0].p - p.p).Normalized();

			//ccw winding�� �����ϱ� ���ؼ� �߸��� normal�� Ȯ���Ѵ�.
			float bias = 0.000001; // dot ����� ���� ���� < 0 �� ��� (������ face�� �����ؼ�)
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
	//���� �ֱ��� ����� ���� ��ȯ�Ѵ�
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
