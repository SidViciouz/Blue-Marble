#include "SphereCollisionComponent.h"
#include "SceneNode.h"


SphereCollisionComponent::SphereCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius)
	: CollisionComponent(NodeAttachedTo) ,mRadius{radius}
{

}

bool SphereCollisionComponent::IsColliding(CollisionComponent* counterPart)
{
    if (auto otherSphere = dynamic_cast<SphereCollisionComponent*>(counterPart))
    {
        XMFLOAT3 position = mNodeAttachedTo->mAccumulatedPosition.Get();;

        XMFLOAT3 otherPosition = otherSphere->mNodeAttachedTo->mAccumulatedPosition.Get();

        XMFLOAT3 lengthVector;
        lengthVector.x = position.x - otherPosition.x;
        lengthVector.y = position.y - otherPosition.y;
        lengthVector.z = position.z - otherPosition.z;

        float distance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&lengthVector)));
        float sumOfRadius = mRadius + otherSphere->mRadius;

        return distance <= sumOfRadius;
    }

	return false;
}