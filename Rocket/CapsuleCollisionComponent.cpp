#include "CapsuleCollisionComponent.h"
#include "SphereCollisionComponent.h"

CapsuleCollisionComponent::CapsuleCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius, float height)
	: CollisionComponent(NodeAttachedTo), mRadius{radius}, mHeight{height}
{

}

bool CapsuleCollisionComponent::IsColliding(CollisionComponent* counterPart)
{
    /*
    if (auto otherCapsule = dynamic_cast<CapsuleCollisionComponent*>(counterPart))
    {
        // calculate distance between capsule axis lines
        // if distance is less than sum of radii, they are colliding
        float distance = ...; // calculate distance between capsule axis lines
        float sumOfRadii = m_radius + otherCapsule->m_radius;
        if (distance <= sumOfRadii) {
            // check for collision between spheres at ends of capsules
            SphereCollisionComponent thisSphere(m_radius);
            thisSphere.setPosition(m_position + m_direction * m_height * 0.5f);
            SphereCollisionComponent otherSphere(otherCapsule->m_radius);
            otherSphere.setPosition(otherCapsule->m_position + otherCapsule->m_direction * otherCapsule->m_height * 0.5f);
            return thisSphere.isColliding(&otherSphere);
        }
        return false;
    }
    */
	return false;
}