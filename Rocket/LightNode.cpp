#include "LightNode.h"
#include "Engine.h"

LightNode::LightNode(string name,LightType lightType)
	: MeshNode(name)
{
	mLight.mDirection = { 0.0,-1.0,0.0f };
	mLight.mColor = { 1.0f,1.0f,1.0f };
}

void LightNode::Draw()
{

	MeshNode::Draw();
}

void LightNode::Update()
{

	MeshNode::Update();
}

void LightNode::SetDirection(const XMFLOAT3& direction)
{
	mLight.mDirection = direction;
}

void LightNode::SetDirection(const float& x, const float& y, const float& z)
{
	mLight.mDirection = { x,y,z };
}

void LightNode::SetColor(const XMFLOAT3& color)
{
	mLight.mColor = color;
}

void LightNode::SetColor(const float& x, const float& y, const float& z)
{
	mLight.mColor = { x,y,z };
}

void LightNode::SetType(LightType&& type)
{
	mLight.mType = type;
}

const Light& LightNode::GetLight() const
{
	return mLight;
}