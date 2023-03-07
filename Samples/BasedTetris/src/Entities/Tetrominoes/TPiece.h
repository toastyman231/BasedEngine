#pragma once
#include "TetrominoBase.h"

class TPiece : public TetrominoBase
{
public:
	TPiece(int x, int y, PlayGrid* grid, glm::vec3 color = { 1, 0, 0 }, float fallTime = 1.f);
};

