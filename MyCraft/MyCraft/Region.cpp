#include "Region.h"

Region::Region(std::string filename):
    rHMapName(filename)
{
    rHeightData.resize(REGION_CHUNK_COUNT * REGION_CHUNK_COUNT);
    rChunks.resize(CHUNKCNT);
}

void Region::InitRegion()
{
    LoadHeightMap();
    TranslateAndInitHeightData();
    GenerateChunksMesh();
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

void Region::GenerateChunksMesh()
{
    // The south-east(bottom-left of the front) corner is the chunk's origin
    for (int i = 0; i < CHUNKCNT;++i) {
        int x = i % REGION_CHUNK_COUNT;
        int z = i / REGION_CHUNK_COUNT;
        rChunks[i].SetChunkPosition(XMFLOAT3((float)(x * CHUNK_WIDTH), 0.0f, (float)(z * CHUNK_WIDTH)));

        rChunks[i].InitChunkMesh(rHeightData[i]);
    }
}
