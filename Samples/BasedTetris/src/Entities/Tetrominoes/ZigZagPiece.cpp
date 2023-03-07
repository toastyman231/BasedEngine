#include "ZigZagPiece.h"

ZigZagPiece::ZigZagPiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({ {-1, 0}, {0, -1}, {1, -1} });

	SetRotation({ {-1, 0}, {0, -1}, {1, -1} }, UP);
	SetRotation({ {0, -1}, {1, 0}, {1, 1} }, LEFT);
	SetRotation({ {-1, -1}, {-1, 0}, {0, 1} }, RIGHT);
	SetRotation({ {-1, 1}, {0, 1}, {1, 0} }, DOWN);
}
