#include "GaugeNode.h"

GaugeNode::GaugeNode()
{

}

void GaugeNode::SetPercentage(unsigned int pPercentage)
{
	mPercentage = pPercentage;
}

unsigned GaugeNode::GetPercentage() const
{
	return mPercentage;
}

void GaugeNode::Draw()
{

	SceneNode::Draw();
}

void GaugeNode::Update()
{

	SceneNode::Update();
}