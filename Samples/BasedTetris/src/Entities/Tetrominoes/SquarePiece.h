#pragma once
#include "TetrominoBase.h"

class SquarePiece : public TetrominoBase
{
public:
	SquarePiece(int x, int y, PlayGrid* grid, glm::vec3 color = { 1, 0, 0 }, float fallTime = 1.f);

	void RotateCW() override { return; }
	void RotateCCW() override { return; }
};

