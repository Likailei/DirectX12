#include "Region.h"

Region::Region(std::string filename):
    rHMapName(filename)
{
    chunkHeightData.resize(rChunkCnt*rChunkCnt);
    rChunks.resize(rChunkCnt*rChunkCnt);
}

void Region::InitRegion()
{
    LoadHeightMap();
    TranslateAndInitHeightData();
    GenerateChunksMesh();
    unsigned i = sizeof(Chunk);
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
    for (int v = rWidth - 1; v >= 0; --v) {
        for (int u = 0; u < rWidth; u++) {
            flippedImg.push_back(rHeightMap.data[(v * rWidth + u) * 4 + 0]);
            flippedImg.push_back(rHeightMap.data[(v * rWidth + u) * 4 + 1]);
            flippedImg.push_back(rHeightMap.data[(v * rWidth + u) * 4 + 2]);
            flippedImg.push_back(rHeightMap.data[(v * rWidth + u) * 4 + 3]);
        }
    }

    // Split the height map to 32*32 pieces and extract the R component.
    for (int v = 0; v < rWidth; ++v) {
        for (int u = 0; u < rWidth; ++u) {
            int n = (v / Chunk::cWidth) * rChunkCnt + u / Chunk::cWidth;
            chunkHeightData[n].push_back(flippedImg[(v * rWidth + u) * 4]);
        }
    }
}

void Region::GenerateChunksMesh()
{
    for (int i = 0; i < 1;++i) {
        float x = static_cast<float>((i % rChunkCnt) * Chunk::cWidth);
        float z = static_cast<float>((i / rChunkCnt) * Chunk::cWidth);
        rChunks[i].SetChunkPosition(XMFLOAT3(x, 0.0f, z));

        rChunks[i].InitChunkMesh(chunkHeightData[i]);
    }
}
