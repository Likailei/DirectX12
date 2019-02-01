#pragma once

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "stdafx.h"

class Block {
public:
    Block() = default;
    Block(UINT id, float x, float y, float z);

    BlockType GetType() { return bType; }
    const XMFLOAT3& GetBlockPos() const { return bPos; }
    void SetFaceTexture(UINT fi);
private:
    BlockType bType;
    UINT bId;
   // std::string bName;

    float bWidth = 1.0f;
    UINT bFaceIndex[6];
    XMFLOAT3 bPos;
};
#endif // !__BLOCK_H__

