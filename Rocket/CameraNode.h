#pragma once

#include "SceneNode.h"

class CameraNode : public SceneNode
{
public:
												CameraNode(int width, int height);
	/*
	* �� ������ ȣ��Ǿ� ī�޶��� ��ġ�� ������ �����Ѵ�.
	*/
	virtual void								Update() override;
	/*
	* ī�޶��� ȭ�� ����� ��ȭ�� �� ����Ѵ�.
	*/
	void										Resize(int width, int height);
	/*
	* ī�޶� ������ �̵���ų �� ����Ѵ�.
	*/
	void										GoFront(float d);
	/*
	* ī�޶� ���������� �̵���ų �� ����Ѵ�.
	*/
	void										GoRight(float d);
	/*
	* ī�޶� ȸ����ų �� ����Ѵ�.
	*/
	void										TurnX(float x);
	/*
	* ī�޶� ȸ����ų �� ����Ѵ�.
	*/
	void										TurnY(float y);
	/*
	* ī�޶��� ������ ���� ������ view matrix�� ��ȯ�Ѵ�.
	*/
	const XMFLOAT4X4&							GetView() const;
	/*
	* ī�޶��� ������ ���� ������ projection matrix�� ��ȯ�Ѵ�.
	*/
	const XMFLOAT4X4&							GetProjection() const;
	/*
	* inverse view projection matrix�� ��ȯ�Ѵ�.
	*/
	const XMFLOAT4X4&							GetInvVIewProjection() const;
	/*
	* ī�޶��� ���� ������ ��Ÿ���� ���͸� ��ȯ�Ѵ�.
	*/
	const XMFLOAT3&								GetFront() const;
	/*
	* ī�޶� �Է��� �޴��� ���θ� Ȱ��ȭ��Ų��.
	*/
	void										ToggleActivate();
	/*
	* ī�޶� Ȱ��ȭ�Ǿ������� ��ȯ�Ѵ�.
	*/
	const bool&									GetActivate() const;
	/*
	* ī�޶��� �ڽĳ����� �׸���.
	*/
	virtual void								Draw() override;

	virtual void								SetRelativePosition(const XMFLOAT3& position) override;
	virtual void								SetRelativePosition(const float& x, const float& y, const float& z) override;

	void										LookFront();

	void										LookDown();

protected:
	/*
	* ��ȭ�� ī�޶��� ������ ���� view matrix�� ������Ʈ�Ѵ�.
	*/
	void										UpdateViewMatrix();
	/*
	* ��ȭ�� ī�޶��� ������ ���� projection matrix�� ������Ʈ�Ѵ�.
	*/
	void										UpdateProjectionMatrix();
	
	/*
	* ī�޶��� �������� ����Ű�� �����̴�.
	*/
	XMFLOAT3									mUp;
	/*
	* ī�޶��� �չ����� ����Ű�� �����̴�.
	*/
	XMFLOAT3									mFront;
	/*
	* ī�޶��� ������ ������ ����Ű�� �����̴�.
	*/
	XMFLOAT3									mRight;

	/*
	* ī�޶� ����ü�� ����� �������� �Ÿ��̴�.
	*/
	float										mNear;
	/*
	* ī�޶� ����ü�� �� �������� �Ÿ��̴�.
	*/
	float										mFar;
	/*
	* ī�޶� ����ü�� �����̴�.
	*/
	float										mAngle;
	/*
	* ī�޶� ����ü�� ȭ����̴�.
	*/
	float										mRatio;

	/*
	* view matrix�̴�.
	*/
	XMFLOAT4X4									mView;
	/*
	* projection matrix�̴�.
	*/
	XMFLOAT4X4									mProjection;
	/*
	* view projection matrix�̴�.
	*/
	XMFLOAT4X4									mViewProjection;
	/*
	* inverse view projection matrix�̴�.
	*/
	XMFLOAT4X4									mInvViewProjection;

	/*
	* view�� ���õ� ������ write�Ǿ����� ���θ� ��Ÿ����.
	*/
	bool										mViewDirty = true;
	/*
	* projection�� ���õ� ������ write�Ǿ����� ���θ� ��Ÿ����.
	*/
	bool										mProjectionDirty = true;
	/*
	* ���콺�� �����ִ����� ���� �����̴�.
	*/
	bool										mMouseDown = false;
	/*
	* ī�޶� �Է��� �޾Ƶ��̴����� Ȱ��ȭ �����̴�.
	*/
	bool										mActivate = true;
};
