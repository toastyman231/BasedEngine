#include "LinePiece.h"

LinePiece::LinePiece(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: TetrominoBase(x, y, grid, color, fallTime)
{
	SetTiles({{-2, 0}, {-1, 0}, {1, 0}});

	SetRotation({ {-2, 0}, {-1, 0}, {1, 0} }, UP);
	SetRotation({ {0, -2}, {0, -1}, {0, 1} }, LEFT);
	SetRotation({ {0, -2}, {0, -1}, {0, 1} }, RIGHT);
	SetRotation({ {-2, 0}, {-1, 0}, {1, 0} }, DOWN);
}

void LinePiece::RotateCW()
{
	TetrominoBase::RotateCW();

	/*if (GetDirection() == LEFT || GetDirection() == RIGHT)
		SetPosition(GetPosition().x + 2, GetPosition().y);
	else
		SetPosition(GetPosition().x - 2, GetPosition().y);*/
}

void LinePiece::RotateCCW()
{
	TetrominoBase::RotateCCW();

	/*if (GetDirection() == LEFT || GetDirection() == RIGHT)
		SetPosition(GetPosition().x + 2, GetPosition().y);
	else
		SetPosition(GetPosition().x - 2, GetPosition().y);*/
}
