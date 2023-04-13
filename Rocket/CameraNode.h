#pragma once

#include "SceneNode.h"

class CameraNode : public SceneNode
{
public:
												CameraNode(int width, int height);
	virtual void								Update() override;
	void										Resize(int width, int height);
	void										GoFront(float d);
	void										GoRight(float d);
	void										TurnX(float x);
	void										TurnY(float y);
	const XMFLOAT4X4&							GetView() const;
	const XMFLOAT4X4&							GetProjection() const;
	const XMFLOAT4X4&							GetInvVIewProjection() const;
	const XMFLOAT3&								GetFront() const;
	void										ToggleActivate();
	const bool&									GetActivate() const;

	virtual void								Draw() override;

protected:

	void										UpdateViewMatrix();
	void										UpdateProjectionMatrix();

	XMFLOAT3									mUp;
	XMFLOAT3									mFront;
	XMFLOAT3									mRight;

	float										mNear;
	float										mFar;
	float										mAngle;
	float										mRatio;

	XMFLOAT4X4									mView;
	XMFLOAT4X4									mProjection;
	XMFLOAT4X4									mViewProjection;
	XMFLOAT4X4									mInvViewProjection;

	bool										mViewDirty = true;
	bool										mProjectionDirty = true;

	PrevMousePosition							mPrevMousePosition;
	bool										mMouseDown = false;

	bool										mActivate = true;
};
