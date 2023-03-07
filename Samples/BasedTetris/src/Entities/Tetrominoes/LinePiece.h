#pragma once

#include "TetrominoBase.h"

class LinePiece : public TetrominoBase
{
private:
public:
	LinePiece(int x, int y, PlayGrid* grid, glm::vec3 color = { 1, 0, 0 }, float fallTime = 1.f);

	void RotateCW() override;
	void RotateCCW() override;
};