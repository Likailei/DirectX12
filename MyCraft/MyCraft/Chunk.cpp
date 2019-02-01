#include "Chunk.h"

Chunk::Chunk()
{
    cBlocks = new Block **[cWidth];
    for (int i = 0; i < cWidth; ++i) {
        cBlocks[i] = new Block *[cHeight];
        for (int j = 0; j < cHeight; ++j) {
            cBlocks[i][j] = new Block[cWidth];
        }
    }
}

Chunk::~Chunk()
{
    for (int i = 0; i < cWidth; ++i) {
        for (int j = 0; j < cHeight; ++j) {
            delete[] cBlocks[i][j];
        }
        delete[] cBlocks[i];
    }
    delete[] cBlocks;
}

void Chunk::SetChunkPosition(XMFLOAT3 p)
{
    cPos = p;
    cCoordMax = XMFLOAT3(cPos.x + 7.5f, 255.0f, cPos.y + 7.5f);
    cCoordMin = XMFLOAT3(cPos.x - 7.5f,   0.0f, cPos.y - 7.5f);
}

void Chunk::InitChunkMesh(std::vector<UINT8>& hmap)
{
    InitBlocks(hmap);

    for (int x = 0; x < cWidth; ++x) {
        for (int y = 0; y < cHeight; ++y) {
            for (int z = 0; z < cWidth; ++z) {
                if (cBlocks[x][y][z].GetType() != NONE) {
                    XMFLOAT3 b = cBlocks[x][y][z].GetBlockPos();
                    for (UINT8 f = 0; f < 6; ++f) {
                        if (IsFaceExposed((Face)f, b)) {
                            switch ((Face)f)
                            {
                            case FRONT:
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z - 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z - 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z - 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z - 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0));
                                break;
                            case RIGHT:
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z - 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z + 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z - 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z + 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1));
                                break;
                            case LEFT:
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z + 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 2));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z - 0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 2));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z + 0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 2));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z - 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 2));
                                break;
                            case BACK:
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z + 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z + 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 3));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z + 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 3));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z + 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3));
                                break;
                            case TOP:
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z + 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z - 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y + 0.5f, b.z - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y + 0.5f, b.z + 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4));
                                break;
                            case BOTTOM:
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z - 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z + 0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 5));
                                cMesh.v.push_back(Vertex(b.x - 0.5f, b.y - 0.5f, b.z + 0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 5));
                                cMesh.v.push_back(Vertex(b.x + 0.5f, b.y - 0.5f, b.z - 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5));
                                break;
                            default:
                                break;
                            }

                            // Add index.
                            int cnt = cMesh.v.size();
                            cMesh.i.push_back(cnt + 0);
                            cMesh.i.push_back(cnt + 1);
                            cMesh.i.push_back(cnt + 2);
                            cMesh.i.push_back(cnt + 0);
                            cMesh.i.push_back(cnt + 3);
                            cMesh.i.push_back(cnt + 1);
                        }
                    }
                }
            }
        }
    }
}

void Chunk::InitBlocks(std::vector<UINT8>& hmap)
{
    // Init block type and center position.
    for (int i = 0; i < hmap.size(); ++i) {
        unsigned z = i / cWidth;
        unsigned x = i % cWidth;
        
        for (int y = 0; y < cHeight; ++y) {
            // Has block.
            if (y < hmap[i]) {
                cBlocks[x][y][z] = Block(1, cCoordMin.x + (float)x, (float)y, cCoordMin.z + (float)z);
            }
            // Air.
            else {
                cBlocks[x][y][z] = Block(0, cCoordMin.x + (float)x, (float)y, cCoordMin.z + (float)z);
            }
        }
    }

}

bool Chunk::IsFaceExposed(Face f, XMFLOAT3 xyz)
{
    XMINT3 pos;
    
    switch (f)
    {
    case FRONT:
        xyz.z -= 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.z < cCoordMin.z || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;
        else
            return true;
    case RIGHT:                            
        xyz.x += 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.x > cCoordMax.x || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;                  
        else                               
            return true;                   
    case LEFT:                             
        xyz.x -= 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.x < cCoordMin.x || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;                  
        else                               
            return true;                   
    case BACK:                             
        xyz.z += 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.z > cCoordMax.z || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;                  
        else                               
            return true;                   
    case TOP:                              
        xyz.y += 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.y > cCoordMax.y || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;                  
        else                               
            return true;                   
    case BOTTOM:                           
        xyz.y -= 1.0f;
        pos.x = (int)(xyz.x - cCoordMin.x);
        pos.y = (int)xyz.y;
        pos.z = (int)(xyz.z - cCoordMin.z);
        if (xyz.y < cCoordMin.y || cBlocks[pos.z][pos.y][pos.x].GetType() == NONE)
            return false;
        else
            return true;
    default:
        break;
    }
   // return true;
}
