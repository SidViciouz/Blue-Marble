#pragma once

#include "MeshNode.h"
#include "Light.h"

class LightNode : public MeshNode
{
public:
												LightNode(string name,LightType lightType);
	/*
	* 조명의 위치에 설정된 mesh를 그린다. (조명의 위치를 시각화하기 위한 목적)
	*/
	virtual void								Draw() override;
	/*
	* 조명의 변경된 상태를 매 프레임 업데이트한다.
	*/
	virtual void								Update() override;
	/*
	* 빛의 방향을 설정한다.
	*/
	void										SetDirection(const XMFLOAT3& direction);
	void										SetDirection(const float& x,const float& y, const float& z);
	/*
	* 빛의 색을 설정한다.
	*/
	void										SetColor(const XMFLOAT3& color);
	void										SetColor(const float& x, const float& y, const float& z);
	/*
	* 조명의 타입을 설정한다.
	*/
	void										SetType(LightType&& type);
	/*
	* 조명의 여러 상태들을 반환한다.
	*/
	const Light&								GetLight() const;

protected:
	/*
	* 조명의 여러 상태들을 저장하고 있는 변수이다.
	*/
	Light										mLight;
};
