#include "TetrominoBase.h"

#include "LinePiece.h"
#include "LPiece.h"
#include "RevLPiece.h"
#include "SquarePiece.h"
#include "ZigZagPiece.h"
#include "TPiece.h"
#include "RevZigZagPiece.h"

TetrominoBase* TetrominoBase::SpawnTetromino(int x, int y, TetrominoType type, PlayGrid* grid)
{
	switch (type)
	{
	case LINE:
		currentTetromino = new LinePiece(x, y, grid, {0.19f, 0.78f, 0.94f});
		break;
	case L:
		currentTetromino = new LPiece(x, y, grid, { 0.35f, 0.40f, 0.68f });
		break;
	case REVERSEL:
		currentTetromino = new RevLPiece(x, y, grid, { 0.94f, 0.47f, 0.13f });
		break;
	case SQUARE:
		currentTetromino = new SquarePiece(x, y, grid, { 0.97f, 0.83f, 0.03f });
		break;
	case ZIGZAG:
		currentTetromino = new ZigZagPiece(x, y, grid, { 0.26f, 0.71f, 0.26f });
		break;
	case T:
		currentTetromino = new TPiece(x, y, grid, { 0.68f, 0.30f, 0.61f });
		break;
	case REVERSEZIGZAG:
		currentTetromino = new RevZigZagPiece(x, y, grid, { 0.94f, 0.13f, 0.16f });
		break;
	}

	return currentTetromino;
}

TetrominoBase::TetrominoBase(int x, int y, PlayGrid* grid, glm::vec3 color, float fallTime)
	: mFallInterval(fallTime), mColor(color), mPosition(x, y), mTiles(std::vector<glm::ivec2>()), mGrid(grid)
{
	mWaitTime = based::core::Time::GetTime() + mFallInterval;
}

void TetrominoBase::DrawTetromino() const
{
	DrawTile(mPosition.x, mPosition.y);

	for (const auto tile : mTiles)
	{
		DrawTile(mPosition.x + tile.x, mPosition.y + tile.y);
	}
}

void TetrominoBase::MoveOver(int dir)
{
	if (mLocked || !ValidateNewPosition({dir, 0})) return;

	mGrid->DrawTile(mPosition.x, mPosition.y, { 0.5f, 0.5f, 0.5f });

	for (const auto tile : mTiles)
	{
		if (mPosition.x + tile.x >= 0 && mPosition.x + tile.x < mGrid->GetSize().x
			&& mPosition.y + tile.y >= 0 && mPosition.y + tile.y < mGrid->GetSize().y)
		{
			mGrid->DrawTile(mPosition.x + tile.x, mPosition.y + tile.y, { 0.5f, 0.5f, 0.5f });
		}
	}

	mPosition.x += dir;
}

void TetrominoBase::DrawTile(int x, int y) const
{
	if (x >= 0 && x < mGrid->GetSize().x && y >= 0 && y < mGrid->GetSize().y)
	{
		mGrid->DrawTile(x, y, mColor);
	}
}

float TetrominoBase::Clamp(float min, float max, float value)
{
	return (value <= min) ? min : (value >= max) ? max : value;
}

TetrominoType TetrominoBase::GetRandomTetromino()
{
	// TODO: Add random number gen to engine
	int randomNum = rand() % (6 - 0 + 1);

	switch (randomNum)
	{
	default:
		return LINE;
	case 0:
		return LINE;
	case 1:
		return L;
	case 2:
		return REVERSEL;
	case 3:
		return SQUARE;
	case 4:
		return ZIGZAG;
	case 5:
		return T;
	case 6:
		return REVERSEZIGZAG;
	}
}

void TetrominoBase::MoveDown()
{
	if (!ValidateNewPosition({0, 1})) return;

	mGrid->DrawTile(mPosition.x, mPosition.y, { 0.5f, 0.5f, 0.5f });

	for (const auto tile : mTiles)
	{
		if (mPosition.x + tile.x >= 0 && mPosition.x + tile.x < mGrid->GetSize().x 
			&& mPosition.y + tile.y >= 0 && mPosition.y + tile.y < mGrid->GetSize().y)
		{
			mGrid->DrawTile(mPosition.x + tile.x, mPosition.y + tile.y, { 0.5f, 0.5f, 0.5f });
		}
	}

	mPosition.y += 1;
}

void TetrominoBase::DropTetromino()
{
	mFallInterval = 0.001f;
	mWaitTime = based::core::Time::GetTime();
}

void TetrominoBase::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	for (const auto tile : mTiles)
	{
		if (!mLocked && mPosition.y + tile.y >= mGrid->GetSize().y - 1)
		{
			LockTetromino();
		}
	}

	if (!mLocked && based::core::Time::GetTime() >= mWaitTime)
	{
		MoveDown();
		mWaitTime += mFallInterval;
	}

	DrawTetromino();
}

void TetrominoBase::LockTetromino()
{
	mLocked = true;
	SpawnTetromino(4, 0, GetRandomTetromino(), mGrid);
}

bool TetrominoBase::ValidateNewPosition(glm::ivec2 dir)
{
	if (mGrid->TileFull(mPosition.x + dir.x, mPosition.y + dir.y) && !IsPartOfCurrentTetromino(mPosition.x + dir.x, mPosition.y + dir.y))
	{
		if (dir.y > 0) LockTetromino();
		return false;
	}

	if (mPosition.x + dir.x < 0 || mPosition.x + dir.x >= mGrid->GetSize().x || mPosition.y + dir.y < -1 || mPosition.y + dir.y >= mGrid->GetSize().y)
		return false;

	for (const auto tile : mTiles)
	{
		if (mGrid->TileFull(mPosition.x + tile.x + dir.x, mPosition.y + tile.y + dir.y) 
			&& !IsPartOfCurrentTetromino(mPosition.x + tile.x + dir.x, mPosition.y + tile.y + dir.y))
		{
			if (dir.y > 0) LockTetromino();
			return false;
		}

		if (mPosition.x + tile.x + dir.x < 0 || mPosition.x + tile.x + dir.x >= mGrid->GetSize().x 
			|| mPosition.y + tile.y + dir.y < -1 || mPosition.y + tile.y + dir.y >= mGrid->GetSize().y)
			return false;
	}

	return true;
}

bool TetrominoBase::IsPartOfCurrentTetromino(int x, int y) const
{
	if (x == mPosition.x && y == mPosition.y) return true;

	auto position = mPosition;
	return std::any_of(mTiles.begin(), mTiles.end(), [position, x, y](const auto tile)
		{
			return (position.x + tile.x == x && position.y + tile.y == y);
		});
}
