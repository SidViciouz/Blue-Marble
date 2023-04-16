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
    * 이 컴포넌트가 붙어있는 노드의 상태를 force를 이용해서 업데이트한다.
    */
    void                                        Update(float deltaTime);
    /*
    * 이 컴포넌트에 force를 작용한다.
    */
    void                                        AddForce(Vector3 force, Vector3 relativePosition);
    /*
    * 이 컴포넌트에 impulse를 작용한다.
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
