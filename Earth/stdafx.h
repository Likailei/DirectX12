#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define SAFE_RELEASE(p) if (p) (p)->Release()

#include <windows.h>
#include <windowsx.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "../Library/d3dx12.h"
#include "../Library/lodepng.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <vector>
#include <fstream>
#include <time.h>

#include "Timer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

enum Face { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

struct Vertex {
    Vertex() {}
    
    Vertex(const XMVECTOR& pos_, const XMVECTOR& normal_) {
        XMStoreFloat3(&position, pos_);
        XMStoreFloat3(&normal, normal_);
    }

    Vertex(const XMFLOAT3& pos_, const XMFLOAT2& uv_) {
        position = pos_;
        uv = uv_;
        XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&position)));
    }

    Vertex(float x, float y, float z, float u, float v, int f) :
        position(x, y, z), uv(u, v), face(f) {
            XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&position)));
    }

    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    UINT face;
};

struct Mesh {
    std::vector<Vertex> v;
    std::vector<UINT> i;
};

struct Texture
{
    std::vector<UINT8> data;
    UINT width;
    UINT height;
};

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

inline float Clamp(float x, float low, float high) {
    return x < low ? low : (x > high ? high : x);
}