#pragma once

#ifndef __CUBE_H__
#define __CUBE_H__

#include "stdafx.h"

enum FaceName { FRONT, RIGHT, LEFT, BACK, TOP, BOTTOM};

class Cube {
public:
	Cube() = default;
	Cube(UINT id);
	
	bool isSolid = true;

	UINT faceIndex[6];
	XMFLOAT3 cubePosition;

	void SetFaceTexture(UINT fi);
private:
	UINT ID;
	std::string cubeName;
};
#endif // !__CUBE_H__

