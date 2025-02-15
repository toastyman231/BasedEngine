#pragma once

#include <based/core/assetlibrary.h>

#include "based/scene/entity.h"
#include <based/graphics/sprite.h>
#include <based/graphics/defaultassetlibraries.h>
#include <based/input/keyboard.h>
#include <based/math/basedmath.h>

class PlayGrid : public based::scene::Entity//, std::enable_shared_from_this<PlayGrid>
{
public:
	PlayGrid(int x, int y);

	glm::ivec2 GetSize() const { return mSize; }

	void SetupTiles() const;
	void Initialize() override;
	void Update(float deltaTime) override;
	void SetScoreText(based::ui::TextEntity* text) { scoreText = text; }

	void DrawTile(int x, int y, glm::vec3 color) const;
	void MoveDown(int x, int y) const;
	void CheckRows();
	void ClearRow(int row, glm::vec3 color = {0.5f, 0.5f, 0.5f}) const;
	void AddScore(int rowsCleared);
	void SetGameOver(bool gameOver) { mGameOver = gameOver; }
	void TogglePaused() { mPaused = !mPaused; }

	bool TileFull(int x, int y) const;
	bool RowFull(int row) const;
	bool PerfectClear() const;
	bool IsClearing() const { return mClearing; }

	int GetScore() const { return mScore; }
	bool GameOver() const { return mGameOver; }
	bool Paused() const { return mPaused; }

private:
	int mScore;
	int mLevel;
	int mLastRow;
	float mTileSize;
	float mTimeToWait;
	glm::ivec2 mSize;
	std::vector<int> mClearedRows;
	std::vector<std::shared_ptr<based::graphics::Sprite>> mTiles;
	based::core::AssetLibrary<std::shared_ptr<based::graphics::Material>> mMaterials;
	based::ui::TextEntity* scoreText;

	bool mClearing;
	bool mSecondClearing;
	bool mGameOver;
	bool mPaused;
};
