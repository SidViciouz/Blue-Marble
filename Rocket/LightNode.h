#pragma once

#include "MeshNode.h"
#include "Light.h"

class LightNode : public MeshNode
{
public:
												LightNode(string name,LightType lightType);

	virtual void								Draw() override;
	virtual void								Update() override;
	void										SetDirection(const XMFLOAT3& direction);
	void										SetDirection(const float& x,const float& y, const float& z);
	void										SetColor(const XMFLOAT3& color);
	void										SetColor(const float& x, const float& y, const float& z);
	void										SetType(LightType&& type);

	const Light&								GetLight() const;

protected:
	Light										mLight;
};
