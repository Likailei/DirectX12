#include "Cube.h"
#include "FaceIndex.h"

Cube::Cube(UINT id) : ID(id){
	for (UINT8 i = 0; i < 6; i++) {
		faceIndex[i] = FACEINDEX[id][i];
	}
}
