#pragma once

#include "Util.h"
#include "Constant.h"
#include "Math/Quaternion.h"

class SceneNode;

class RigidBodyComponent
{
public:
                                                RigidBodyComponent(shared_ptr<SceneNode> NodeAttachedTo,float mass);
    /*
    * �� ������Ʈ�� �پ��ִ� ����� ���¸� force�� �̿��ؼ� ������Ʈ�Ѵ�.
    */
    void                                        Update(float deltaTime);
    /*
    * �� ������Ʈ�� force�� �ۿ��Ѵ�.
    */
    void                                        AddForce(Vector3 force, Vector3 relativePosition);
    /*
    * �� ������Ʈ�� impulse�� �ۿ��Ѵ�.
    */
    void                                        AddImpulse(CollisionInfo& collisionInfo,shared_ptr<RigidBodyComponent> other);

    Vector3                                     mPosition;    // position
    Vector3                                     mVelocity;    // linear velocity
    Vector3                                     mForce;       // force acting on the body
    float                                       mMass;           // mass
    XMFLOAT3X3                                  mInertiaTensor;     // moment of inertia
    XMFLOAT3X3                                  mInvInertiaTensor; //inverse of inertisa tensor
    Vector3                                     mAngularVel;  // angular velocity
    Vector3                                     mTorque;      // torque acting on the body
    Quaternion                                  mRotation;    // rotation quaternion
    XMFLOAT3X3                                  mRotatedInvInertiaTensor;


    shared_ptr<SceneNode>						mNodeAttachedTo;
};
