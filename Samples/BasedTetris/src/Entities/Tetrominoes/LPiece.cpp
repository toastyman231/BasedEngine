#include "LPiece.h"

LPiece::LPiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({ {-1, -1}, {-1, 0}, {1, 0} });

	SetRotation({ {-1, -1}, {-1, 0}, {1, 0} }, UP);
	SetRotation({ {0, -1}, {1, -1}, {0, 1} }, LEFT);
	SetRotation({ {0, -1}, {0, 1}, {-1, 1} }, RIGHT); 
	SetRotation({ {-1, 0}, { 1, 0 }, { 1, 1 } }, DOWN);
}