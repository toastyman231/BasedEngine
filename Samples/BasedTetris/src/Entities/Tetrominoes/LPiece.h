#pragma once
#include "TetrominoBase.h"

class LPiece : public TetrominoBase
{
public:
	LPiece(int x, int y, PlayGrid* grid, glm::vec3 color = { 1, 0, 0 }, float fallTime = 1.f);
};

