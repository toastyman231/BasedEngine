#include "LPiece.h"

LPiece::LPiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({ {-1, -1}, {-1, 0}, {1, 0} });
}