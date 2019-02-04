#pragma once
#include "stdafx.h"

class Camera
{
public:
    Camera(HWND h, float aspectRatio);
    ~Camera();
    
    XMFLOAT4X4 mWVPMat;

    // Get/Set world camera position.
    DirectX::XMVECTOR GetPosition()const;
    DirectX::XMFLOAT3 GetPosition3f()const;
    void SetPosition(float x, float y, float z);
    void SetPosition(const DirectX::XMFLOAT3& v);

    // Get camera basis vectors.
    DirectX::XMVECTOR GetRight()const;
    DirectX::XMFLOAT3 GetRight3f()const;
    DirectX::XMVECTOR GetUp()const;
    DirectX::XMFLOAT3 GetUp3f()const;
    DirectX::XMVECTOR GetLook()const;
    DirectX::XMFLOAT3 GetLook3f()const;

    // Set frustum.
    void SetLens(float fovY, float aspect, float zn, float zf);

    // Define camera space via LookAt parameters.
    void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
    void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

    // Get View/Proj matrices.
    DirectX::XMMATRIX GetView()const;
    DirectX::XMMATRIX GetProj()const;

    DirectX::XMFLOAT4X4 GetView4x4f()const;
    DirectX::XMFLOAT4X4 GetProj4x4f()const;

    // Strafe/Walk the camera a distance d.
    void Strafe(float d);
    void Walk(float d);

    // Rotate the camera.
    void Pitch(float angle);
    void RotateY(float angle);

    // After modifying camera position/orientation, call to rebuild the view matrix.
    void UpdateViewMatrix();

private:
    HWND mHwnd;
    bool mViewDirty = true;

    XMFLOAT3 mPosition = { 0.0f, 255.0f, 0.0f };
    XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
    XMFLOAT3 mLook = { 0.0f, 0.0f, 1.0f };
    XMFLOAT3 mFocus = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;
};