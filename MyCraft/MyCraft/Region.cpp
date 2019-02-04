#include "Region.h"

Region::Region(std::string filename):
    rHMapName(filename)
{
    rHeightData.resize(REGION_CHUNK_COUNT * REGION_CHUNK_COUNT);
    rChunks.resize(CHUNKCNT);
}

void Region::InitRegion(XMFLOAT3* playerPos)
{
    LoadHeightMap();
    TranslateAndInitHeightData();
    GenerateChunksMesh(playerPos);
}

void Region::LoadHeightMap()
{
    lodepng::decode(rHeightMap.data, rHeightMap.width, rHeightMap.height, rHMapName);
}

void Region::TranslateAndInitHeightData()
{
    // Height map's  bottom_left to front_left.
    // Flip the height map Vertically.
    std::vector<UINT8> flippedImg;
    for (int v = REGION_WIDTH - 1; v >= 0; --v) {
        for (int u = 0; u < REGION_WIDTH; u++) {
            flippedImg.push_back(rHeightMap.data[(v * REGION_WIDTH + u) * 4 + 0]);
            flippedImg.push_back(rHeightMap.data[(v * REGION_WIDTH + u) * 4 + 1]);
            flippedImg.push_back(rHeightMap.data[(v * REGION_WIDTH + u) * 4 + 2]);
            flippedImg.push_back(rHeightMap.data[(v * REGION_WIDTH + u) * 4 + 3]);
        }
    }

    // Split the height map to 32*32 pieces and extract the R component.
    for (int v = 0; v < REGION_WIDTH; ++v) {
        for (int u = 0; u < REGION_WIDTH; ++u) {
            int n = (v / CHUNK_WIDTH) * REGION_CHUNK_COUNT + u / CHUNK_WIDTH;
            rHeightData[n].push_back(flippedImg[(v * REGION_WIDTH + u) * 4]);
        }
    }
}

void Region::GenerateChunksMesh(XMFLOAT3* playerPos)
{
    XMINT3 playerPosInt;
    playerPosInt.x = (int)playerPos->x;
    playerPosInt.y = (int)playerPos->y;
    playerPosInt.z = (int)playerPos->z;

    int indexPlayerChunk = (playerPosInt.z / CHUNK_WIDTH) * REGION_CHUNK_COUNT + playerPosInt.x / CHUNK_WIDTH;
    int surroundedChunks[CHUNKCNT] = {
        indexPlayerChunk - 64 - 2, indexPlayerChunk - 64 - 1, indexPlayerChunk - 64, indexPlayerChunk - 64 + 1, indexPlayerChunk - 64 + 2,
        indexPlayerChunk - 32 - 2, indexPlayerChunk - 32 - 1, indexPlayerChunk - 32, indexPlayerChunk - 32 + 1, indexPlayerChunk - 32 + 2,
        indexPlayerChunk -  0 - 2, indexPlayerChunk -  0 - 1, indexPlayerChunk -  0, indexPlayerChunk -  0 + 1, indexPlayerChunk -  0 + 2,
        indexPlayerChunk + 32 - 2, indexPlayerChunk + 32 - 1, indexPlayerChunk + 32, indexPlayerChunk + 32 + 1, indexPlayerChunk + 32 + 2,
        indexPlayerChunk + 64 - 2, indexPlayerChunk + 64 - 1, indexPlayerChunk + 64, indexPlayerChunk + 64 + 1, indexPlayerChunk + 64 + 2
    };


    // Set player's y coordinate.
    playerPos->y = (float)rHeightData[indexPlayerChunk][playerPosInt.z % 16 * 16 + playerPosInt.x % 16];

    // The south-east(bottom-left of the front) corner is the chunk's origin
    for (int i = 0; i < CHUNKCNT; ++i) {
        int x = surroundedChunks[i] % REGION_CHUNK_COUNT;
        int z = surroundedChunks[i] / REGION_CHUNK_COUNT;
        rChunks[i].SetChunkPosition(XMFLOAT3((float)(x * CHUNK_WIDTH), 0.0f, (float)(z * CHUNK_WIDTH)));

        rChunks[i].InitChunkMesh(rHeightData[surroundedChunks[i]]);
    }
}
