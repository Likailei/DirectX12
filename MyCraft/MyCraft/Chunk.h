#pragma once

#ifndef __CHUNK_H__
#define __CHUNK_H__

#include "stdafx.h"
#include "Block.h"
#include <cmath>

class Chunk
{
public:
    Chunk();
    ~Chunk();

    static const unsigned cWidth = 16;
    static const unsigned cHeight = 256;

    XMFLOAT3 GetChunkPosition() { return cPos; }
    void SetChunkPosition(XMFLOAT3 p);
    const Mesh& GetChunkMesh() const{ return cMesh; }

    void InitChunkMesh(std::vector<UINT8>& hmap);
private:
    XMFLOAT3 cPos;
    Block*** cBlocks;
    Mesh cMesh;
    XMFLOAT3 cCoordMax;
    XMFLOAT3 cCoordMin;

    void InitBlocks(std::vector<UINT8>& hmap);
    bool IsFaceExposed(Face f, XMFLOAT3 xyz);
};

#endif // !__CHUNK_H__

