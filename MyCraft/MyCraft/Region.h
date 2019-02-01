#pragma once
#ifndef __REGION_H__

#include "Chunk.h"

class Region {
public:
    Region(std::string filename);

    void InitRegion();
    const std::vector<Chunk>& GetChunks() const { return rChunks; }
private:
    static const unsigned rWidth = 512;
    static const unsigned rChunkCnt = 32;
    Texture rHeightMap;
    std::string rHMapName;
    std::vector<std::vector<UINT8>> chunkHeightData;
    std::vector<Chunk> rChunks;

    void LoadHeightMap();
    void TranslateAndInitHeightData();
    void GenerateChunksMesh();
};
#endif // !__REGION_H__
