#include "LinePiece.h"

LinePiece::LinePiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({{1, 0}, {2, 0}, {3, 0}});
}

void LinePiece::RotateCW()
{
	TetrominoBase::RotateCW();
}

void LinePiece::RotateCCW()
{
	TetrominoBase::RotateCCW();
}
