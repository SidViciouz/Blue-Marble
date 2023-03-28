#pragma once

#include "Util.h"

class RigidBodyComponent
{
public:
                                                
    XMFLOAT3                                    position;    // position
    XMFLOAT3                                    velocity;    // linear velocity
    XMFLOAT3                                    force;       // force acting on the body
    float                                       mass;           // mass
    XMFLOAT3                                    inertia;     // moment of inertia
    XMFLOAT3                                    angularVel;  // angular velocity
    XMFLOAT3                                    torque;      // torque acting on the body
    XMFLOAT4                                    rotation;    // rotation quaternion
};
