#pragma once

#include "SceneNode.h"

class VolumeNode : public SceneNode
{
public:
												VolumeNode(float width,float height,float depth);
	/*
	* volume�� draw�Ѵ�. (����� ������ drawing�ϵ��� �Ǿ��ִ�. ���� ���� ����)
	*/
	virtual void								Draw() override;
};
