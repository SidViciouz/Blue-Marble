#include "CapsuleCollisionComponent.h"
#include "SceneNode.h"

CapsuleCollisionComponent::CapsuleCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius, float height)
	: CollisionComponent(NodeAttachedTo), mRadius{radius}, mHeight{height}
{

}

bool CapsuleCollisionComponent::IsColliding(CollisionComponent* counterPart, CollisionInfo& collisionInfo)
{
    // Calculate the sum of the radii
    float radiusSum = mRadius + dynamic_cast<CapsuleCollisionComponent*>(counterPart)->mRadius;

    XMVECTOR position1 = XMLoadFloat3(&mNodeAttachedTo->mAccumulatedPosition.Get());
    XMVECTOR quaternion1 = XMLoadFloat4(&mNodeAttachedTo->mAccumulatedQuaternion.Get());
    XMVECTOR direction1ToEnd = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),quaternion1));

    XMVECTOR position2 = XMLoadFloat3(&counterPart->mNodeAttachedTo->mAccumulatedPosition.Get());
    XMVECTOR quaternion2 = XMLoadFloat4(&counterPart->mNodeAttachedTo->mAccumulatedQuaternion.Get());
    XMVECTOR direction2ToEnd = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), quaternion2));

    XMVECTOR capsule1End = position1 + direction1ToEnd * mHeight / 2.0f;
    XMVECTOR capsule1Start = position1 - direction1ToEnd * mHeight / 2.0f;

    XMVECTOR capsule2End = position2 + direction2ToEnd * mHeight / 2.0f;
    XMVECTOR capsule2Start = position2 - direction2ToEnd * mHeight / 2.0f;


    XMVECTOR capsule1Vector = capsule1End - capsule1Start;
    XMVECTOR capsule2Vector = capsule2End - capsule2Start;

    // Calculate the vector between the two start points
    XMVECTOR startVector =capsule2Start - capsule1Start;

    // Calculate the dot products of the vectors
    float dot1 = XMVectorGetX(XMVector3Dot(capsule1Vector, capsule2Vector));
    float dot2 = XMVectorGetX(XMVector3Dot(capsule1Vector, startVector));
    float dot3 = XMVectorGetX(XMVector3Dot(capsule2Vector, startVector));
    float dot4 = XMVectorGetX(XMVector3Dot(capsule1Vector, capsule1Vector));
    float dot5 = XMVectorGetX(XMVector3Dot(capsule2Vector, capsule2Vector));

    // Calculate the denominator of the t and s parameters
    float denominator = dot4 * dot5 - dot1 * dot1;

    // If the denominator is zero, the capsules are parallel
    if (denominator == 0.0f)
    {
        // Calculate the distance between the two start points
        if (XMVectorGetX(XMVector3Length(startVector)) <= radiusSum)
            return true;
        else
            return false;
        //return XMVectorGetX(XMVector3Length(startVector));
    }

    // Calculate the t and s parameters
    float t = (dot1 * dot3 - dot2 * dot5) / denominator;
    float s = (dot1 * t + dot3) / dot5;

    // Calculate the closest points on the two lines
    XMVECTOR closestPoint1 = XMVectorMultiplyAdd(capsule1Vector, XMVectorReplicate(t), capsule1Start);
    XMVECTOR closestPoint2 = XMVectorMultiplyAdd(capsule2Vector, XMVectorReplicate(s), capsule2Start);

    // Calculate the vector between the closest points
    XMVECTOR distanceVector = closestPoint2 - closestPoint1;

    // Calculate the distance between the closest points
    float distance = XMVectorGetX(XMVector3Length(distanceVector));

    // If the distance is less than the sum of the radii, there is a collision
    if (distance < radiusSum)
    {
        return true;
    }

    // Otherwise, there is no collision
    return false;
}