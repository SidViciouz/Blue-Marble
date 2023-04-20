#include "SphereCollisionComponent.h"
#include "BoxCollisionComponent.h"
#include "SceneNode.h"
#include "Plane.h"

#define GJK_MAX_NUM_ITERATIONS 64
#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64

SphereCollisionComponent::SphereCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius)
	: CollisionComponent(NodeAttachedTo), mRadius{radius}
{

}

bool SphereCollisionComponent::IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo)
{
	BoxCollisionComponent* boxOther = dynamic_cast<BoxCollisionComponent*>(other);

	if (boxOther == nullptr)
		return false;

	int otherW = boxOther->mWidth * 0.5f;
	int otherH = boxOther->mHeight * 0.5f;
	int otherD = boxOther->mDepth * 0.5f;

	vector<Vector3> otherPoints;
	otherPoints.push_back(Vector3(-otherW, -otherH, -otherD));
	otherPoints.push_back(Vector3(-otherW, -otherH, otherD));
	otherPoints.push_back(Vector3(-otherW, otherH, -otherD));
	otherPoints.push_back(Vector3(-otherW, otherH, otherD));
	otherPoints.push_back(Vector3(otherW, -otherH, -otherD));
	otherPoints.push_back(Vector3(otherW, -otherH, otherD));
	otherPoints.push_back(Vector3(otherW, otherH, -otherD));
	otherPoints.push_back(Vector3(otherW, otherH, otherD));

	//rotate vector
	Quaternion otherQ = other->mNodeAttachedTo->GetAccumulatedQuaternion();
	XMMATRIX otherQm = XMMatrixRotationQuaternion(XMLoadFloat4(&otherQ.Get()));

	Position otherP = other->mNodeAttachedTo->GetAccumulatedPosition();
	XMMATRIX otherPm = XMMatrixTranslationFromVector(XMLoadFloat3(&otherP.Get()));

	XMMATRIX otherQmpm = XMMatrixMultiply(otherQm, otherPm);

	for (auto& point : otherPoints)
	{
		XMStoreFloat3(&point.v, XMVector3Transform(XMLoadFloat3(&point.v), otherQmpm));
	}

	Vector3 self(mNodeAttachedTo->GetAccumulatedPosition().Get());

	mSelfPosition = Vector3(self.v.x, self.v.y, self.v.z);
	mOtherPosition = Vector3(otherP.Get().x, otherP.Get().y, otherP.Get().z);

	return GJK(otherPoints, collisionInfo);
	//return true;
}

void SphereCollisionComponent::Draw()
{

}

bool SphereCollisionComponent::GJK(const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo)
{
	Vector3 self(mNodeAttachedTo->GetAccumulatedPosition().Get());
	Vector3 dir(1.0f*mRadius, 0.0f, 0.0f);

	Vector3* mtv;
	Points a, b, c, d;
	Vector3 searchDir = (self + dir) - otherPoints[0];

	CalculateSearchPoint(c, searchDir, otherPoints);
	searchDir = -c.p;

	CalculateSearchPoint(b, searchDir, otherPoints);

	if (b.p * searchDir < 0)
		return false;

	searchDir = ((c.p - b.p) ^ (-b.p)) ^ (c.p - b.p);
	if (searchDir.Zero())
	{
		searchDir = (c.p - b.p) ^ Vector3(1, 0, 0);
		if (searchDir.Zero())
		{
			searchDir = (c.p - b.p) ^ Vector3(0, 0, -1);
		}
	}

	int simpDim = 2;

	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
	{
		CalculateSearchPoint(a, searchDir, otherPoints);

		if (a.p * searchDir < 0)
			return false;

		simpDim++;

		if (simpDim == 3)
			UpdateSimplex3(a, b, c, d, simpDim, searchDir);
		else if (UpdateSimplex4(a, b, c, d, simpDim, searchDir))
		{
			EPA(a, b, c, d, otherPoints, collisionInfo);
			return true;
		}
	}
	
	return false;
}

