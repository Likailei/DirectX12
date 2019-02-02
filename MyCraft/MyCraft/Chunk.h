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

    XMFLOAT3 GetChunkPosition3f() { return cPos; }
    const Mesh& GetChunkMesh() const{ return cMesh; }

    void SetChunkPosition(XMFLOAT3 p);
    void InitChunkMesh(std::vector<UINT8>& hmap);
private:
    XMFLOAT3 cPos;    // The south-east(bottom-left of the front) corner is the chunk's origin
    Block* cBlocks;
    Mesh cMesh;

    void InitBlocks(std::vector<UINT8>& hmap);
};

#endif // !__CHUNK_H__

