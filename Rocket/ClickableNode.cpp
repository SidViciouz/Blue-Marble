#include "ClickableNode.h"
#include "Engine.h"

ClickableNode::ClickableNode(string name)
	: MeshNode(name)
{
}

void ClickableNode::Draw()
{
	MeshNode::Draw();
}

void ClickableNode::Update()
{

	MeshNode::Update();
}