void SphereCollisionComponent::UpdateSimplex3(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir)
{
	Vector3 n = (b.p - a.p) ^ (c.p - a.p);
	Vector3 AO = -a.p;

	simpDim = 2;
	if (((b.p - a.p) ^ n) * AO > 0)
	{
		c = a;
		searchDir = ((b.p - a.p) ^ AO) ^ (b.p - a.p);
		return;
	}

	if ((n ^ (c.p - a.p)) * AO > 0)
	{
		b = a;
		searchDir = ((c.p - a.p) ^ AO) ^ (c.p - a.p);
		return;
	}

	simpDim = 3;
	if (n * AO > 0)
	{
		d = c;
		c = b;
		b = a;
		searchDir = n;
		return;
	}
	d = b;
	b = a;
	searchDir = -n;
	return;
}

bool SphereCollisionComponent::UpdateSimplex4(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir)
{
	Vector3 ABC = (b.p - a.p) ^ (c.p - a.p);
	Vector3 ACD = (c.p - a.p) ^ (d.p - a.p);
	Vector3 ADB = (d.p - a.p) ^ (b.p - a.p);

	Vector3 AO = -a.p;
	simpDim = 3;


	if (ABC * AO > 0) {
		d = c;
		c = b;
		b = a;
		searchDir = ABC;
		return false;
	}

	if (ACD * AO > 0) {
		b = a;
		searchDir = ACD;
		return false;
	}
	if (ADB * AO > 0) {
		c = d;
		d = b;
		b = a;
		searchDir = ADB;
		return false;
	}

	return true;
}

