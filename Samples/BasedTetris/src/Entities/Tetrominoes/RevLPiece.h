#pragma once
#include "TetrominoBase.h"

class RevLPiece : public TetrominoBase
{
public:
	RevLPiece(int x, int y, PlayGrid* grid, glm::vec3 color = { 1, 0, 0 }, float fallTime = 1.f);
};

