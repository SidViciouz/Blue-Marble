#pragma once

#include "SceneNode.h"

class CameraNode : public SceneNode
{
public:
												CameraNode(int width, int height);
	/*
	* 매 프레임 호출되어 카메라의 위치를 정보를 수정한다.
	*/
	virtual void								Update() override;
	/*
	* 카메라의 화면 사이즈를 변화할 때 사용한다.
	*/
	void										Resize(int width, int height);
	/*
	* 카메라를 앞으로 이동시킬 때 사용한다.
	*/
	void										GoFront(float d);
	/*
	* 카메라를 오른쪽으로 이동시킬 때 사용한다.
	*/
	void										GoRight(float d);
	/*
	* 카메라를 회전시킬 때 사용한다.
	*/
	void										TurnX(float x);
	/*
	* 카메라를 회전시킬 때 사용한다.
	*/
	void										TurnY(float y);
	/*
	* 카메라의 정보를 토대로 생성된 view matrix를 반환한다.
	*/
	const XMFLOAT4X4&							GetView() const;
	/*
	* 카메라의 정보를 토대로 생성된 projection matrix를 반환한다.
	*/
	const XMFLOAT4X4&							GetProjection() const;
	/*
	* inverse view projection matrix를 반환한다.
	*/
	const XMFLOAT4X4&							GetInvVIewProjection() const;
	/*
	* 카메라의 앞의 방향을 나타내는 벡터를 반환한다.
	*/
	const XMFLOAT3&								GetFront() const;
	/*
	* 카메라가 입력을 받는지 여부를 활성화시킨다.
	*/
	void										ToggleActivate();
	/*
	* 카메라가 활성화되었는지를 반환한다.
	*/
	const bool&									GetActivate() const;
	/*
	* 카메라의 자식노드들을 그린다.
	*/
	virtual void								Draw() override;

	virtual void								SetRelativePosition(const XMFLOAT3& position) override;
	virtual void								SetRelativePosition(const float& x, const float& y, const float& z) override;

	void										LookFront();

	void										LookDown();

protected:
	/*
	* 변화된 카메라의 정보를 토대로 view matrix를 업데이트한다.
	*/
	void										UpdateViewMatrix();
	/*
	* 변화된 카메라의 정보를 토대로 projection matrix를 업데이트한다.
	*/
	void										UpdateProjectionMatrix();
	
	/*
	* 카메라의 윗방향을 가리키는 벡터이다.
	*/
	XMFLOAT3									mUp;
	/*
	* 카메라의 앞방향을 가리키는 벡터이다.
	*/
	XMFLOAT3									mFront;
	/*
	* 카메라의 오른쪽 방향을 가리키는 벡터이다.
	*/
	XMFLOAT3									mRight;

	/*
	* 카메라 절두체의 가까운 평면까지의 거리이다.
	*/
	float										mNear;
	/*
	* 카메라 절두체의 먼 평면까지의 거리이다.
	*/
	float										mFar;
	/*
	* 카메라 절두체의 각도이다.
	*/
	float										mAngle;
	/*
	* 카메라 절두체의 화면비이다.
	*/
	float										mRatio;

	/*
	* view matrix이다.
	*/
	XMFLOAT4X4									mView;
	/*
	* projection matrix이다.
	*/
	XMFLOAT4X4									mProjection;
	/*
	* view projection matrix이다.
	*/
	XMFLOAT4X4									mViewProjection;
	/*
	* inverse view projection matrix이다.
	*/
	XMFLOAT4X4									mInvViewProjection;

	/*
	* view에 관련된 정보가 write되었는지 여부를 나타낸다.
	*/
	bool										mViewDirty = true;
	/*
	* projection에 관련된 정보가 write되었는지 여부를 나타낸다.
	*/
	bool										mProjectionDirty = true;
	/*
	* 마우스가 눌려있는지에 대한 여부이다.
	*/
	bool										mMouseDown = false;
	/*
	* 카메라가 입력을 받아들이는지의 활성화 여부이다.
	*/
	bool										mActivate = true;
};