void SphereCollisionComponent::EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo)
{
	Points faces[EPA_MAX_NUM_FACES][4];

	Vector3 VertexA[3];
	Vector3 VertexB[3];

	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3].p = ((b.p - a.p) ^ (c.p - a.p)).normalize(); //ABC
	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3].p = ((c.p - a.p) ^ (d.p - a.p)).normalize(); //ACD
	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3].p = ((d.p - a.p) ^ (b.p - a.p)).normalize(); //ADB
	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3].p = ((d.p - b.p) ^ (c.p - b.p)).normalize(); //BDC

	int num_faces = 4;
	int closest_face;

	for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
		//Find face that's closest to origin
		float min_dist = faces[0][0].p * faces[0][3].p;
		closest_face = 0;
		for (int i = 1; i < num_faces; i++) {
			float dist = faces[i][0].p * faces[i][3].p;
			if (dist < min_dist) {
				min_dist = dist;
				closest_face = i;
			}
		}

		//search normal to face that's closest to origin
		Vector3 search_dir = faces[closest_face][3].p;

		Points p;
		CalculateSearchPoint(p, search_dir, otherPoints);

		if (p.p * search_dir - min_dist < EPA_TOLERANCE) {

			//collisionInfo.normal = faces[closest_face][3].p * (p.p * search_dir);

			Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p); //plane of closest triangle face
			Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0)); //projecting the origin onto the triangle(both are in Minkowski space)
			float u, v, w;
			Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
				projectionPoint, u, v, w); //finding the barycentric coordinate of this projection point to the triangle

			Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
			Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
			float penetration = (localA - localB).length();
			Vector3 normal = (localA - localB).normalize();

			localA = localA - mSelfPosition;
			localB = localB - mOtherPosition;

			collisionInfo.localA = localA;
			collisionInfo.localB = localB;
			collisionInfo.penetration = penetration;
			collisionInfo.normal = normal;
			//collisionInfo.AddContactPoint(localA, localB, normal, penetration);

			return;
		}

		Points loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //keep track of edges we need to fix after removing faces
		int num_loose_edges = 0;

		//Find all triangles that are facing p
		for (int i = 0; i < num_faces; i++)
		{
			if (faces[i][3].p * (p.p - faces[i][0].p) > 0) //triangle i faces p, remove it
			{
				//Add removed triangle's edges to loose edge list.
				//If it's already there, remove it (both triangles it belonged to are gone)
				for (int j = 0; j < 3; j++) //Three edges per face
				{
					Points current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found_edge = false;
					for (int k = 0; k < num_loose_edges; k++) //Check if current edge is already in list
					{
						if (loose_edges[k][1].p == current_edge[0].p && loose_edges[k][0].p == current_edge[1].p) {
							loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //Overwrite current edge
							loose_edges[k][1] = loose_edges[num_loose_edges - 1][1]; //with last edge in list
							num_loose_edges--;
							found_edge = true;
							k = num_loose_edges; //exit loop because edge can only be shared once
						}
					}//endfor loose_edges

					if (!found_edge) { //add current edge to list
						// assert(num_loose_edges<EPA_MAX_NUM_LOOSE_EDGES);
						if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
						loose_edges[num_loose_edges][0] = current_edge[0];
						loose_edges[num_loose_edges][1] = current_edge[1];
						num_loose_edges++;
					}
				}

				//Remove triangle i from list
				faces[i][0] = faces[num_faces - 1][0];
				faces[i][1] = faces[num_faces - 1][1];
				faces[i][2] = faces[num_faces - 1][2];
				faces[i][3] = faces[num_faces - 1][3];
				num_faces--;
				i--;
			}//endif p can see triangle i
		}//endfor num_faces

		//Reconstruct polytope with p added
		for (int i = 0; i < num_loose_edges; i++)
		{
			// assert(num_faces<EPA_MAX_NUM_FACES);
			if (num_faces >= EPA_MAX_NUM_FACES) break;
			faces[num_faces][0] = loose_edges[i][0];
			faces[num_faces][1] = loose_edges[i][1];
			faces[num_faces][2] = p;
			faces[num_faces][3].p = ((loose_edges[i][0].p - loose_edges[i][1].p) ^ (loose_edges[i][0].p - p.p)).normalize();

			//Check for wrong normal to maintain CCW winding
			float bias = 0.000001; //in case dot result is only slightly < 0 (because origin is on face)
			if ((faces[num_faces][0].p * faces[num_faces][3].p) + bias < 0) {
				Points temp = faces[num_faces][0];
				faces[num_faces][0] = faces[num_faces][1];
				faces[num_faces][1] = temp;
				faces[num_faces][3].p = -faces[num_faces][3].p;
			}
			num_faces++;
		}
	} //End for iterations
	printf("EPA did not converge\n");
	//Return most recent closest point
	//collisionInfo.normal = faces[closest_face][3].p * (faces[closest_face][0].p*faces[closest_face][3].p);

	Vector3 search_dir = faces[closest_face][3].p;

	Points p;
	CalculateSearchPoint(p, search_dir, otherPoints);

	Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p);
	Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0));
	float u, v, w;
	Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
		projectionPoint, u, v, w);
	Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
	Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
	float penetration = (localA - localB).length();
	Vector3 normal = (localA - localB).normalize();

	collisionInfo.localA = localA;
	collisionInfo.localB = localB;
	collisionInfo.penetration = penetration;
	collisionInfo.normal = normal;
	//collisionInfo.AddContactPoint(localA, localB, normal, penetration);

	return;
}

void SphereCollisionComponent::CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& otherPoints)
{
	Vector3 self(mNodeAttachedTo->GetAccumulatedPosition().Get());

	point.b = Support(searchDir, otherPoints);
	point.a = -searchDir.normalize() * mRadius + self;
	point.p = point.b - point.a;
}

Vector3 SphereCollisionComponent::Support(const Vector3& dir, const vector<Vector3>& Points)
{
	float maxDot = Points[0] * dir;
	int index = 0;

	for (int i = 0; i < Points.size(); ++i)
	{
		float dot = Points[i] * dir;
		if (dot > maxDot)
		{
			maxDot = dot;
			index = i;
		}
	}

	return Points[index];
}

void SphereCollisionComponent::Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w)
{
	Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = v0 * v0;
	float d01 = v0 * v1;
	float d11 = v1 * v1;
	float d20 = v2 * v0;
	float d21 = v2 * v1;
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;

}