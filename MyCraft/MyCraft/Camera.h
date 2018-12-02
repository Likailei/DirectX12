#pragma once
#include "stdafx.h"

class Camera
{
public:
	Camera(HWND h, float aspectRatio, XMVECTOR cPos);
	~Camera();
	
	XMFLOAT4X4 mWVPMat;

	void Reset();
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseWheelRotate(WPARAM btnState);

	void SetCameraPositin(XMVECTOR camPos) { mEye = camPos; }
	XMMATRIX GetObjTransWVPMat(XMVECTOR objPos);

private:
	struct KeysPressed{
		bool w;
		bool a;
		bool s;
		bool d;
	} mKeyPressed;

	HWND mHwnd;

	float mSpeed;

	XMVECTOR mEye;
	XMVECTOR mAt;
	XMVECTOR mUp;

	XMFLOAT4X4 mProjMat;
	XMFLOAT4X4 mViewMat;
	XMFLOAT4X4 mWorldMat;
	XMFLOAT4X4 mRotMat;

	float mTheta = 0.0f;
	float mPhi = 0.0f;
	float mRadius = 5.0f;
	XMFLOAT3 mRotXYZ;

	POINT mMousePos;

	void GetViewMat();
	void GetWorldMat(XMVECTOR objPos);
	void GetRotMat(XMFLOAT3 xyz);
};