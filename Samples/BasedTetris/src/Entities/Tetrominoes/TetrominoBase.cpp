#include "TetrominoBase.h"

#include "LinePiece.h"
#include "LPiece.h"

TetrominoBase* TetrominoBase::SpawnTetromino(int x, int y, TetrominoType type, PlayGrid* grid)
{
	switch (type)
	{
	case LINE:
		currentTetromino = new LinePiece(x, y, grid, {0.19f, 0.78f, 0.94f});
		break;
	case L:
		currentTetromino = new LPiece(x, y, grid, { 0.19f, 0.78f, 0.94f });
		break;
	case REVERSEL:
		break;
	case SQUARE:
		break;
	case ZIGZAG:
		break;
	case T:
		break;
	case REVERSEZIGZAG:
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
	BASED_TRACE("Finished drawing tetromino at {},{}", mPosition.x, mPosition.y)
}

void TetrominoBase::LockTetromino()
{
	mLocked = true;
	const int num = rand() % (1 - 0 + 1) + 0;
	const auto tetromino = (num == 0) ? LINE : L;
	SpawnTetromino(4, 0, tetromino, mGrid);
	// TODO: Spawn a random tetromino
}

bool TetrominoBase::ValidateNewPosition(glm::ivec2 dir)
{
	int width = 1;
	int height = 1;

	for (const auto tile : mTiles)
	{
		width += (tile.x != 0 && tile.y == 0) ? 1 : 0;
		height += (tile.y != 0) ? 1 : 0;
	}

	// Maybe have each tetromino return it's own bounds? i.e. x=-1y=-1 to x=1y=0 is the L piece
	const int xMin = mPosition.x + dir.x;
	const int xMax = (int)Clamp(0, (float)(mGrid->GetSize().x - 1), (float)(mPosition.x + (width - 1) + dir.x));
	const int yMin = mPosition.y + dir.y;
	const int yMax = mPosition.y + (height - 1) + dir.y;

	BASED_TRACE("Info for tetromino at {},{}: Width: {}, Height:{}, xMin-xMax:{}-{}, yMin-yMax:{}-{}",
		mPosition.x, mPosition.y, width, height, xMin, xMax, yMin, yMax)

	if (mGrid->TileFull(mPosition.x + dir.x, mPosition.y + dir.y) && !IsPartOfCurrentTetromino(mPosition.x + dir.x, mPosition.y + dir.y))
	{
		if (dir.y > 0) LockTetromino();
		return false;
	}

	for (const auto tile : mTiles)
	{
		if (mGrid->TileFull(mPosition.x + tile.x + dir.x, mPosition.y + tile.y + dir.y) 
			&& !IsPartOfCurrentTetromino(mPosition.x + tile.x + dir.x, mPosition.y + tile.y + dir.y))
		{
			if (dir.y > 0) LockTetromino();
			return false;
		}
	}

	return (xMin >= 0 && xMin < mGrid->GetSize().x && xMax >= 0 && xMax < mGrid->GetSize().x)
		&& (yMin >= -1 && yMin < mGrid->GetSize().y && yMax >= -1 && yMax < mGrid->GetSize().y);
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
