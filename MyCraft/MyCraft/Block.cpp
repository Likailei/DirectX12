#include "Block.h"
#include "FaceIndex.h"

Block::Block(UINT id, float x, float y, float z) :
    bId(id),
    bPos(XMFLOAT3(x, y, z))
{
    if (bId == 0)
        bType = BlockType::NONE;
    else
        bType = BlockType::SOLID;

    for (UINT8 i = 0; i < 6; i++) {
        bFaceIndex[i] = FACEINDEX[id][i];
    }
}

void Block::SetFaceTexture(UINT fi)
{
    for (UINT8 i = 0; i < 6; i++) {
        bFaceIndex[i] = FACEINDEX[fi][i];
    }
}





