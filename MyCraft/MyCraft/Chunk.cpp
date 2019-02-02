#include "Chunk.h"

Chunk::Chunk()
{
    cBlocks = new Block[CHUNK_WIDTH*CHUNK_WIDTH*CHUNK_HEIGHT];
}

Chunk::~Chunk()
{
    if(cBlocks != nullptr)
        delete[] cBlocks;
}

void Chunk::SetChunkPosition(XMFLOAT3 p)
{
    cPos = p;
}

void Chunk::InitBlocks(std::vector<UINT8>& hmap)
{
    // Init block type and position.
    for (int i = 0; i < hmap.size(); ++i) {
        int z = i / CHUNK_WIDTH;
        int x = i % CHUNK_WIDTH;

        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            // Has block.
            if (y < hmap[i]) {
                cBlocks[Index(z, x, y)] = Block(1, x, y, z, cPos);
            }
            // Air.
            else {
                cBlocks[Index(z, x, y)] = Block(0, x, y, z, cPos);
            }
        }
    }

    for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT; ++i) {
        cBlocks[i].CheckSurrounded(cBlocks);
    }
}

void Chunk::InitChunkMesh(std::vector<UINT8>& hmap)
{
    InitBlocks(hmap);

    int vertexOffset = 0;
    for (int i = 0; i < CHUNK_WIDTH*CHUNK_WIDTH*CHUNK_HEIGHT; ++i) {
        if (cBlocks[i].GetType() != NONE) {
            XMFLOAT3 pw = cBlocks[i].GetBlockPosW();
            for (UINT8 f = 0; f < 6; ++f) {
                switch ((Face)f)
                {
                case FRONT:
                    if (cBlocks[i].FrontVisible()) {
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z - 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z - 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z - 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z - 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0));

                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4;

                        break;
                    }

                case RIGHT:
                    if (cBlocks[i].RightVisible()) {
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z - 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z + 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z - 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z + 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1));

                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4;

                        break;
                    }

                case LEFT:
                    if (cBlocks[i].LeftVisible()) {
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z + 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 2));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z - 0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 2));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z + 0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 2));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z - 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 2));
                        
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4;

                        break;
                    }

                case BACK:
                    if (cBlocks[i].BackVisible()) {
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z + 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z + 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 3));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z + 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 3));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z + 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3));
                        
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4; 
                        
                        break;
                    }

                case TOP:
                    if (cBlocks[i].TopVisible()) {
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z + 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z - 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y + 0.5f, pw.z - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y + 0.5f, pw.z + 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4));
                        
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4; 
                        
                        break;
                    }

                case BOTTOM:
                    if (cBlocks[i].BottomVisible()) {
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z - 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z + 0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 5));
                        cMesh.v.push_back(Vertex(pw.x - 0.5f, pw.y - 0.5f, pw.z + 0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 5));
                        cMesh.v.push_back(Vertex(pw.x + 0.5f, pw.y - 0.5f, pw.z - 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5));
                        
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 1); cMesh.i.push_back(vertexOffset + 2);
                        cMesh.i.push_back(vertexOffset + 0); cMesh.i.push_back(vertexOffset + 3); cMesh.i.push_back(vertexOffset + 1);
                        vertexOffset += 4; 
                        
                        break;
                    }

                default:
                    break;
                }
            }
        }
    }
}


