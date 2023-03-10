#include "PlayGrid.h"

#include <based/core/basedtime.h>
#include <based/ui/textentity.h>

#include "Tetrominoes/TetrominoBase.h"

PlayGrid::PlayGrid(int x, int y)
	: mLastRow(16), mTimeToWait(0), mSize(x, y), scoreText(nullptr), mSecondClearing(false), mGameOver(false),
	  mPaused(false)
{
	srand((unsigned)time(NULL));
	mClearing = false;
	mScore = 0;
	mLevel = 1;

	for (int i = 0; i < x * y; i++)
	{
		auto sprite = new based::graphics::Sprite(glm::vec4(0.5f, 0.5f, 0.5f, 1.f));
		sprite->SetPivot(based::graphics::Align::TopLeft);
		mTiles.emplace_back(sprite);
	}

	mTileSize = ((float)based::Engine::Instance().GetWindow().GetSize().x / 3.f /
		(float)mSize.x) / ((float)based::Engine::Instance().GetWindow().GetSize().x / 3.f);
}

void PlayGrid::SetupTiles() const
{
	for (int i = 0; i < mSize.x; i++)
	{
		for (int j = 0; j < mSize.y; j++)
		{
			// TODO: Make it so you don't need to reset pivot after scaling
			mTiles[j * mSize.x + i]->SetScale({ mTileSize, mTileSize, 1.f });
			mTiles[j * mSize.x + i]->SetPivot(based::graphics::Align::TopLeft);

			const float gridWidth = (float)based::Engine::Instance().GetWindow().GetSize().x / 3.f;

			float xPos = Lerp(gridWidth, gridWidth + gridWidth, (float)i / (float)mSize.x);
			float yPos = Lerp(5.f, (float)based::Engine::Instance().GetWindow().GetSize().y, (float)j / (float)mSize.y);

			mTiles[j * mSize.x + i]->SetPosition(
				based::Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint({xPos, yPos}));
		}
	}
}

void PlayGrid::Initialize()
{
	Entity::Initialize();

	SetupTiles();
}

void PlayGrid::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (mGameOver || mPaused) return;

	if (mClearing && based::core::Time::GetTime() >= mTimeToWait)
	{
		if (!mSecondClearing)
		{
			mClearing = true;
			for (const auto i : mClearedRows)
			{
				ClearRow(i);
			}
			AddScore(static_cast<int>(mClearedRows.size()));
			mSecondClearing = true;
			mTimeToWait = based::core::Time::GetTime() + 0.5f;
		}
		else
		{
			mClearing = false;
			for (int y = mLastRow - 1; y >= 0; y--)
			{
				for (int x = 0; x < mSize.x; x++)
				{
					if (!TileFull(x, y)) continue;

					int tempY = y;
					while (tempY + 1 < mSize.y && !TileFull(x, tempY+ 1))
					{
						MoveDown(x, tempY++);
					}
				}
			}

			TetrominoBase::SpawnTetromino(4, 0, TetrominoBase::GetRandomTetromino(), this);
		}
	}
}

void PlayGrid::DrawTile(int x, int y, glm::vec3 color) const
{
	mTiles[y * mSize.x + x]->SetColor(glm::vec4(color, 1.f));
}

void PlayGrid::MoveDown(int x, int y) const
{
	if (y+1 >= mSize.y) return;

	const auto color = mTiles[y * mSize.x + x]->GetColor();
	mTiles[y * mSize.x + x]->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f));
	mTiles[(y + 1) * mSize.x + x]->SetColor(color);
}

void PlayGrid::CheckRows()
{
	mLastRow = mSize.y;
	int rowsCleared = 0;
	mClearedRows.clear();
	mSecondClearing = false;

	mClearing = true;
	for (int i = 0; i < mSize.y; i++)
	{
		if (RowFull(i))
		{
			ClearRow(i, {1.f, 1.f, 1.f});
			mClearedRows.emplace_back(i);
			mLastRow = i;
			rowsCleared++;
		}
	}

	if (rowsCleared > 0)
	{
		// TODO: Add timer system or something to make waiting easier
		mTimeToWait = based::core::Time::GetTime() + 0.5f;
		return;
	}

	TetrominoBase::SpawnTetromino(4, 0, TetrominoBase::GetRandomTetromino(), this);
	mClearing = false;
}

void PlayGrid::ClearRow(int row, glm::vec3 color) const
{
	for (int i = 0; i < mSize.x; i++)
	{
		DrawTile(i, row, color);
	}
}

void PlayGrid::AddScore(int rowsCleared)
{
	int scoreToAdd = 0;
	switch (rowsCleared)
	{
	default:
		scoreToAdd += 0;
		break;
	case 1:
		scoreToAdd += 100 * mLevel;
		if (PerfectClear()) scoreToAdd += 800 * mLevel;
		break;
	case 2:
		scoreToAdd += 300 * mLevel;
		if (PerfectClear()) scoreToAdd += 1200 * mLevel;
		break;
	case 3:
		scoreToAdd += 500 * mLevel;
		if (PerfectClear()) scoreToAdd += 1800 * mLevel;
		break;
	case 4:
		scoreToAdd += 800 * mLevel;
		if (PerfectClear()) scoreToAdd += 2000 * mLevel;
		break;
	}

	mScore += scoreToAdd;
	scoreText->SetText("Score: " + std::to_string(mScore));
}

bool PlayGrid::TileFull(int x, int y) const
{
	if (x < 0 || x >= mSize.x || y < 0 || y >= mSize.y) return false;
	return mTiles[y * mSize.x + x]->GetColor() != glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
}

bool PlayGrid::RowFull(int row) const
{
	for (int i = 0; i < mSize.x; i++)
	{
		if (!TileFull(i, row)) return false;
	}

	return true;
}

bool PlayGrid::PerfectClear() const
{
	for (int i = 0; i < mSize.x; i++)
	{
		for (int j = 0; j < mSize.y; j++)
		{
			if (mTiles[j * mSize.x + i]->GetColor() != glm::vec4(0.5f, 0.5f, 0.5f, 1.f))
			{
				return false;
			}
		}
	}

	return true;
}

// TODO: Make this part of the engine
float PlayGrid::Lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}
