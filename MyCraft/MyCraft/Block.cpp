#include "Block.h"
#include "FaceIndex.h"

Block::Block(UINT id, int x, int y, int z, const XMFLOAT3& cPos) :
    bId(id),
    bPosInChunk(XMINT3(x, y, z))
{
    bPosInWorld = XMFLOAT3(x + cPos.x, y + cPos.y, z + cPos.z);

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

void Block::CheckSurrounded(Block* chunkBlocks)
{
    if (bType != NONE) {
        for (int i = 0; i < 6; ++i) {
            Face f = (Face)i;
            XMINT3 p = bPosInChunk;

            switch (f)
            {
            case FRONT:
                if (p.z == BLOCK_COORD_MIN.z) {
                    faceVisible[FRONT] = true;
                    break;
                }
                p.z -= 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[FRONT] = true;
                    break;
                }
                break;

            case RIGHT:
                if (p.x == BLOCK_COORD_MAX.x) {
                    faceVisible[RIGHT] = true;
                    break;
                }
                p.x += 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[RIGHT] = true;
                    break;
                }
                break;

            case LEFT:
                if (p.x == BLOCK_COORD_MIN.x) {
                    faceVisible[LEFT] = true;
                    break;
                }
                p.x -= 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[LEFT] = true;
                    break;
                }
                break;

            case BACK:
                if (p.z == BLOCK_COORD_MAX.z) {
                    faceVisible[BACK] = true;
                    break;
                }
                p.z += 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[BACK] = true;
                    break;
                }
                break;

            case TOP:
                if (p.y == BLOCK_COORD_MAX.y) {
                    faceVisible[TOP] = true;
                    break;
                }
                p.y += 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[TOP] = true;
                    break;
                }
                break;

            case BOTTOM:
                if (p.y == BLOCK_COORD_MIN.y) {
                    faceVisible[BOTTOM] = true;
                    break;
                }
                p.y -= 1;
                if (chunkBlocks[Index(p)].GetType() == NONE) {
                    faceVisible[BOTTOM] = true;
                    break;
                }
                break;

            default:
                break;
            }
        }
    }
}



