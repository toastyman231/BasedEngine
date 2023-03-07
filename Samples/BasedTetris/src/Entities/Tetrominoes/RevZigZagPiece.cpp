#include "RevZigZagPiece.h"

RevZigZagPiece::RevZigZagPiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({ {0, -1}, {-1, -1}, {1, 0} });
}
