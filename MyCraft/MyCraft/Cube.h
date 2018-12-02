#pragma once

#ifndef __CUBE_H__
#define __CUBE_H__

#include "stdafx.h"

enum FaceName { FRONT, RIGHT, LEFT, BACK, TOP, BOTTOM};

class Cube {
public:
	Cube(UINT id);
	UINT faceIndex[6];
	XMVECTOR cubePosition;
private:
	UINT ID;
	std::string cubeName;
};
#endif // !__CUBE_H__

