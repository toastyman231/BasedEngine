#pragma once

#include <based/core/basedtime.h>
#include <based/scene/entity.h>
#include "../PlayGrid.h"

enum TetrominoType
{
	LINE, L, REVERSEL, SQUARE, ZIGZAG, T, REVERSEZIGZAG
};

class TetrominoBase : public based::scene::Entity
{
private:
	float mWaitTime;
	float mFallInterval;
	bool mLocked = false;
	glm::vec3 mColor;
	glm::ivec2 mPosition;
	std::vector<glm::ivec2> mTiles;
	PlayGrid* mGrid;
	inline static TetrominoBase* currentTetromino = nullptr;

	void DrawTile(int x, int y) const;
	static float Clamp(float min, float max, float value);
	static TetrominoType GetRandomTetromino();
public:
	TetrominoBase(int x, int y, PlayGrid* grid, glm::vec3 color = {1.f, 0.f, 0.f}, float fallTime = 1.f);

	static TetrominoBase* SpawnTetromino(int x, int y, TetrominoType type, PlayGrid* grid);

	static TetrominoBase* GetCurrentTetromino() { return currentTetromino; }

	glm::vec3 GetColor() const { return mColor; }

	void DrawTetromino() const;
	void MoveOver(int dir = 1);
	void MoveDown();
	void DropTetromino();
	void Update(float deltaTime) override;
	void LockTetromino();

	bool ValidateNewPosition(glm::ivec2 dir = {0, 0});
	bool IsPartOfCurrentTetromino(int x, int y) const;

	void SetTiles(std::vector<glm::ivec2> tiles) { mTiles = tiles; }

	virtual void RotateCW() {}
	virtual void RotateCCW() {}
};