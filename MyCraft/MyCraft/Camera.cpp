#include "Camera.h"

Camera::Camera(HWND h, float aspectRatio, XMVECTOR cPos) :
	mEye(cPos),
	mSpeed(0.001f),
	mAt(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
	mUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
	mRotXYZ(XMFLOAT3(0.000f, 0.000f, 0.000f))
{
	mHwnd = h;
	XMStoreFloat4x4(&mProjMat, XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), aspectRatio, 0.1f, 1000.0f));

	XMStoreFloat4x4(&mRotMat, XMMatrixIdentity());
}

Camera::~Camera()
{
	mHwnd = nullptr;
}

void Camera::GetViewMat()
{
	if (mKeyPressed.w) {
		
		mEye.m128_f32[2] += mSpeed;
		mAt.m128_f32[2] += mSpeed;
	}
	if (mKeyPressed.a) {
		mEye.m128_f32[0] -= mSpeed;
		mAt.m128_f32[0] -= mSpeed;
	}
	if (mKeyPressed.s) {
		mEye.m128_f32[2] -= mSpeed;
		mAt.m128_f32[2] -= mSpeed;
	}
	if (mKeyPressed.d) {
		mEye.m128_f32[0] += mSpeed;
		mAt.m128_f32[0] += mSpeed;
	}

	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//mEye = XMVectorSet(0.0f, 1.0f, -2.0f, 1.0f);
	//mAt = XMVectorZero();
	//mAt = XMVectorSet(mTheta, mPhi, z, 1.0f);
	mUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mViewMat, XMMatrixLookAtLH(mEye, mAt, mUp));
}

void Camera::GetWorldMat(XMVECTOR objPos)
{
	XMStoreFloat4x4(&mWorldMat, XMLoadFloat4x4(&mRotMat) * XMMatrixTranslationFromVector(objPos));
}

void Camera::GetRotMat(XMFLOAT3 xyz)
{
	XMMATRIX rotXMat = XMMatrixRotationX(xyz.x);
	XMMATRIX rotYMat = XMMatrixRotationY(xyz.y);
	XMMATRIX rotZMat = XMMatrixRotationZ(xyz.z);

	XMStoreFloat4x4(&mRotMat, XMLoadFloat4x4(&mRotMat) * rotXMat * rotYMat * rotZMat);
}

XMMATRIX Camera::GetObjTransWVPMat(XMVECTOR objPos)
{
	GetViewMat();
	GetRotMat(mRotXYZ);
	GetWorldMat(objPos);
	
	XMMATRIX tempWVPMat = XMLoadFloat4x4(&mWorldMat) *  XMLoadFloat4x4(&mViewMat) *  XMLoadFloat4x4(&mProjMat);
	return XMMatrixTranspose(tempWVPMat);
}

void Camera::Reset()
{
	mEye = XMVectorSet(0.0f, 1.0f, -10.0f, 0.0f);
	mAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	mUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
}

void Camera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		mKeyPressed.w = true;
		break;
	case 'A':
		mKeyPressed.a = true;
		break;
	case 'S':
		mKeyPressed.s = true;
		break;
	case 'D':
		mKeyPressed.d = true;
		break;
	}
}

void Camera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		mKeyPressed.w = false;
		break;
	case 'A':
		mKeyPressed.a = false;
		break;
	case 'S':
		mKeyPressed.s = false;
		break;
	case 'D':
		mKeyPressed.d = false;
		break;
	}
}

void Camera::OnMouseDown(WPARAM btnState, int x, int y)
{
	mMousePos.x = x;
	mMousePos.y = y;
	SetCapture(mHwnd);
}

void Camera::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Camera::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mMousePos.y));

		// Change the lookat's position
		mAt.m128_f32[0] += dx * 1.2f;
		mAt.m128_f32[1] -= dy * 3.0f;
		// Update angles based on input to orbit camera around box.
		/*mTheta += dx;
		mPhi += dy;*/

		// Restrict the angle mPhi.
		//mPhi = Clamp(mPhi, 0.1f, XM_PI - 0.1f);
	}
	mMousePos.x = x;
	mMousePos.y = y;
}

void Camera::OnMouseWheelRotate(WPARAM btnState)
{
	short delta = GET_WHEEL_DELTA_WPARAM(btnState);
	float r = delta < 0 ? 0.7f : -0.7f;
	mRadius += r;
	mRadius = Clamp(mRadius, 3.0f, 15.0f);
}
