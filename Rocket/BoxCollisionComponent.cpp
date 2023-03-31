#include "BoxCollisionComponent.h"
#include "Constant.h"
#include "SceneNode.h"
#include "Engine.h"
#include <unordered_set>
#include <set>

BoxCollisionComponent::BoxCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, int width, int height, int depth)
	: CollisionComponent(NodeAttachedTo), mWidth{ width }, mHeight{ height }, mDepth{ depth }
{
	assert(NodeAttachedTo != nullptr);
	assert(mWidth != 0.0f);
	assert(mHeight != 0.0f);
	assert(mDepth != 0.0f);

	mVertices.push_back(Vertex{ {-0.5f,-0.5f,-0.5f},{},{} });
	mVertices.push_back(Vertex{ {-0.5f,-0.5f,0.5f},{},{} });
	mVertices.push_back(Vertex{ {-0.5f,0.5f,-0.5f},{},{} });
	mVertices.push_back(Vertex{ {-0.5f,0.5f,0.5f},{},{} });
	mVertices.push_back(Vertex{ {0.5f,-0.5f,-0.5f},{},{} });
	mVertices.push_back(Vertex{ {0.5f,-0.5f,0.5f},{},{} });
	mVertices.push_back(Vertex{ {0.5f,0.5f,-0.5f},{},{} });
	mVertices.push_back(Vertex{ {0.5f,0.5f,0.5f},{},{} });

	mVertices.push_back(Vertex{ {0.0f,0.0f,0.0f},{},{} });
	mVertices.push_back(Vertex{ {0.0f,0.0f,0.0f},{},{} });

	mIndices.push_back(0);
	mIndices.push_back(1);
	mIndices.push_back(1);
	mIndices.push_back(3);
	mIndices.push_back(3);
	mIndices.push_back(2);
	mIndices.push_back(2);
	mIndices.push_back(0);

	mIndices.push_back(2);
	mIndices.push_back(6);
	mIndices.push_back(3);
	mIndices.push_back(7);
	mIndices.push_back(0);
	mIndices.push_back(4);
	mIndices.push_back(1);
	mIndices.push_back(5);

	mIndices.push_back(6);
	mIndices.push_back(7);
	mIndices.push_back(7);
	mIndices.push_back(5);
	mIndices.push_back(5);
	mIndices.push_back(4);
	mIndices.push_back(4);
	mIndices.push_back(6);

	mIndices.push_back(8);
	mIndices.push_back(9);

	mVertexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(Vertex) * mVertices.size());
	mIndexBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(sizeof(uint16_t) * mIndices.size());

	mVertexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertices.size());
	mIndexUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndices.size());

	Engine::mResourceManager->Upload(mVertexUploadBufferIdx, mVertices.data(), sizeof(Vertex)* mVertices.size(), 0);
	Engine::mResourceManager->Upload(mIndexUploadBufferIdx, mIndices.data(), sizeof(uint16_t)* mIndices.size(), 0);

	D3D12_RESOURCE_BARRIER barrier[2];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mVertexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	barrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(Engine::mResourceManager->GetResource(mIndexBufferIdx),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	Engine::mCommandList->ResourceBarrier(2, barrier);

	Engine::mResourceManager->CopyUploadToBuffer(mVertexUploadBufferIdx, mVertexBufferIdx);
	Engine::mResourceManager->CopyUploadToBuffer(mIndexUploadBufferIdx, mIndexBufferIdx);

	barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	Engine::mCommandList->ResourceBarrier(2, barrier);
}

