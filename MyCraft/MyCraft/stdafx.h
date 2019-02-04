#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define SAFE_RELEASE(p) if (p) (p)->Release()

#define CHUNKCNT 25

#include <windows.h>
#include <windowsx.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "d3dx12.h"
#include "lodepng.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <vector>
#include <fstream>
#include <time.h>

#include "Timer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

enum Face { FRONT, RIGHT, LEFT, BACK, TOP, BOTTOM };
enum BlockType { NONE, SOLID};

static const int CHUNK_WIDTH = 16;
static const int CHUNK_HEIGHT = 256;
static const int REGION_WIDTH = 512;
static const int REGION_CHUNK_COUNT = 32;
static const XMINT3 BLOCK_COORD_MAX = XMINT3(CHUNK_WIDTH - 1, CHUNK_HEIGHT - 1, CHUNK_WIDTH - 1);
static const XMINT3 BLOCK_COORD_MIN = XMINT3(0, 0, 0);

struct Vertex {
    Vertex() {}
    Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, UINT fi) :
        position(x, y, z), uv(u, v), normal(nx, ny, nz), faceIndex(fi) {}

    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    UINT faceIndex;
};

struct Mesh {
    std::vector<Vertex> v;
    std::vector<UINT> i;
};

struct Texture
{
    std::vector<UINT8> data;
    std::string name;
    UINT16 ID;
    UINT width;
    UINT height;
};

inline size_t Index(XMINT3 bPosInChunk)
{
    return bPosInChunk.z * CHUNK_WIDTH * CHUNK_HEIGHT + bPosInChunk.x * CHUNK_HEIGHT + bPosInChunk.y;
}

inline size_t Index(int z, int x, int y)
{
    return z * CHUNK_WIDTH * CHUNK_HEIGHT + x * CHUNK_HEIGHT + y;
}


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