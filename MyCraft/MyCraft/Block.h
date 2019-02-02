#pragma once

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "stdafx.h"

class Block {
public:
    Block() = default;
    Block(UINT id, int x, int y, int z, const XMFLOAT3& cPos);

    void SetFaceTexture(UINT fi);

    BlockType GetType() { return bType; }
    const XMFLOAT3& GetBlockPosW() const { return bPosInWorld; }
    const XMINT3& GetBlockPosC() const { return bPosInChunk; }

    bool FrontVisible() { return faceVisible[FRONT]; }
    bool RightVisible() { return faceVisible[RIGHT]; }
    bool LeftVisible() { return faceVisible[LEFT]; }
    bool BackVisible() { return faceVisible[BACK]; }
    bool TopVisible() { return faceVisible[TOP]; }
    bool BottomVisible() { return faceVisible[BOTTOM]; }

    void CheckSurrounded(Block* chunkBlocks);
private:
    BlockType bType;
    UINT bId;

    float bWidth = 1.0f;
    UINT bFaceIndex[6];
    XMFLOAT3 bPosInWorld;
    XMINT3 bPosInChunk;
    bool faceVisible[6]{ false, false, false, false, false, false, };
};
#endif // !__BLOCK_H__