bool BoxCollisionComponent::IsColliding(CollisionComponent* other)
{
	bool retval = false;


	//두 collider에 대한 shape를 얻는다.
	int selfW = mWidth * 0.5f;
	int selfH = mHeight * 0.5f;
	int selfD = mDepth * 0.5f;

	vector<Vector3> selfPoints;
	selfPoints.push_back(Vector3(-selfW, -selfH, -selfD));
	selfPoints.push_back(Vector3(-selfW, -selfH, selfD));
	selfPoints.push_back(Vector3(-selfW, selfH, -selfD));
	selfPoints.push_back(Vector3(-selfW, selfH, selfD));
	selfPoints.push_back(Vector3(selfW, -selfH, -selfD));
	selfPoints.push_back(Vector3(selfW, -selfH, selfD));
	selfPoints.push_back(Vector3(selfW, selfH, -selfD));
	selfPoints.push_back(Vector3(selfW, selfH , selfD));

	//rotate vector
	Quaternion selfQ = mNodeAttachedTo->mAccumulatedQuaternion;
	XMMATRIX selfQm = XMMatrixRotationQuaternion(XMLoadFloat4(&selfQ.Get()));

	Position selfP = mNodeAttachedTo->mAccumulatedPosition;
	XMMATRIX selfPm = XMMatrixTranslationFromVector(XMLoadFloat3(&selfP.Get()));

	XMMATRIX selfQmpm = XMMatrixMultiply(selfQm, selfPm);

	for (auto& point : selfPoints)
	{
		XMStoreFloat3(&point.v,XMVector3Transform(XMLoadFloat3(&point.v), selfQmpm));
	}


	BoxCollisionComponent* boxOther = dynamic_cast<BoxCollisionComponent*>(other);

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
	Quaternion otherQ = other->mNodeAttachedTo->mAccumulatedQuaternion;
	XMMATRIX otherQm = XMMatrixRotationQuaternion(XMLoadFloat4(&otherQ.Get()));

	Position otherP = other->mNodeAttachedTo->mAccumulatedPosition;
	XMMATRIX otherPm = XMMatrixTranslationFromVector(XMLoadFloat3(&otherP.Get()));

	XMMATRIX otherQmpm = XMMatrixMultiply(otherQm, otherPm);

	for (auto& point : otherPoints)
	{
		XMStoreFloat3(&point.v, XMVector3Transform(XMLoadFloat3(&point.v), otherQmpm));
	}


	// gjk algorithm

	bool loopContinue = true;
	vector<Vector3> S;
	Vector3 O;
	Vector3 C = selfPoints[0] - otherPoints[0];
	S.push_back(C);
	Vector3 D = O - C;
	while (loopContinue)
	{
		Vector3 N = Support(D, selfPoints, otherPoints);
		if (N * D < 0.0f)
		{
			loopContinue = false;
		}
		else
		{
			S.push_back(N);
			if (DoSimplex(S, D))
			{
				loopContinue = false;
				retval = true;
			}
		}
	}


	if (retval)
	{
		mIsColliding = 1;
		CollisionInfo collision = EPA(S,selfPoints,otherPoints);
	}
	else
		mIsColliding = 0;

	return retval;
}

void BoxCollisionComponent::Draw()
{
	int data[4] = { mWidth,mHeight,mDepth,mIsColliding };
	Engine::mCommandList->SetGraphicsRoot32BitConstants(2, 4, data, 0);
	Engine::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());
	Engine::mCommandList->IASetIndexBuffer(GetIndexBufferView());
	Engine::mCommandList->DrawIndexedInstanced(mIndices.size(), 1, 0, 0, 0);
}

Vector3	BoxCollisionComponent::Support(const Vector3& D, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints) const
{
	const Vector3 minusD = Vector3(-D.v.x, -D.v.y, -D.v.z);

	Vector3 maxSelf = selfPoints[0];
	float maxSelfDot = selfPoints[0] * D;
	Vector3 maxOther = otherPoints[0];
	float maxOtherDot = otherPoints[0] * minusD;
	
	for (auto& selfPoint : selfPoints)
	{
		float dot = selfPoint * D;
		if (dot > maxSelfDot)
		{
			maxSelf = selfPoint;
			maxSelfDot = dot;
		}
	}

	for (auto& otherPoint : otherPoints)
	{
		float dot = otherPoint * minusD;
		if (dot > maxOtherDot)
		{
			maxOther = otherPoint;
			maxOtherDot = dot;
		}
	}

	return maxSelf - maxOther;
}

