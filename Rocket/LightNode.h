#pragma once

#include "MeshNode.h"
#include "Light.h"

class LightNode : public MeshNode
{
public:
												LightNode(string name,LightType lightType);
	/*
	* ������ ��ġ�� ������ mesh�� �׸���. (������ ��ġ�� �ð�ȭ�ϱ� ���� ����)
	*/
	virtual void								Draw() override;
	/*
	* ������ ����� ���¸� �� ������ ������Ʈ�Ѵ�.
	*/
	virtual void								Update() override;
	/*
	* ���� ������ �����Ѵ�.
	*/
	void										SetDirection(const XMFLOAT3& direction);
	void										SetDirection(const float& x,const float& y, const float& z);
	/*
	* ���� ���� �����Ѵ�.
	*/
	void										SetColor(const XMFLOAT3& color);
	void										SetColor(const float& x, const float& y, const float& z);
	/*
	* ������ Ÿ���� �����Ѵ�.
	*/
	void										SetType(LightType&& type);
	/*
	* ������ ���� ���µ��� ��ȯ�Ѵ�.
	*/
	const Light&								GetLight() const;

protected:
	/*
	* ������ ���� ���µ��� �����ϰ� �ִ� �����̴�.
	*/
	Light										mLight;
};