bool BoxCollisionComponent::DoSimplex(vector<Vector3>& S, Vector3& D) const
{
	/*
	* S는 적어도 2개 이상의 point를 갖고있다.
	* gjk algorithm에서 적어도 2개 이상일 때부터 호출하기 때문이다.
	*/
	assert(S.size() > 1 && S.size() < 5);

	bool isIntersected = false;
	Vector3 O;
	
	if (S.size() == 2)
	{
		/*
		* S = [B,A]
		*/
		Vector3 AB = S[0] - S[1];
		Vector3 AO = O - S[1];

		if (AB * AO > 0.0f)
		{
			/*
			* S = [B,A]
			*/
			D = (AB ^ AO) ^ AB;
		}
		else
		{
			/*
			* S = [A]
			*/
			S.erase(S.cbegin());
			D = AO;
		}
	}

	else if (S.size() == 3)
	{
		/*
		* S = [C,B,A]
		*/
		Vector3 AB = S[1] - S[2];
		Vector3 AC = S[0] - S[2];
		Vector3 AO = O - S[2];

		if (((AB ^ AC) ^ AC) * AO > 0.0f)
		{
			if (AC * AO)
			{
				/*
				* S = [C,A]
				*/
				S.erase(S.cbegin() + 1);
				D = (AC ^ AO) ^ AC;
			}
			else
			{
				/*
				* S = [A]
				*/
				S.erase(S.cbegin(), S.cbegin() + 2);
				D = AO;
			}
		}
		else if((AB^(AB^AC))*AO > 0.0f)
		{
			if (AB * AO)
			{
				/*
				* S = [B,A]
				*/
				S.erase(S.cbegin());
				D = (AB ^ AO) ^ AB;
			}
			else
			{
				/*
				* S = [A]
				*/
				S.erase(S.cbegin(), S.cbegin() + 2);
				D = AO;
			}
		}
		else
		{
			if ((AB ^ AC) * AO > 0.0f)
			{
				/*
				* S = [C,B,A]
				*/
				D = (AB ^ AC);
			}
			else
			{
				/*
				* S = [C,B,A]
				*/
				D = (AC ^ AB);
			}
		}
	}

	else if (S.size() == 4)
	{
		/*
		* S = [D,C,B,A]
		*/
		
		/*
		* D->C->B 순으로 clockwise가 아닌 경우 clockwise로 변경한다.
		*/
		if (( (S[2] - S[1] ) ^ (S[0] - S[1]) ) * (S[1] - S[3]) > 0.0f)
			swap(S[0], S[2]);
		
		Vector3 AB = S[2] - S[3];
		Vector3 AC = S[1] - S[3];
		Vector3 AD = S[0] - S[3];
		Vector3 AO = O - S[3];

		/*
		* temp가 0이어도 temp >= 0.0f이 거짓으로 판정됨.
		*/
		float temp;
		if ((temp = (AC ^ AB) * AO > 0.0f))// || fabs(temp) < 0.00001f)
		{
			if ((AC ^ (AC ^ AB)) * AO > 0.0f)
			{
				if (AC * AO > 0.0f)
				{
					/*
					* S = [C,A]
					*/
					S.erase(S.cbegin()+2);
					S.erase(S.cbegin());
					D = (AC ^ AO) ^ AC;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else if (((AC^AB)^AB)* AO > 0.0f)
			{
				if (AB * AO)
				{
					/*
					* S = [B,A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 2);
					D = (AB ^ AO) ^ AB;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else
			{
				/*
				* S = [C,B,A]
				*/
				S.erase(S.cbegin());
				D = AC ^ AB;
			}
		}

		else if ((temp = (AD ^ AC) * AO > 0.0f))// || (fabs(temp) < 0.00001f))
		{
			if ((AD ^ (AD ^ AC)) * AO > 0.0f)
			{
				if (AD * AO > 0.0f)
				{
					/*
					* S = [D,A]
					*/
					S.erase(S.cbegin() + 1,S.cbegin() + 3);
					D = (AD ^ AO) ^ AD;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else if (((AD ^ AC) ^ AC) * AO > 0.0f)
			{
				if (AC * AO)
				{
					/*
					* S = [C,A]
					*/
					S.erase(S.cbegin() + 2);
					S.erase(S.cbegin());
					D = (AC ^ AO) ^ AC;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else
			{
				/*
				* S = [D,C,A]
				*/
				S.erase(S.cbegin() + 2);
				D = AD ^ AC;
			}
		}

		else if ((temp = (AB ^ AD) * AO > 0.0f))// || (fabs(temp) < 0.00001f))
		{
			if ((AB ^ (AB ^ AD)) * AO > 0.0f)
			{
				if (AB * AO > 0.0f)
				{
					/*
					* S = [B,A]
					*/
					S.erase(S.cbegin(),S.cbegin()+2);
					D = (AB ^ AO) ^ AB;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else if (((AB ^ AD) ^ AD) * AO > 0.0f)
			{
				if (AD * AO)
				{
					/*
					* S = [D,A]
					*/
					S.erase(S.cbegin()+1, S.cbegin() + 3);
					D = (AD ^ AO) ^ AD;
				}
				else
				{
					/*
					* S = [A]
					*/
					S.erase(S.cbegin(), S.cbegin() + 3);
					D = AO;
				}
			}
			else
			{
				/*
				* S = [B,D,A]
				*/
				S.erase(S.cbegin() + 1);
				swap(S[0], S[1]);
				D = AB ^ AD;
			}
		}

		else
		{
			/*
			* S = [D,C,B,A]
			*/
			isIntersected = true;
		}
	}

	if (D.length() < 0.00001f)
		isIntersected = true;

	return isIntersected;
}

CollisionInfo BoxCollisionComponent::EPA(vector<Vector3>& S, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints)
{
	assert(S.size() > 0 && S.size() < 5);

	bool loopContinue = true;
	Vector3 O;
	Vector3 C;

	if (S.size() == 1)
	{
		C = S[0];
	}
	else if (S.size() == 2)
	{
		C = (S[0] + S[1]) * 0.5f;
	}
	else if (S.size() == 3)
	{
		/*
		* 가장 가까운 모서리를 선택하려면 수정해야한다.
		*/
		C = (S[0] + S[1] + S[2]) / 3.0f;
	}
	else if (S.size() == 4)
	{
		/*		 
		* loop:
		*	 D = GetClosestPoints(polytope,selfPoints,otherPoints) - O;
		*	 N = Support(D,selfPoints,otherPoints);
		*	 if( false == Expand(polytope,N)) 
		*	 {
		*		loop out
		*	 }
		*/

		/*
		* S = [D,C,B,A]
		*/
		/*
		* face를 정의한다. (counter clockwise)
		*/
		vector<int> F = { 0,1,3,
						  1,2,3,
						  2,0,3,
						  1,0,2 };
		while (loopContinue)
		{
			C = GetClosestPoints(S, F);
			Vector3 D = C;// -O;
			/*
			if (D.length() < 0.00001f)
			{
				C = Vector3(0.0f, 0.0f, 0.0f);
				break;
			}
			*/
			Vector3 N = Support(D, selfPoints, otherPoints);

			if (!Expand(S, F, N))
			{
				loopContinue = false;
			}
		}

	}
	
	mVertices[9].position = { C.v.x, C.v.y, C.v.z };
	Engine::mResourceManager->Upload(mVertexUploadBufferIdx, mVertices.data(), sizeof(Vertex) * mVertices.size(), 0);
	printf("%f , [%f %f %f]\n", C.length(), C.v.x, C.v.y, C.v.z);

	return CollisionInfo{ true,C.length(), C };
}

Vector3 BoxCollisionComponent::GetClosestPoints(const vector<Vector3>& S, const vector<int>& F) const
{
	/*
	* 우선 closest face를 계산한다.
	*/
	int numOfFace = F.size() / 3;
	Vector3 O;
	float minDistance = D3D12_FLOAT32_MAX;
	int minFace = -1;
	bool flip = false;

	for (int i = 0; i < numOfFace; ++i)
	{
		bool curFlip = false;

		int offset = i * 3;
		Vector3 normal = ((S[F[offset + 1]] - S[F[offset]]) ^ (S[F[offset + 2]] - S[F[offset]])).normalize();
		Vector3 toOrigin = O - S[F[offset]];
		float distance = normal * toOrigin;

		
		if (distance < 0.0f)
		{
			normal = ((S[F[offset + 2]] - S[F[offset]]) ^ (S[F[offset + 1]] - S[F[offset]])).normalize();
			distance = normal * toOrigin;
			curFlip = true;
		}
		

		if (distance < minDistance)
		{
			minDistance = distance;
			minFace = i;
			flip = curFlip;
		}
	}

	/*
	* origin을 closest face에 투영한다.
	*/

	int offset = minFace * 3;
	minDistance *= -1.0f;
	if (!flip)
	{
		Vector3 closestPoint = ((S[F[offset + 1]] - S[F[offset]]) ^ (S[F[offset + 2]] - S[F[offset]])).normalize() * minDistance;
		if (closestPoint.length() == 0.0f)
		printf("\nface : %d, distance : %d\n", minFace,-1.0f*minDistance);
		return closestPoint;
	}
	
	else
	{
		Vector3 closestPoint = ((S[F[offset + 2]] - S[F[offset]]) ^ (S[F[offset + 1]] - S[F[offset]])).normalize() * minDistance;
		if (closestPoint.length() == 0.0f)
		printf("\nface : %d, distance : %d\n", minFace, -1.0f * minDistance);
		return closestPoint;
	}

}

bool BoxCollisionComponent::Expand(vector<Vector3>& S, vector<int>& F, const Vector3& N) const
{
	/*
	* 우선, 어떤 facet들이 N을 볼 수 있는 지를 계산한다.
	* 즉 F의 normal vector 방향에 N이 존재하는 지를 계산한다.
	* 볼 수 있는 facet은 제거한다.
	*/
	int numOfFacet = F.size() / 3;
	set<pair<int, int>> Edges;

	for (int i = numOfFacet - 1 ; i >= 0; --i)
	{
		int offset = i * 3;
		Vector3 A = S[F[offset]];
		Vector3 B = S[F[offset + 1]];
		Vector3 C = S[F[offset + 2]];

		float dir = ((B - A) ^ (C - A)) * (N - A);

		if (dir == 0.0f)
		{
			return false;
		}
		else if (dir > 0.0f)
		{
			pair<int, int> edges[3] =
			{
				{F[offset],F[offset + 1]},
				{F[offset + 1],F[offset + 2]},
				{F[offset + 2],F[offset]}
			};
			for (int j = 0; j < 3; ++j)
			{
				if (Edges.count(edges[j]) == 0)
					Edges.insert(edges[j]);
				else
					Edges.erase(edges[j]);
			}

			F.erase(F.cbegin() + offset, F.cbegin() + offset + 3);
		}
	}
	
	/*
	* 만약 N을 볼 수 있는 facet이 없다면, false를 반환한다.
	*/
	
	if (Edges.size() == 0)
		return false;

	/*
	* edge들과 N을 이용해서 convex hull을 생성한다.
	*/
	
	S.push_back(N);
	int newVertexIdx = S.size() - 1;
	for (auto& Edge : Edges)
	{
		F.push_back(Edge.first);
		F.push_back(Edge.second);
		F.push_back(newVertexIdx);
	}
	
	return true;
}

Vector3 BoxCollisionComponent::BaryCentric(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c) const
{
	Vector3 v0 = b - a;
	Vector3 v1 = c - a;
	Vector3 v2 = p - a;

	float d00 = v0 * v0;
	float d01 = v0 * v1;
	float d11 = v1 * v1;
	float d20 = v2 * v0;
	float d21 = v2 * v1;

	float denominator = d00 * d11 + d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denominator;
	float w = (d00 * d21 - d01 * d20) / denominator;

	return Vector3(1.0f - v - w, v, w);
}

D3D12_VERTEX_BUFFER_VIEW* BoxCollisionComponent::GetVertexBufferView()
{
	mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexUploadBufferIdx)->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertices.size();

	return &mVertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* BoxCollisionComponent::GetIndexBufferView()
{
	mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexUploadBufferIdx)->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndices.size();

	return &mIndexBufferView;
}